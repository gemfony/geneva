CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

ADD_CUSTOM_TARGET ( "geneva-tests-manual" )

ADD_DEPENDENCIES ( "geneva-tests-manual"
	# Add all the available tests targets (EXECUTABLENAMEs) here
	GBooleanProbabilityTest
	GConstrainedFPTTest
	GConstrainedIntTTest
	GFloatBiGaussAdaptorTest
	GFloatGaussAdaptorTest
	GDoubleBiGaussAdaptorTest
	GDoubleGaussAdaptorTest
)

ADD_SUBDIRECTORY ( GBooleanProbabilityTest )
ADD_SUBDIRECTORY ( GConstrainedFPTTest )
ADD_SUBDIRECTORY ( GConstrainedIntegerTTest )
ADD_SUBDIRECTORY ( GFloatBiGaussAdaptorTest )
ADD_SUBDIRECTORY ( GFloatGaussAdaptorTest )
ADD_SUBDIRECTORY ( GDoubleBiGaussAdaptorTest )
ADD_SUBDIRECTORY ( GDoubleGaussAdaptorTest )
