#include "output_manager_preprocessor_utils.h"
#include "output_manager_preprocessor_trace_utils.h"
#include "output_manager_preprocessor_logging_utils.h"
#include "output_manager_preprocessor_errhdl_utils.h"


#define PP_FILE_NAME 'grib_encoder_grib2_gg_p_mod.F90'
#define PP_SECTION_TYPE 'MODULE'
#define PP_SECTION_NAME 'GRIB_ENCODER_GRIB2_GG_P_MOD'
MODULE GRIB_ENCODER_GRIB2_GG_P_MOD

  ! Symbols imported from other modules within the project.
  USE :: GRIB_ENCODER_BASE_MOD, ONLY: GRIB_ENCODER_A
  USE :: METADATA_BASE_MOD,     ONLY: METADATA_BASE_A

IMPLICIT NONE

! Default visibility of the module
PRIVATE

! Encoder for GRIB2 pressure level fields
TYPE, EXTENDS(GRIB_ENCODER_A) :: ENCODER_GRIB2_GG_P

  ! Default visibility
  PRIVATE

  !> @brief Metadata handlers used to store encoded metadata.
  CLASS(METADATA_BASE_A), POINTER :: SAMPLE_ => NULL()

  !> @brief local variable to store the type of the field
  CHARACTER(LEN=16) :: CLTYPE_ = REPEAT( ' ', 16 )

CONTAINS
  ! Virtual procedures
  PROCEDURE, PUBLIC, PASS, NON_OVERRIDABLE :: INITIALISE => ENCODER_GRIB2_GG_P_INITIALISE
  PROCEDURE, PUBLIC, PASS, NON_OVERRIDABLE :: ENCODE_ATM => ENCODER_GRIB2_GG_P_ENCODE_ATM
  PROCEDURE, PUBLIC, PASS, NON_OVERRIDABLE :: ENCODE_WAM => ENCODER_GRIB2_GG_P_ENCODE_WAM
  PROCEDURE, PUBLIC, PASS, NON_OVERRIDABLE :: FINALISE   => ENCODER_GRIB2_GG_P_FINALISE
END TYPE

! Name of the grib sample to be loaded
! CHARACTER(LEN=*), PARAMETER :: SAMPLE_NAME_GRIB2_GG_P='gg_sfc_grib2'
CHARACTER(LEN=*), PARAMETER :: SAMPLE_NAME_GRIB2_GG_P='multio-gg-sfc-grib2'

! Whitelist of public symbols
PUBLIC :: MAKE_GRIB2_GG_P_ENCODER
PUBLIC :: DESTROY_GRIB2_GG_P_ENCODER

CONTAINS


#define PP_PROCEDURE_TYPE 'SUBROUTINE'
#define PP_PROCEDURE_NAME 'MAKE_GRIB2_GG_P_ENCODER'
SUBROUTINE MAKE_GRIB2_GG_P_ENCODER( CFG, MODEL_PARAMS, METADATA_KIND, ENCODER, MIOH )

  ! Symbols imported from other modules within the project.
  USE :: GRIB_ENCODER_BASE_MOD, ONLY: GRIB_ENCODER_A
  USE :: OM_CORE_MOD,           ONLY: MODEL_PAR_T
  USE :: OM_CORE_MOD,           ONLY: JPIB_K

  ! Symbols imported from other libraries
  USE :: FCKIT_CONFIGURATION_MODULE, ONLY: FCKIT_CONFIGURATION
  USE :: MULTIO_API,                 ONLY: MULTIO_HANDLE

  ! Symbols imported by the preprocessor for debugging purposes
  PP_DEBUG_USE_VARS

  ! Symbols imported by the preprocessor for tracing purposes
  PP_TRACE_USE_VARS

IMPLICIT NONE

  ! Dummy arguments
  TYPE(FCKIT_CONFIGURATION),      INTENT(IN)    :: CFG
  TYPE(MODEL_PAR_T),              INTENT(IN)    :: MODEL_PARAMS
  CHARACTER(LEN=*),               INTENT(IN)    :: METADATA_KIND
  CLASS(GRIB_ENCODER_A), POINTER, INTENT(INOUT) :: ENCODER
  TYPE(MULTIO_HANDLE), OPTIONAL,  INTENT(IN)    :: MIOH

  ! Local variables
  INTEGER(KIND=JPIB_K) :: GRIB_HANDLE
  INTEGER(KIND=JPIB_K) :: STAT
  CHARACTER(LEN=:), ALLOCATABLE :: ERRMSG

  ! Local variables declared by the preprocessor for debugging purposes
  PP_DEBUG_DECL_VARS

  ! Local variables declared by the preprocessor for tracing purposes
  PP_TRACE_DECL_VARS

  ! Trace begin of procedure
  PP_TRACE_ENTER_PROCEDURE()

  ! Allocate the encoder
  NULLIFY(ENCODER)
  ALLOCATE(ENCODER_GRIB2_GG_P::ENCODER, STAT=STAT, ERRMSG=ERRMSG)
  PP_DEBUG_CRITICAL_COND_THROW( STAT.NE.0, 1 )

  ! Allocation of the internal structure of the encoder
  IF ( PRESENT(MIOH) ) THEN
    CALL ENCODER%INITIALISE( MODEL_PARAMS, METADATA_KIND, MIOH=MIOH )
  ELSE
    CALL ENCODER%INITIALISE( MODEL_PARAMS, METADATA_KIND )
  ENDIF

  ! If errmsg is allocated and any I arrive here something is wrong
  IF ( ALLOCATED(ERRMSG) ) THEN
    DEALLOCATE(ERRMSG)
  ENDIF

  ! Trace end of procedure (on success)
  PP_TRACE_EXIT_PROCEDURE_ON_SUCCESS()

  ! Exit point on success
  RETURN

! Error handler
PP_ERROR_HANDLER

  ErrorHandler: BLOCK

    ! Error handling variables
    CHARACTER(LEN=:), ALLOCATABLE :: STR

    ! HAndle different errors
    SELECT CASE(ERRIDX)

    CASE (1)
      PP_DEBUG_CREATE_ERROR_MSG( STR, 'Unable to allocate the encoder' )
    CASE DEFAULT
      PP_DEBUG_CREATE_ERROR_MSG( STR, 'Unhandled error' )
    END SELECT

    ! Trace end of procedure (on error)
    PP_TRACE_EXIT_PROCEDURE_ON_ERROR()

    ! Write the error message and stop the program
    PP_DEBUG_ABORT( STR )

  END BLOCK ErrorHandler

  ! Exit point on error
  RETURN

END SUBROUTINE MAKE_GRIB2_GG_P_ENCODER
#undef PP_PROCEDURE_NAME
#undef PP_PROCEDURE_TYPE



#define PP_PROCEDURE_TYPE 'SUBROUTINE'
#define PP_PROCEDURE_NAME 'DESTROY_GRIB2_GG_P_ENCODER'
SUBROUTINE DESTROY_GRIB2_GG_P_ENCODER( ENCODER )

  ! Symbols imported by the preprocessor for debugging purposes
  PP_DEBUG_USE_VARS

  ! Symbols imported by the preprocessor for tracing purposes
  PP_TRACE_USE_VARS

IMPLICIT NONE

  ! Dummy arguments
  CLASS(GRIB_ENCODER_A), POINTER, INTENT(INOUT) :: ENCODER

  ! Local variables declared by the preprocessor for debugging purposes
  PP_DEBUG_DECL_VARS

  ! Local variables declared by the preprocessor for tracing purposes
  PP_TRACE_DECL_VARS

  ! Trace begin of procedure
  PP_TRACE_ENTER_PROCEDURE()

  ! Free the internal memory of the encoder
  IF ( ASSOCIATED( ENCODER ) ) THEN
    CALL ENCODER%FINALISE()
    DEALLOCATE( ENCODER )
  ENDIF

  ! Trace end of procedure (on success)
  PP_TRACE_EXIT_PROCEDURE_ON_SUCCESS()

  ! Exit point on success
  RETURN

END SUBROUTINE DESTROY_GRIB2_GG_P_ENCODER
#undef PP_PROCEDURE_NAME
#undef PP_PROCEDURE_TYPE



#define PP_PROCEDURE_TYPE 'SUBROUTINE'
#define PP_PROCEDURE_NAME 'ENCODER_GRIB2_GG_P_INITIALISE'
SUBROUTINE ENCODER_GRIB2_GG_P_INITIALISE( THIS, MODEL_PARAMS, METADATA_KIND, MIOH )

  ! Symbols imported from other modules within the project.
  USE :: OM_CORE_MOD,          ONLY: MODEL_PAR_T
  USE :: METADATA_FACTORY_MOD, ONLY: MAKE_METADATA

  USE :: GENERAL_PRESET_UTILS_MOD, ONLY: LATEST_TABLE_VERSION
  USE :: GENERAL_PRESET_UTILS_MOD, ONLY: GENERATING_PROCESS_IDENTIFIER_PRESET
  USE :: MARS_PRESET_UTILS_MOD, ONLY: MARS_PRESET_GRIBX_ATM
  USE :: TIME_PRESET_UTILS_MOD, ONLY: TIME_PRESET_GRIBX_ATM
  USE :: GEOMETRY_PRESET_UTILS_MOD, ONLY: GG_GEOMETRY_PRESET_GRIBX_ATM

  ! Symbols imported from other libraries
  USE :: MULTIO_API, ONLY: MULTIO_HANDLE

  ! Symbols imported by the preprocessor for debugging purposes
  PP_DEBUG_USE_VARS

  ! Symbols imported by the preprocessor for tracing purposes
  PP_TRACE_USE_VARS

IMPLICIT NONE

  ! Dummy arguments
  CLASS(ENCODER_GRIB2_GG_P),     INTENT(INOUT) :: THIS
  CHARACTER(LEN=*),              INTENT(IN)    :: METADATA_KIND
  TYPE(MODEL_PAR_T),             INTENT(IN)    :: MODEL_PARAMS
  TYPE(MULTIO_HANDLE), OPTIONAL, INTENT(IN)    :: MIOH

  ! Local variables declared by the preprocessor for debugging purposes
  PP_DEBUG_DECL_VARS

  ! Local variables declared by the preprocessor for tracing purposes
  PP_TRACE_DECL_VARS

  ! Trace begin of procedure
  PP_TRACE_ENTER_PROCEDURE()

  ! Allocate the metadata
  IF ( PRESENT(MIOH) ) THEN
    CALL MAKE_METADATA( METADATA_KIND, THIS%SAMPLE_, MIOH=MIOH )
  ELSE
    CALL MAKE_METADATA( METADATA_KIND, THIS%SAMPLE_ )
  ENDIF

  ! Load the sample
  PP_METADATA_INIT_FROM_SAMPLE_NAME( THIS%SAMPLE_, TRIM(ADJUSTL(SAMPLE_NAME_GRIB2_GG_P)) )

  ! Load the grib sample
  PP_LOG_DEVELOP_STR( 'ENCODER: grib2, gridded, pressure_level, preset' )

  ! Preset operations (to be moved in the initialisation)
  CALL LATEST_TABLE_VERSION( MODEL_PARAMS, THIS%SAMPLE_ )

  CALL GENERATING_PROCESS_IDENTIFIER_PRESET( MODEL_PARAMS, THIS%SAMPLE_ )

  THIS%CLTYPE_ = MARS_PRESET_GRIBX_ATM( MODEL_PARAMS, THIS%SAMPLE_ )

  CALL TIME_PRESET_GRIBX_ATM( MODEL_PARAMS, THIS%SAMPLE_ )

  CALL GG_GEOMETRY_PRESET_GRIBX_ATM( MODEL_PARAMS, THIS%SAMPLE_ )

  ! Trace end of procedure (on success)
  PP_TRACE_EXIT_PROCEDURE_ON_SUCCESS()

  ! Exit point on success
  RETURN

END SUBROUTINE ENCODER_GRIB2_GG_P_INITIALISE
#undef PP_PROCEDURE_NAME
#undef PP_PROCEDURE_TYPE


#define PP_PROCEDURE_TYPE 'FUNCTION'
#define PP_PROCEDURE_NAME 'ENCODER_GRIB2_GG_P_ENCODE_ATM'
FUNCTION ENCODER_GRIB2_GG_P_ENCODE_ATM( THIS, MODEL_PARAMS, GRIB_INFO, &
& TIME_HIST, CURR_TIME, MSG, METADATA ) RESULT(EX)

  ! Symbols imported from other modules within the project.
  USE :: OM_CORE_MOD,        ONLY: JPIB_K
  USE :: OM_CORE_MOD,        ONLY: GRIB2_E
  USE :: OM_CORE_MOD,        ONLY: MODEL_PAR_T
  USE :: OM_CORE_MOD,        ONLY: OM_ATM_MSG_T
  USE :: OM_CORE_MOD,        ONLY: GRIB_INFO_T
  USE :: OM_CORE_MOD,        ONLY: TIME_HISTORY_T
  USE :: OM_CORE_MOD,        ONLY: CURR_TIME_T
  USE :: METADATA_BASE_MOD,  ONLY: METADATA_BASE_A

  USE :: PACKAGING_RUNTIME_UTILS_MOD, ONLY: SET_MISSING_VALUE_GG_GRIBX_ATM
  USE :: PACKAGING_RUNTIME_UTILS_MOD, ONLY: SET_BITS_PER_VALUE_GG_GRIBX_ATM

  USE :: LEVELS_RUNTIME_UTILS_MOD, ONLY: SET_PRESSURE_LEVEL_GRIBX_ATM
  USE :: TIME_RUNTIME_UTILS_MOD, ONLY: TIME_ENCODERS_ATM
  USE :: TIME_RUNTIME_UTILS_MOD, ONLY: TIME_INDEXER
  USE :: PARAMETERS_RUNTIME_UTILS_MOD, ONLY: COMPUTE_PARAM_GRIBX_ATM

  USE :: GENERAL_RUNTIME_UTILS_MOD, ONLY: SET_PRODUCT_DEFINITION_TEMPLATE_NUMBER_ATM
  USE :: COMPRESSION_RUNTIME_UTILS_MOD, ONLY: SET_COMPRESSION_ATM

  ! Symbols imported by the preprocessor for logging purposes
  PP_LOG_USE_VARS

  ! Symbols imported by the preprocessor for debugging purposes
  PP_DEBUG_USE_VARS

  ! Symbols imported by the preprocessor for tracing purposes
  PP_TRACE_USE_VARS

IMPLICIT NONE

  ! Dummy arguments
  CLASS(ENCODER_GRIB2_GG_P),       INTENT(INOUT) :: THIS
  TYPE(MODEL_PAR_T),               INTENT(IN)    :: MODEL_PARAMS
  TYPE(GRIB_INFO_T),               INTENT(IN)    :: GRIB_INFO
  TYPE(TIME_HISTORY_T),            INTENT(IN)    :: TIME_HIST
  TYPE(CURR_TIME_T),               INTENT(IN)    :: CURR_TIME
  TYPE(OM_ATM_MSG_T),              INTENT(IN)    :: MSG
  CLASS(METADATA_BASE_A), POINTER, INTENT(INOUT) :: METADATA

  ! Function result
  LOGICAL :: EX

  ! Local variables
  INTEGER(KIND=JPIB_K) :: IDX
  INTEGER(KIND=JPIB_K) :: ILPARAM
  CHARACTER(LEN=16)    :: CLTYPE

  ! Local variables declared by the preprocessor for debugging purposes
  PP_LOG_DECL_VARS

  ! Local variables declared by the preprocessor for debugging purposes
  PP_DEBUG_DECL_VARS

  ! Local variables declared by the preprocessor for tracing purposes
  PP_TRACE_DECL_VARS

  ! Trace begin of procedure
  PP_TRACE_ENTER_PROCEDURE()
  PP_METADATA_ENTER_PROCEDURE( METADATA )

  EX = .TRUE.

  PP_LOG_DEVELOP_STR( 'ENCODER: grib2, gridded, pressure_level, runtime' )

  ! Metadata initialisation (clone the sample)
  PP_METADATA_INIT_FROM_METADATA( METADATA, THIS%SAMPLE_ )

  CALL SET_PRODUCT_DEFINITION_TEMPLATE_NUMBER_ATM( MODEL_PARAMS, GRIB_INFO, TIME_HIST, CURR_TIME, MSG, METADATA )

  CALL SET_MISSING_VALUE_GG_GRIBX_ATM( MODEL_PARAMS, GRIB_INFO, MSG, METADATA )

  CALL SET_BITS_PER_VALUE_GG_GRIBX_ATM( MODEL_PARAMS, GRIB_INFO, MSG, METADATA )

  CALL SET_PRESSURE_LEVEL_GRIBX_ATM( MODEL_PARAMS, GRIB_INFO, MSG, METADATA )

  IDX = TIME_INDEXER(GRIB_INFO%TYPE_OF_STATISTICAL_PROCESS_,GRIB_INFO%TYPE_OF_TIME_RANGE_,GRIB2_E)

  CALL TIME_ENCODERS_ATM(IDX)%ENCODE_TIME( MODEL_PARAMS, GRIB_INFO, TIME_HIST, CURR_TIME, MSG, METADATA )

  CALL SET_COMPRESSION_ATM( MODEL_PARAMS, GRIB_INFO, MSG, METADATA )

  ILPARAM = COMPUTE_PARAM_GRIBX_ATM( MODEL_PARAMS, GRIB_INFO, MSG, THIS%CLTYPE_, METADATA )

  IF ( ILPARAM .NE. 128 ) THEN
    PP_METADATA_SET( METADATA,  'paramId', ILPARAM )
  ENDIF

  ! Trace end of procedure (on success)
  PP_METADATA_EXIT_PROCEDURE( METADATA )
  PP_TRACE_EXIT_PROCEDURE_ON_SUCCESS()

  ! Exit point on success
  RETURN

END FUNCTION ENCODER_GRIB2_GG_P_ENCODE_ATM
#undef PP_PROCEDURE_NAME
#undef PP_PROCEDURE_TYPE


#define PP_PROCEDURE_TYPE 'FUNCTION'
#define PP_PROCEDURE_NAME 'ENCODER_GRIB2_GG_P_ENCODE_WAM'
FUNCTION ENCODER_GRIB2_GG_P_ENCODE_WAM( THIS, MODEL_PARAMS, GRIB_INFO, &
& TIME_HIST, CURR_TIME, MSG, METADATA ) RESULT(EX)

  ! Symbols imported from other modules within the project.
  USE :: OM_CORE_MOD,        ONLY: JPIB_K
  USE :: OM_CORE_MOD,        ONLY: GRIB2_E
  USE :: OM_CORE_MOD,        ONLY: MODEL_PAR_T
  USE :: OM_CORE_MOD,        ONLY: OM_WAM_MSG_T
  USE :: OM_CORE_MOD,        ONLY: GRIB_INFO_T
  USE :: OM_CORE_MOD,        ONLY: TIME_HISTORY_T
  USE :: OM_CORE_MOD,        ONLY: CURR_TIME_T
  USE :: METADATA_BASE_MOD,  ONLY: METADATA_BASE_A

  ! Symbols imported by the preprocessor for logging purposes
  PP_LOG_USE_VARS

  ! Symbols imported by the preprocessor for debugging purposes
  PP_DEBUG_USE_VARS

  ! Symbols imported by the preprocessor for tracing purposes
  PP_TRACE_USE_VARS

IMPLICIT NONE

  ! Dummy arguments
  CLASS(ENCODER_GRIB2_GG_P),       INTENT(INOUT) :: THIS
  TYPE(MODEL_PAR_T),               INTENT(IN)    :: MODEL_PARAMS
  TYPE(GRIB_INFO_T),               INTENT(IN)    :: GRIB_INFO
  TYPE(TIME_HISTORY_T),            INTENT(IN)    :: TIME_HIST
  TYPE(CURR_TIME_T),               INTENT(IN)    :: CURR_TIME
  TYPE(OM_WAM_MSG_T),              INTENT(IN)    :: MSG
  CLASS(METADATA_BASE_A), POINTER, INTENT(INOUT) :: METADATA

  ! Function result
  LOGICAL :: EX

  ! Local variables declared by the preprocessor for debugging purposes
  PP_LOG_DECL_VARS

  ! Local variables declared by the preprocessor for debugging purposes
  PP_DEBUG_DECL_VARS

  ! Local variables declared by the preprocessor for tracing purposes
  PP_TRACE_DECL_VARS

  ! Trace begin of procedure
  PP_TRACE_ENTER_PROCEDURE()
  PP_METADATA_ENTER_PROCEDURE( METADATA )


  EX = .FALSE.

  PP_LOG_DEVELOP_STR( 'Grib2 PRESSURE_LEVEL encoder' )

  ! Trace end of procedure (on success)
  PP_METADATA_EXIT_PROCEDURE( METADATA )
  PP_TRACE_EXIT_PROCEDURE_ON_SUCCESS()

  ! Exit point on success
  RETURN

END FUNCTION ENCODER_GRIB2_GG_P_ENCODE_WAM
#undef PP_PROCEDURE_NAME
#undef PP_PROCEDURE_TYPE



#define PP_PROCEDURE_TYPE 'SUBROUTINE'
#define PP_PROCEDURE_NAME 'ENCODER_GRIB2_GG_P_FINALISE'
SUBROUTINE ENCODER_GRIB2_GG_P_FINALISE( THIS )

  ! Symbols imported from other modules within the project.
  USE :: METADATA_FACTORY_MOD, ONLY: DESTROY_METADATA

  ! Symbols imported by the preprocessor for debugging purposes
  PP_DEBUG_USE_VARS

  ! Symbols imported by the preprocessor for tracing purposes
  PP_TRACE_USE_VARS

IMPLICIT NONE

  ! Dummy arguments
  CLASS(ENCODER_GRIB2_GG_P), INTENT(INOUT) :: THIS

  ! Local variables declared by the preprocessor for debugging purposes
  PP_DEBUG_DECL_VARS

  ! Local variables declared by the preprocessor for tracing purposes
  PP_TRACE_DECL_VARS

  ! Trace begin of procedure
  PP_TRACE_ENTER_PROCEDURE()

  ! Destroy the handle
  IF ( ASSOCIATED(THIS%SAMPLE_) ) THEN
    CALL THIS%SAMPLE_%DESTROY()
    CALL DESTROY_METADATA( THIS%SAMPLE_ )
  ENDIF

  ! Trace end of procedure (on success)
  PP_TRACE_EXIT_PROCEDURE_ON_SUCCESS()

  ! Exit point on success
  RETURN

END SUBROUTINE ENCODER_GRIB2_GG_P_FINALISE
#undef PP_PROCEDURE_NAME
#undef PP_PROCEDURE_TYPE


END MODULE GRIB_ENCODER_GRIB2_GG_P_MOD
#undef PP_SECTION_NAME
#undef PP_SECTION_TYPE
#undef PP_FILE_NAME
