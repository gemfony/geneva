CMAKE_MINIMUM_REQUIRED (VERSION 2.6)

SET ( DATASETS
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
IF( NOT OLD_CMAKE )
	FOREACH ( file IN LISTS DATASETS )
		CONFIGURE_FILE ( ${file} . COPYONLY )
	ENDFOREACH ()
ENDIF()

INSTALL ( FILES ${DATASETS} DESTINATION examples/${PROJECT_NAME}/DataSets
	PERMISSIONS
	OWNER_READ OWNER_WRITE
	GROUP_READ
	WORLD_READ
)
