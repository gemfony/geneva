cmake_minimum_required(VERSION 2.6)

SET ( GENEVASOURCES
GBasePopulation
GBoostThreadPopulation
GBoundary
GBoundedDouble
GDoubleCollection 
GDoubleGaussAdaptor
GEvaluator
GIndividual
GIndividualSet
GLogger
GLogTargets
GObject
GParameterBase
GParameterSet
GRandom
GRange
)

SET ( GENEVAINCLUDEDIRS
	../include
)

add_library ( geneva SHARED ${GENEVASOURCES} )

include_directories ( 
${GENEVAINCLUDEDIRS} 
${Boost_INCLUDE_DIRS}
)
