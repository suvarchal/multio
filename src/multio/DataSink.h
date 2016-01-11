/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Tiago Quintino
/// @author Simon Smart
/// @date Dec 2015


#ifndef multio_DataSink_H
#define multio_DataSink_H

#include <iosfwd>
#include <string>
#include <vector>

#include "eckit/config/LocalConfiguration.h"
#include "eckit/config/Configuration.h"
#include "eckit/memory/NonCopyable.h"
#include "eckit/io/Length.h"
#include "eckit/io/Buffer.h"

#include "Journal.h"

namespace multio {

    class Metadata;

//----------------------------------------------------------------------------------------------------------------------

class DataSink : private eckit::NonCopyable {

public: // methods

    DataSink(const eckit::Configuration& config);

    virtual ~DataSink();

    virtual bool ready() const;

    virtual void write(const void* buffer,
                       const eckit::Length& length,
                       JournalRecord *const record = NULL,
                       Metadata *const metadata = NULL) = 0;

    /// Dump all relevant config details to json, which can be used to reinitialise
    /// the datasink in playjournal. Not necessarily equal to the supplied config
    /// as other sources of configuration may be used. By default this just returns
    /// the supplied Configuration.
    virtual std::string json() const;

    /// Set the datasink ID that is used by other classes to identify this one.
    /// In particular, it labels which sink within a MultIO this one is.
    void setId(int id);

    ///
    /// LEGACY INTERFACE TO REMOVE AFTER IFS CHANGED TO SIMPLE WRITE() INTERFACE
    ///

    virtual int iopenfdb(const char *name, const char *mode, int name_len, int mode_len);
    virtual int iinitfdb();
    virtual int iclosefdb();

    virtual int isetcommfdb(int *rank);
    virtual int isetrankfdb(int *rank);
    virtual int iset_fdb_root(const char *name, int name_len);

    virtual int iflushfdb();

    virtual int isetfieldcountfdb(int *all_ranks, int *this_rank);
    virtual int isetvalfdb(const char *name, const char *value, int name_len, int value_len);

    // virtual int ireadfdb(void *data, int *words);
    // virtual iwritefdb(void *data, int *words);

protected: // methods

    virtual void print(std::ostream&) const = 0;

    bool failOnError() { return failOnError_; }

private: // methods

    friend std::ostream &operator<<(std::ostream &s, const DataSink &p) {
        p.print(s);
        return s;
    }

protected: // members

    bool failOnError_;
    bool journaled_;        /// Write to a journal file
    bool journalAlways_;    /// Write details to journal even if a write succeeds.

    eckit::LocalConfiguration config_;
    int id_;
};

//----------------------------------------------------------------------------------------------------------------------

class DataSinkFactory {

    std::string name_;
    virtual DataSink* make(const eckit::Configuration& config) = 0;

protected:

    DataSinkFactory(const std::string&);
    virtual ~DataSinkFactory();

public:

    static void list(std::ostream &);
    static DataSink* build(const std::string&, const eckit::Configuration& config);

};

template< class T>
class DataSinkBuilder : public DataSinkFactory {

    virtual DataSink* make(const eckit::Configuration& config) {
        return new T(config);
    }

public:
    DataSinkBuilder(const std::string &name) : DataSinkFactory(name) {}
};

//----------------------------------------------------------------------------------------------------------------------

}  // namespace multio

#endif

