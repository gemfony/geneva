CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

SET ( COMMUNICATIONINCLUDES
	GAsioHelperFunctions.hpp
	GAsioTCPClientT.hpp
	GAsioTCPConsumerT.hpp
	GBaseClientT.hpp
	GBoostThreadConsumerT.hpp
	GBoundedBufferWithIdT.hpp
	GBrokerT.hpp
	GBufferPortT.hpp
	GConsumer.hpp
)

INSTALL ( FILES ${COMMUNICATIONINCLUDES} DESTINATION include/communication
	PERMISSIONS
	OWNER_READ OWNER_WRITE
	GROUP_READ
	WORLD_READ
)
