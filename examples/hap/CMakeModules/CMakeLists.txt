CMAKE_MINIMUM_REQUIRED(VERSION 3.1)

SET ( CMAKEFILES
	README
	CommonGenevaBuild.cmake
	FindGeneva.cmake
	# Other macro files
	CleanCmakeTemporaries.cmake
	IdentifySystemParameters.cmake
	IndependentBuild.cmake
)

INSTALL ( FILES ${CMAKEFILES} DESTINATION ${INSTALL_PREFIX_DATA}/CMakeModules )
