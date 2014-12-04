CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET ( COMMUNICATIONINCLUDES
	GAsioHelperFunctions.hpp
	GAsioTCPConsumerT.hpp
	GBaseClientT.hpp
	GBaseConsumerT.hpp
	GBoostThreadConsumerT.hpp
	GBrokerConnector2T.hpp
	GBrokerT.hpp
	GBufferPortT.hpp
	GCourtierEnums.hpp
	GSerialConsumerT.hpp
	GSubmissionContainerT.hpp
)

INSTALL ( FILES ${COMMUNICATIONINCLUDES} DESTINATION include/courtier )

IF ( GENEVA_WITH_MPI )
	ADD_SUBDIRECTORY( mpi )
ENDIF ()
