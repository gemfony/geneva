CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

SET ( GENEVASOURCES
	GAsioHelperFunctions
	GAsioTCPClient
	GAsioTCPConsumer
	GBaseClient
	GBoolean
	GBooleanAdaptor
	GBooleanCollection
	GBooleanObjectCollection
	GBoostThreadConsumer
	GBoundedDouble
	GBoundedDoubleCollection
	GBoundedInt32
	GBoundedInt32Collection
	GBoundedNumT
	GBrokerEA
	GDouble
	GDoubleCollection 
	GDoubleGaussAdaptor
	GDoubleObjectCollection
	GEAPersonalityTraits
	GEvolutionaryAlgorithm
	GGDPersonalityTraits
	GIdentityAdaptorT
	GIndividual
	GInt32
	GInt32Collection
	GInt32FlipAdaptor
	GInt32GaussAdaptor
	GInt32ObjectCollection
	GMultiThreadedEA
	GNumCollectionT
	GObject
	GOptimizationAlgorithm
	GParameterBase
	GParameterBaseWithAdaptorsT
	GParameterSet
	GPersonalityTraits
	GPODExpectationChecksT
	GRandom
	GRandomFactory
	GSerializationHelperFunctions
#	GSwarm
	GSwarmAdaptor
	GSwarmPersonalityTraits
	GThreadGroup
)

SET ( DATAEXCHANGESOURCES
	GBoolParameter
	GDataExchange
	GDataExchangeException
	GDoubleParameter
	GLongParameter
	GNumericParameterT
	GParameterValuePair
)

# BUILD_SHARED_LIBS controls the behavior of ADD_LIBRARY if STATIC/SHARED omitted
IF( GENEVA_STATIC )
	ADD_LIBRARY ( ${GENEVALIBNAME} STATIC ${GENEVASOURCES} )
	ADD_LIBRARY ( ${DATAEXCHANGELIBNAME} STATIC ${DATAEXCHANGESOURCES} )
ELSE()
	ADD_LIBRARY ( ${GENEVALIBNAME} SHARED ${GENEVASOURCES} )
	ADD_LIBRARY ( ${DATAEXCHANGELIBNAME} SHARED ${DATAEXCHANGESOURCES} )
ENDIF( GENEVA_STATIC )

SET_TARGET_PROPERTIES(${PROJECT_NAME}
	PROPERTIES VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}
)

INSTALL ( TARGETS ${GENEVALIBNAME} ${DATAEXCHANGELIBNAME} DESTINATION lib
	PERMISSIONS
	OWNER_READ OWNER_WRITE
	GROUP_READ
	WORLD_READ
)
