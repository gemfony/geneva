CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET ( UTILITYFILES
	GEqualityPrinter.hpp
	GStandard_test.hpp
)

INSTALL ( FILES ${UTILITYFILES} DESTINATION tests/geneva/UnitTests/UtilityHeaders )
