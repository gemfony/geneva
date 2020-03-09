CMAKE_MINIMUM_REQUIRED(VERSION 3.1)

SET ( FIGUREDATA
        ml.ppm
        ml_small192.ppm
        ml_small256.ppm
        squareTriangle.ppm
        )

# Copy the config files to the build tree for easier testing
FILE ( COPY ${FIGUREDATA} DESTINATION . )

INSTALL ( FILES ${FIGUREDATA} DESTINATION ${INSTALL_PREFIX_DATA}/examples/${EXECUTABLENAME}/pictures )
