CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET ( CONFIGFILES
	configFile.json
)

# The default action of this example is creating the configuration file,
# so do not copy the config files to the build tree or it will fail
#FILE ( COPY ${CONFIGFILES} DESTINATION . )

INSTALL ( FILES ${CONFIGFILES} DESTINATION ${INSTALL_PREFIX_DATA}/examples/${EXECUTABLENAME}/config )
