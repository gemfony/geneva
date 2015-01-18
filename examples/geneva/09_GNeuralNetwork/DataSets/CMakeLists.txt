CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET ( DATAFILES
	axis_centric.dat.C
	axis_centric.dat	
	hyper_cube.dat.C
	hyper_cube.dat
	hyper_sphere.dat.C
	hyper_sphere.dat
	sinus.dat.C
	sinus.dat
)

# Copy the config files to the build tree for easier testing
FOREACH ( file IN LISTS DATAFILES )
	CONFIGURE_FILE ( ${file} ${file} COPYONLY )
ENDFOREACH ()

INSTALL ( FILES ${DATAFILES} DESTINATION ${INSTALL_PREFIX_DATA}/examples/${EXECUTABLENAME}/DataSets )
