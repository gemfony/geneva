CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET ( HAPINCLUDES
	GHapEnums.hpp
	GRandomBase.hpp
	GRandomDefines.hpp
	GRandomFactory.hpp
	GRandomT.hpp
	GSeedManager.hpp
)

INSTALL ( FILES ${HAPINCLUDES} DESTINATION include/hap )
