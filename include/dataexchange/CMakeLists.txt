CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

SET ( DATAEXCHANGEINCLUDES
	GBoolParameter.hpp
	GDataExchange.hpp
	GDataExchangeException.hpp
	GDoubleParameter.hpp
	GLongParameter.hpp
	GNumericParameterT.hpp
	GParameterValuePair.hpp
)

INSTALL ( FILES ${DATAEXCHANGEINCLUDES} DESTINATION include/dataexchange
	PERMISSIONS
	OWNER_READ OWNER_WRITE
	GROUP_READ
	WORLD_READ
)
