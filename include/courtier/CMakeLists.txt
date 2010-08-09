CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

SET ( COMMUNICATIONINCLUDES
	GAsioHelperFunctions.hpp
	GAsioTCPClientT.hpp
	GAsioTCPConsumerT.hpp
	GBaseClientT.hpp
	GBoostThreadConsumerT.hpp
	GBrokerT.hpp
	GBufferPortT.hpp
	GCommunicationEnums.hpp
	GConsumer.hpp
)

INSTALL ( FILES ${COMMUNICATIONINCLUDES} DESTINATION include/courtier
	PERMISSIONS
	OWNER_READ OWNER_WRITE
	GROUP_READ
	WORLD_READ
)
