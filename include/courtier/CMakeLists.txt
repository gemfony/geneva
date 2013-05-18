CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

SET ( COMMUNICATIONINCLUDES
	GAsioHelperFunctions.hpp
	GAsioTCPConsumerT.hpp
	GBaseClientT.hpp
	GBaseConsumerT.hpp
	GBoostThreadConsumerT.hpp
	GBrokerConnectorT.hpp
	GBrokerT.hpp
	GBufferPortT.hpp
	GCourtierEnums.hpp
	GSerialConsumerT.hpp
	GSubmissionContainerT.hpp
)

INSTALL ( FILES ${COMMUNICATIONINCLUDES} DESTINATION include/courtier
	PERMISSIONS
	OWNER_READ OWNER_WRITE
	GROUP_READ
	WORLD_READ
)
