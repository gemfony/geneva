AUTOMAKE_OPTIONS = foreign
DX_CONFIG = doxygen.cfg
include aminclude.am
ACLOCAL_AMFLAGS = -I m4

DISTCLEANFILES = Makefile aclocal.m4 config.h config.h.in config.log  config.guess config.sub
DISTCLEANFILES += depcomp config.status install-sh ltmain.sh missing 
DISTCLEANDIRS = autom4te.cache doc

DIST_SUBDIRS= src tests/GenEvATestCase1 tests/GenEvATestCase2 tests/GenEvATestCase3 tests/GenEvATestCase4 
#examples_SUBDIRS= tests/GenEvATestCase1 tests/GenEvATestCase2 tests/GenEvATestCase3 tests/GenEvATestCase4
SUBDIRS = src  tests/GenEvATestCase1 tests/GenEvATestCase2 tests/GenEvATestCase3 tests/GenEvATestCase4  
INCLUDES = -I include  -I $(BOOST_CPPFLAGS) -I tests/GenEvATestCase1 -I tests/GenEvATestCase2-I tests/GenEvATestCase3 tests/GenEvATestCase4 
EXTRA_DIST = autogen.sh $(DX_CONFIG) doc/html include license m4/boost.m4 
AM_LDFLAGS = -shared
#examples:   $(examples_SUBDIRS)  

include_HEADERS= include/GAsioTCPClient.hpp include/GBoostThreadGeneral.hpp include/GException.hpp include/GMemberCarrier.hpp include/GTemplateBitValue.hpp include/GAsioTCPConsumer.hpp include/GBoostThreadPopulation.hpp include/GHelperFunctions.hpp include/GMember.hpp include/GTemplateHelperFunctions.hpp include/GBaseClient.hpp include/GBoundary.hpp include/GLogger.hpp include/GMutable.hpp include/GTemplateValueCollection.hpp include/GBasePopulation.hpp include/GBoundedBuffer.hpp include/GLogStreamer.hpp include/GObject.hpp include/GTemplateValue.hpp include/GBiBuffer.hpp include/GConsumer.hpp include/GLogTargets.hpp include/GRandom.hpp include/GTransferPopulation.hpp include/GBitCollection.hpp include/GDoubleCollection.hpp include/GLowBitsAdaptor.hpp include/GRange.hpp include/GBitFlipAdaptor.hpp include/GDoubleGaussAdaptor.hpp include/GManagedMemberCollection.hpp include/GServerSession.hpp include/GBit.hpp include/GDouble.hpp include/GManipulator.hpp include/GSimpleMemberAdaptor.hpp include/GBoostThreadConsumer.hpp include/GEnums.hpp include/GMemberBroker.hpp include/GTemplateAdaptor.hpp include/GRateable.hpp
