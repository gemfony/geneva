CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

#
# Check the major/minor/patch strings configure in the top level CMakeLists file
#   and use them to define the GENEVAVERSION variable in GGlobalDefines.hpp
#
SET ( VERSION_PREFIX "0" )

IF ( NOT VERSION_MAJOR MATCHES "^[0-9]$" )
	MESSAGE ( AUTHOR_WARNING "Wrong major version number! (${VERSION_MAJOR})" )
ENDIF ( NOT VERSION_MAJOR MATCHES "^[0-9]$" )

IF ( NOT VERSION_MINOR MATCHES "^[0-9]$" )
	MESSAGE ( AUTHOR_WARNING "Wrong minor version number! (${VERSION_MINOR})" )
ENDIF ( NOT VERSION_MINOR MATCHES "^[0-9]$" )

# Allow additional sufixes like "rc1" in the patch number
IF ( NOT VERSION_PATCH MATCHES "^[0-9]" )
	MESSAGE ( AUTHOR_WARNING "Wrong patch version number! (${VERSION_PATCH})" )
ENDIF ( NOT VERSION_PATCH MATCHES "^[0-9]" )

# Keep the first character only (a number)
STRING ( SUBSTRING ${VERSION_PATCH} 0 1 VERSION_PATCH_SHORT )

SET ( GENEVA_API_VERSION ${VERSION_PREFIX}${VERSION_MAJOR}${VERSION_MINOR}${VERSION_PATCH_SHORT} )
CONFIGURE_FILE (
	GGlobalDefines.hpp.in ${PROJECT_SOURCE_DIR}/include/GGlobalDefines.hpp
	@ONLY
)

ADD_SUBDIRECTORY (thirdparty)

SET ( GENEVAINCLUDES
	GAsioHelperFunctions.hpp
	GAsioTCPClient.hpp
	GAsioTCPConsumer.hpp
	GAdaptorT.hpp
	GBaseClient.hpp
	GBoolean.hpp
	GBooleanAdaptor.hpp
	GBooleanCollection.hpp
	GBooleanObjectCollection.hpp
	GBoostThreadConsumer.hpp
	GBoundedBufferT.hpp
	GBoundedBufferWithIdT.hpp
	GBoundedDouble.hpp
	GBoundedDoubleCollection.hpp
	GBoundedInt32.hpp
	GBoundedInt32Collection.hpp
	GBoundedNumT.hpp
	GBrokerEA.hpp
	GBrokerT.hpp
	GBufferPortT.hpp
	GConsumer.hpp
	GDouble.hpp
	GDoubleCollection.hpp
	GDoubleGaussAdaptor.hpp
	GDoubleObjectCollection.hpp
	GEAPersonalityTraits.hpp
	GenevaExceptions.hpp
	GEnums.hpp
	GEvolutionaryAlgorithm.hpp
	GGaussAdaptorT.hpp
	GGDPersonalityTraits.hpp
	GGlobalDefines.hpp
	GGlobalOptionsT.hpp
	GHelperFunctionsT.hpp
	GIdentityAdaptorT.hpp
	GIndividualBroker.hpp
	GIndividual.hpp
	GInt32.hpp
	GInt32Collection.hpp
	GInt32FlipAdaptor.hpp
	GInt32GaussAdaptor.hpp
	GInt32ObjectCollection.hpp
	GIntFlipAdaptorT.hpp
	GMultiThreadedEA.hpp
	GMutableI.hpp
	GMutableSetT.hpp
	GNumCollectionT.hpp
	GObject.hpp
	GOptimizationAlgorithm.hpp
	GParameterBase.hpp
	GParameterBaseWithAdaptorsT.hpp
	GParameterCollectionT.hpp
	GParameterSet.hpp
	GParameterT.hpp
	GParameterTCollectionT.hpp
	GPersonalityTraits.hpp
	GPODExpectationChecksT.hpp
	GRandom.hpp
	GRandomFactory.hpp
	GRateableI.hpp
	GSeedManager.hpp
	GSerializableI.hpp
	GSerializationHelperFunctions.hpp
	GSingletonT.hpp
	GStdPtrVectorInterfaceT.hpp
	GStdSimpleVectorInterfaceT.hpp
	GUnitTestFrameworkT.hpp
#	GSwarm.hpp
	GSwarmAdaptor.hpp
	GSwarmPersonalityTraits.hpp
	GThreadGroup.hpp
)

SET ( DATAEXCHANGEINCLUDES
	GBoolParameter.hpp
	GDataExchange.hpp
	GDataExchangeException.hpp
	GDoubleParameter.hpp
	GLongParameter.hpp
	GNumericParameterT.hpp
	GParameterValuePair.hpp
)

INSTALL ( FILES ${GENEVAINCLUDES} ${DATAEXCHANGEINCLUDES} DESTINATION include 
	PERMISSIONS
	OWNER_READ OWNER_WRITE
	GROUP_READ
	WORLD_READ
)
