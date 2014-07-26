CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET ( CMAKEFILES
	COPYING-CMAKE-SCRIPTS
	FindBoost.cmake
	FindGeneva.cmake
	# Other macro files
	CleanCmakeTemporaries.cmake
	IdentifySystemParameters.cmake
	IndependentBuild.cmake
)

INSTALL ( FILES ${CMAKEFILES} DESTINATION CMakeModules
	PERMISSIONS
	OWNER_READ OWNER_WRITE
	GROUP_READ
	WORLD_READ
)
