CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

SET (EXECUTABLENAME GUnitTests)

SET (${EXECUTABLENAME}_SRCS 
	GUnitTests
	GAdaptorT_test
	GBasePopulation_test
	GBoolean_test
	GBooleanAdaptor_test
	GBooleanCollection_test
	GBoostThreadPopulation_test
	GBoundedDouble_test
	GBoundedDoubleCollection_test
	GBrokerPopulation_test
	GChar_test
	GDataExchange_test
	GDoubleCollection_test
	GGaussAdaptorT_test
	GInt32Collection_test
	GIntFlipAdaptorT_test
	GLogger_test
	GObject_test
	GParameterBaseWithAdaptorsT_test
	GParameterSet_test
)
	
ADD_EXECUTABLE(${EXECUTABLENAME}
	${${EXECUTABLENAME}_SRCS}
)

INCLUDE_DIRECTORIES (AFTER
	${PROJECT_SOURCE_DIR}/examples/SampleIndividuals
)

TARGET_LINK_LIBRARIES (${EXECUTABLENAME}
	${PROJECT_NAME}
	${Boost_LIBRARIES}
)

INSTALL ( TARGETS ${EXECUTABLENAME} DESTINATION tests/unittests
	PERMISSIONS
	OWNER_READ OWNER_WRITE OWNER_EXECUTE
	GROUP_READ GROUP_EXECUTE
	WORLD_READ WORLD_EXECUTE
)
