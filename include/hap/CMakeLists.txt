CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET ( HAPINCLUDES
	GHapEnums.hpp
    GRandomDistributionsT.hpp
	GRandomBase.hpp
	GRandomDefines.hpp
	GRandomFactory.hpp
	GRandomT.hpp
)

# This is a workaround for a CLion-problem -- see CPP270 in the JetBrains issue tracker
ADD_CUSTOM_TARGET(CLION_ALL_HAP_HEADERS SOURCES ${HAPINCLUDES})

INSTALL ( FILES ${HAPINCLUDES} DESTINATION ${INSTALL_PREFIX_INCLUDES}/hap )
