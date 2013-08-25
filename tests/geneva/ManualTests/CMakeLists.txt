CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

ADD_CUSTOM_TARGET ( "geneva-tests-manual" )

ADD_DEPENDENCIES ( "geneva-tests-manual"
	# Add all the available tests targets (EXECUTABLENAMEs) here
	GBooleanProbabilityTest
	GConstrainedFPTTest
	GConstrainedIntTTest
	# GDataHierarchies
	GFloatBiGaussAdaptorTest
	GFloatGaussAdaptorTest
	GDoubleBiGaussAdaptorTest
	GDoubleGaussAdaptorTest
	GRandomWalk
)

ADD_SUBDIRECTORY ( GBooleanProbabilityTest )
ADD_SUBDIRECTORY ( GConstrainedFPTTest )
ADD_SUBDIRECTORY ( GConstrainedIntegerTTest )
# ADD_SUBDIRECTORY ( GDataHierarchies )
ADD_SUBDIRECTORY ( GFloatBiGaussAdaptorTest )
ADD_SUBDIRECTORY ( GFloatGaussAdaptorTest )
ADD_SUBDIRECTORY ( GDoubleBiGaussAdaptorTest )
ADD_SUBDIRECTORY ( GDoubleGaussAdaptorTest )
ADD_SUBDIRECTORY ( GRandomWalk )
