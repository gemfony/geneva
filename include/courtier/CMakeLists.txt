CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET ( COMMUNICATIONINCLUDES
    GBaseClientT.hpp
	GAsioHelperFunctions.hpp
    GAsioSerialTCPConsumerT.hpp
	GBaseConsumerT.hpp
    GBoostThreadConsumerT.hpp
    GExecutorT.hpp
	GBrokerT.hpp
	GBufferPortT.hpp
	GCourtierEnums.hpp
    GProcessingContainerT.hpp
    GSerialSubmissionClientT.hpp
	GSerialConsumerT.hpp
    GWebSocketConsumerT.hpp
)

# This is a workaround for a CLion-problem -- see CPP270 in the JetBrains issue tracker
ADD_CUSTOM_TARGET(CLION_ALL_COURTIER_HEADERS SOURCES ${COMMUNICATIONINCLUDES})

INSTALL ( FILES ${COMMUNICATIONINCLUDES} DESTINATION ${INSTALL_PREFIX_INCLUDES}/courtier )
