CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET ( CMAKEFILES
	README
	COPYING-CMAKE-SCRIPTS
	CommonGenevaBuild.cmake
	FindBoost.cmake
	FindGeneva.cmake
	# Other macro files
	CleanCmakeTemporaries.cmake
	IdentifySystemParameters.cmake
	IndependentBuild.cmake
)

INSTALL ( FILES ${CMAKEFILES} DESTINATION CMakeModules )
