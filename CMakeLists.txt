set(contents)

# We remove from compilation those processs that connect to other libraries
if (NOT DEFINED RESTLIB_DETECTOR OR NOT ${RESTLIB_DETECTOR} MATCHES "ON")
	set(excludes ${excludes} TRestRawZeroSuppresionProcess TRestRawReadoutAnalysisProcess TRestRawSignalRecoverChannelsProcess TRestRawSignalToSignalProcess )
else()
	target_link_libraries( RestDetector)
endif (NOT DEFINED RESTLIB_DETECTOR OR NOT ${RESTLIB_DETECTOR} MATCHES "ON")

COMPILEDIR(RestRaw)
