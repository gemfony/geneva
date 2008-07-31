cmake_minimum_required(VERSION 2.6)

SET ( GENEVASOURCES
GBasePopulation
GBoostThreadPopulation
GBoundary
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
)

SET ( GENEVAINCLUDEDIRS
	../include
)

add_library ( geneva SHARED ${GENEVASOURCES} )

IF(UNIX)
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
ENDIF(UNIX)

include_directories ( 
${GENEVAINCLUDEDIRS} 
${Boost_INCLUDE_DIRS}
)
