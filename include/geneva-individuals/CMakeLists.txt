SET ( SAMPLEINDIVIDUALINCLUDES
    GDelayIndividual.hpp
	GExternalEvaluatorIndividual.hpp
	GFunctionIndividual.hpp
	GLineFitIndividual.hpp
	GMetaOptimizerIndividualT.hpp
	GNeuralNetworkIndividual.hpp
	GTestIndividual2.hpp
	GTestIndividual3.hpp
)

SET ( SAMPLEINDIVIDUALCONFIGS
	# No config files at the moment
)

# This is a workaround for a CLion-problem -- see CPP270 in the JetBrains issue tracker
ADD_CUSTOM_TARGET(CLION_ALL_GENEVAINDIVIDUAL_HEADERS SOURCES ${SAMPLEINDIVIDUALINCLUDES})

SET_TARGET_PROPERTIES(
    CLION_ALL_GENEVAINDIVIDUAL_HEADERS
    PROPERTIES
	CXX_STANDARD 17
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS OFF
)


# Copy the config files to the build tree for easier testing
FILE ( COPY ${SAMPLEINDIVIDUALCONFIGS} DESTINATION . )

INSTALL ( FILES ${SAMPLEINDIVIDUALINCLUDES} DESTINATION ${INSTALL_PREFIX_INCLUDES}/geneva-individuals )

INSTALL ( FILES ${SAMPLEINDIVIDUALCONFIGS} DESTINATION ${INSTALL_PREFIX_INCLUDES}/geneva-individuals )
