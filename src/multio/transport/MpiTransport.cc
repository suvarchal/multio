/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "MpiTransport.h"

#include <algorithm>
#include <fstream>

#include "eckit/config/Resource.h"
#include "eckit/exception/Exceptions.h"
#include "eckit/maths/Functions.h"
#include "eckit/runtime/Main.h"
#include "eckit/serialisation/MemoryStream.h"

#include "multio/util/ScopedTimer.h"
#include "multio/util/logfile_name.h"

namespace multio {
namespace transport {

namespace {
Message decodeMessage(eckit::Stream& stream) {
    unsigned t;
    stream >> t;

    std::string src_grp;
    stream >> src_grp;
    size_t src_id;
    stream >> src_id;

    std::string dest_grp;
    stream >> dest_grp;
    size_t dest_id;
    stream >> dest_id;

    std::string fieldId;
    stream >> fieldId;

    unsigned long sz;
    stream >> sz;

    eckit::Buffer buffer(sz);
    stream >> buffer;

    return Message{Message::Header{static_cast<Message::Tag>(t), MpiPeer{src_grp, src_id},
                                   MpiPeer{dest_grp, dest_id}, std::move(fieldId)},
                   std::move(buffer)};
}

const size_t defaultBufferSize = 64 * 1024 * 1024;
const size_t defaultPoolSize = 128;

}  // namespace

MpiTransport::MpiTransport(const ConfigurationContext& confCtx) :
    Transport(confCtx),
    local_{confCtx.config().getString("group"),
           eckit::mpi::comm(confCtx.config().getString("group").c_str()).rank()},
    pool_{eckit::Resource<size_t>("multioMpiPoolSize;$MULTIO_MPI_POOL_SIZE", defaultPoolSize),
          eckit::Resource<size_t>("multioMpiBufferSize;$MULTIO_MPI_BUFFER_SIZE", defaultBufferSize),
          comm(), statistics_} {}

MpiTransport::~MpiTransport() {
    std::ofstream logFile{util::logfile_name(), std::ios_base::app};
    logFile << "\n ** " << *this << "\n";
    statistics_.report(logFile);
}

void MpiTransport::openConnections() {
    for (auto& server : serverPeers()) {
        Message msg{Message::Header{Message::Tag::Open, local_, *server}};
        send(msg);
    }
}

void MpiTransport::closeConnections() {
    for (auto& server : serverPeers()) {
        Message msg{Message::Header{Message::Tag::Close, local_, *server}};
        bufferedSend(msg);
        pool_.sendBuffer(msg.destination(), static_cast<int>(msg.tag()));
    }
    pool_.waitAll();
}

Message MpiTransport::receive() {
    util::ScopedTiming timing{statistics_.totReturnTimer_, statistics_.totReturnTiming_};
    /**
     * Read raw messages from streamQueue_ (filled by listen() in other thread)
     *
     * Decode and add to msgPack_ (msgQueue)
     *
     * Return single messages until msgPack_ is empty and start over
     */

    do {
        while (not msgPack_.empty()) {
            util::ScopedTiming retTiming{statistics_.returnTimer_, statistics_.returnTiming_};
            //! TODO For switch to MPMC queue: combine front() and pop()
            auto msg = msgPack_.front();
            msgPack_.pop();
            return msg;
        }

        //! TODO For switch to MPMC queue: combine front() and pop()
        if (auto strm = streamQueue_.front()) {
            while (strm->position() < strm->size()) {
                util::ScopedTiming decodeTiming{statistics_.decodeTimer_,
                                                statistics_.decodeTiming_};
                auto msg = decodeMessage(*strm);
                msgPack_.push(msg);
            }
            streamQueue_.pop();
        }

    } while (true);
}

void MpiTransport::abort() {
    comm().abort();
}

void MpiTransport::send(const Message& msg) {
    std::lock_guard<std::mutex> lock{mutex_};

    auto msg_tag = static_cast<int>(msg.tag());

    // TODO: find available buffer instead
    // Add 4K for header/footer etc. Should be plenty
    eckit::Buffer buffer{eckit::round(msg.size(), 8) + 4096};

    eckit::ResizableMemoryStream stream{buffer};

    encodeMessage(stream, msg);

    util::ScopedTiming timing{statistics_.sendTimer_, statistics_.sendTiming_};

    auto sz = static_cast<size_t>(stream.bytesWritten());
    auto dest = static_cast<int>(msg.destination().id());
    eckit::mpi::comm(local_.group().c_str()).send<void>(buffer, sz, dest, msg_tag);

    ++statistics_.sendCount_;
    statistics_.sendSize_ += sz;
}

void MpiTransport::bufferedSend(const Message& msg) {
    std::lock_guard<std::mutex> lock{mutex_};
    encodeMessage(pool_.getStream(msg), msg);
}

void MpiTransport::createPeers() const {
    auto rank = 0ul;
    auto clientCount = comm().size() - confCtx_.config().getUnsigned("count");
    while (rank != clientCount) {
        clientPeers_.emplace_back(new MpiPeer{local_.group(), rank++});
    }
    while (rank != comm().size()) {
        serverPeers_.emplace_back(new MpiPeer{local_.group(), rank++});
    }
}

void MpiTransport::print(std::ostream& os) const {
    os << "MpiTransport(" << local_ << ")";
}

Peer MpiTransport::localPeer() const {
    return local_;
}

void MpiTransport::listen() {
    auto status = probe();
    if (status.error()) {
        return;
    }
    // TODO status contains information on required message size - use that to retrieve a sufficient
    // large buffer?
    auto& buf = pool_.findAvailableBuffer();
    auto sz = blockingReceive(status, buf);
    util::ScopedTiming timing{statistics_.pushToQueueTimer_, statistics_.pushToQueueTiming_};
    streamQueue_.emplace(buf, sz);
}

PeerList MpiTransport::createServerPeers() const {
    PeerList serverPeers;

    // This is dangerous as it requires having the same logic as in NEMO or IFS
    // This needs to come from the configuration or perhaps you want to create an intercommunicator
    auto rank = comm().size() - confCtx_.config().getUnsigned("count");
    while (rank != comm().size()) {
        serverPeers.emplace_back(new MpiPeer{local_.group(), rank++});
    }
    return serverPeers;
}

const eckit::mpi::Comm& MpiTransport::comm() const {
    return eckit::mpi::comm(local_.group().c_str());
}

eckit::mpi::Status MpiTransport::probe() {
    util::ScopedTiming timing{statistics_.probeTimer_, statistics_.probeTiming_};
    auto status = comm().iProbe(comm().anySource(), comm().anyTag());

    return status;
}

size_t MpiTransport::blockingReceive(eckit::mpi::Status& status, MpiBuffer& buffer) {
    auto sz = comm().getCount<void>(status);
    ASSERT(sz < buffer.content.size());

    util::ScopedTiming timing{statistics_.receiveTimer_, statistics_.receiveTiming_};
    comm().receive<void>(buffer.content, sz, status.source(), status.tag());

    ++statistics_.receiveCount_;
    statistics_.receiveSize_ += sz;

    return sz;
}

void MpiTransport::encodeMessage(eckit::Stream& strm, const Message& msg) {
    util::ScopedTiming timing{statistics_.encodeTimer_, statistics_.encodeTiming_};

    msg.encode(strm);
}

static TransportBuilder<MpiTransport> MpiTransportBuilder("mpi");

}  // namespace transport
}  // namespace multio
