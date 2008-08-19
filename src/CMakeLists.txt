CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

SET ( GENEVASOURCES
	GBasePopulation
	GBit
	GBitCollection
	GBitFlipAdaptor
	GBoostThreadConsumer
	GBoostThreadPopulation
	GBoundedDouble
	GBrokerPopulation
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

ADD_LIBRARY ( ${PROJECT_NAME} SHARED ${GENEVASOURCES} )

INSTALL ( TARGETS ${PROJECT_NAME} DESTINATION lib
	PERMISSIONS
	OWNER_READ OWNER_WRITE
	GROUP_READ
	WORLD_READ
)
