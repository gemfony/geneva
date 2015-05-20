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

# This is a workaround for a CLion-problem -- see CPP270 in the JetBrains issue tracker
ADD_CUSTOM_TARGET(CLION_ALL_COURTIER_HEADERS SOURCES ${COMMUNICATIONINCLUDES})

INSTALL ( FILES ${COMMUNICATIONINCLUDES} DESTINATION ${INSTALL_PREFIX_INCLUDES}/courtier )

IF ( GENEVA_WITH_MPI )
	ADD_SUBDIRECTORY( mpi )
ENDIF ()
