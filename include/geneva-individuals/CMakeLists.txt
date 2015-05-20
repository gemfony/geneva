CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET ( SAMPLEINDIVIDUALINCLUDES
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

# Copy the config files to the build tree for easier testing
FILE ( COPY ${SAMPLEINDIVIDUALCONFIGS} DESTINATION . )

INSTALL ( FILES ${SAMPLEINDIVIDUALINCLUDES} DESTINATION ${INSTALL_PREFIX_INCLUDES}/geneva-individuals )

INSTALL ( FILES ${SAMPLEINDIVIDUALCONFIGS} DESTINATION ${INSTALL_PREFIX_INCLUDES}/geneva-individuals )
