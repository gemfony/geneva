/**
 * @file GParameterScanFactory.cpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) eu
 *
 * This file is part of the Geneva library collection.
 *
 * Geneva was developed with kind support from Karlsruhe Institute of
 * Technology (KIT) and Steinbuch Centre for Computing (SCC). Further
 * information about KIT and SCC can be found at http://www.kit.edu/english
 * and http://scc.kit.edu .
 *
 * Geneva is free software: you can redistribute and/or modify it under
 * the terms of version 3 of the GNU Affero General Public License
 * as published by the Free Software Foundation.
 *
 * Geneva is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with the Geneva library. If not, see <http://www.gnu.org/licenses/>.
 *
 * For further information on Gemfony scientific and Geneva, visit
 * http://www.gemfony.eu .
 */

#include "geneva/GParameterScanFactory.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
/** A short identifier suitable for storage in a std::map */
const std::string GParameterScanFactory::nickname = "ps";

/******************************************************************************/
/** This will register the factory in the global factory store */
GOAInitializerT<GParameterScanFactory> GPSStoreRegistrant;

/******************************************************************************/
/**
 * The default constructor
 */
GParameterScanFactory::GParameterScanFactory()
   : GOptimizationAlgorithmFactoryT<GOptimizationAlgorithmT<GParameterSet> >("./config/GParameterScan.json")
   , parameterSpec_("empty")
   , parameterSpecCL_("empty")
{ /* nothing */ }

/******************************************************************************/
/**
 * Initialization with the name of the config file and the default parallelization mode
 */
GParameterScanFactory::GParameterScanFactory(
   const std::string& configFile
)
   : GOptimizationAlgorithmFactoryT<GOptimizationAlgorithmT<GParameterSet> >(configFile)
   , parameterSpec_("empty")
   , parameterSpecCL_("empty")
{ /* nothing */ }

/******************************************************************************/
/**
 * A constructor with the ability to switch the parallelization mode. It initializes a
 * target item as needed.
 */
GParameterScanFactory::GParameterScanFactory(
   const std::string& configFile
   , const execMode& pm
)
   : GOptimizationAlgorithmFactoryT<GOptimizationAlgorithmT<GParameterSet> >(configFile, pm)
   , parameterSpec_("empty")
   , parameterSpecCL_("empty")
{ /* nothing */ }

/******************************************************************************/
/**
 * A constructor with the ability to switch the parallelization mode and
 * to add a content creator. It initializes a target item as needed.
 */
GParameterScanFactory::GParameterScanFactory(
   const std::string& configFile
   , const execMode& pm
   , boost::shared_ptr<Gem::Common::GFactoryT<GParameterSet> > contentCreatorPtr
)
   : GOptimizationAlgorithmFactoryT<GOptimizationAlgorithmT<GParameterSet> >(configFile, pm, contentCreatorPtr)
   , parameterSpec_("empty")
   , parameterSpecCL_("empty")
{ /* nothing */ }

/******************************************************************************/
/**
 * The destructor
 */
GParameterScanFactory::~GParameterScanFactory()
{ /* nothing */ }

/******************************************************************************/
/**
 * Gives access to the mnemonics / nickname describing an algorithm
 */
std::string GParameterScanFactory::getMnemonic() const {
   return GParameterScanFactory::nickname;
}

/******************************************************************************/
/**
 * Gives access to a clear-text description of the algorithm
 */
std::string GParameterScanFactory::getAlgorithmName() const {
   return std::string("Parameter-Scan");
}

/******************************************************************************/
/**
 * Allows to specify the parameter settings manually for variables to be scanned
 */
void GParameterScanFactory::setParameterSpecs(std::string parameterSpecCL) {
   if(parameterSpecCL != "empty" && !parameterSpecCL.empty()) {
      parameterSpecCL_ = parameterSpecCL;
   }
}

/******************************************************************************/
/**
 * Allows to retrieve the parameter settings for variables to be scanned
 */
std::string GParameterScanFactory::getParameterSpecs() const {
   if(parameterSpecCL_ != "empty" && !parameterSpecCL_.empty()) {
      return parameterSpecCL_;
   } else {
      return parameterSpec_;
   }
}

/******************************************************************************/
/**
 * Allows to reset the parameter specs
 */
void GParameterScanFactory::resetParameterSpecs() {
   parameterSpecCL_ = "empty";
   parameterSpec_   = "empty";
}

/******************************************************************************/
/**
 * Creates items of this type
 *
 * @return Items of the desired type
 */
boost::shared_ptr<GOptimizationAlgorithmT<GParameterSet> > GParameterScanFactory::getObject_(
   Gem::Common::GParserBuilder& gpb
   , const std::size_t& id
) {
   // Will hold the result
   boost::shared_ptr<GBasePS> target;

   // Fill the target pointer as required
   switch(pm_) {
   case EXECMODE_SERIAL:
      target = boost::shared_ptr<GSerialPS>(new GSerialPS());
      break;

   case EXECMODE_MULTITHREADED:
      target = boost::shared_ptr<GMultiThreadedPS>(new GMultiThreadedPS());
      break;

   case EXECMODE_BROKERAGE:
      target = boost::shared_ptr<GBrokerPS>(new GBrokerPS());
      break;
   }

   // Make the local configuration options known
   target->GBasePS::addConfigurationOptions(gpb, true);

   return target;
}

/***************************************************************************/
/**
 * Adds local command line options to a boost::program_options::options_description object.
 *
 * @param visible Command line options that should always be visible
 * @param hidden Command line options that should only be visible upon request
 */
void GParameterScanFactory::addCLOptions(
   boost::program_options::options_description& visible
   , boost::program_options::options_description& hidden
) {
   namespace po = boost::program_options;

   visible.add_options() (
      "parameterSpec"
      , po::value<std::string>(&parameterSpecCL_)->default_value(std::string("empty"))
      , "\t[GParameterScanFactory] Specification of parameters to be scanned. Syntax: \"d(0, -10., 10., 100)\". Use a comma-separated list for more than one variable. A single entry \"s(1000)\" will lead to a random scan over all parameters of up to 1000 individuals"
   )
   ;

   // Add the parent class'es options
   GOptimizationAlgorithmFactoryT<GOptimizationAlgorithmT<GParameterSet> >::addCLOptions(visible, hidden);
}

/******************************************************************************/
/**
 * Allows to describe local configuration options for gradient descents
 */
void GParameterScanFactory::describeLocalOptions_(Gem::Common::GParserBuilder& gpb) {
   // Describe our own options
   using namespace Gem::Courtier;

   std::string comment;

   // local data
   comment = ""; // Reset the comment string
   comment += "Specification of the parameters to be used;";
   comment += "in the parameter scan.;";
   gpb.registerFileParameter<std::string>(
      "parameterOptions"
      , parameterSpec_
      , std::string("d(0, -10., 10., 100), d(1, -10., 10., 100)")
      , Gem::Common::VAR_IS_ESSENTIAL // Could also be VAR_IS_SECONDARY
      , comment
   );

   // Allow our parent class to describe its options
   GOptimizationAlgorithmFactoryT<GOptimizationAlgorithmT<GParameterSet> >::describeLocalOptions_(gpb);
}

/******************************************************************************/
/**
 * Allows to act on the configuration options received from the configuration file. Here
 * we can add the options described in describeLocalOptions to the object.
 *
 * @param p A smart-pointer to be acted on during post-processing
 */
void GParameterScanFactory::postProcess_(boost::shared_ptr<GOptimizationAlgorithmT<GParameterSet> >& p_base) {
   // Convert the object to the correct target type
   switch(pm_) {
   case EXECMODE_SERIAL:
      {
         boost::shared_ptr<GSerialPS> p
            = Gem::Common::convertSmartPointer<GOptimizationAlgorithmT<GParameterSet>, GSerialPS>(p_base);
         p->setParameterSpecs(this->getParameterSpecs());
      }
      break;

   case EXECMODE_MULTITHREADED:
      {
         boost::shared_ptr<GMultiThreadedPS> p
            = Gem::Common::convertSmartPointer<GOptimizationAlgorithmT<GParameterSet>, GMultiThreadedPS>(p_base);
         p->setNThreads(GOptimizationAlgorithmFactoryT<GOptimizationAlgorithmT<GParameterSet> >::nEvaluationThreads_);
         p->setParameterSpecs(this->getParameterSpecs());
      }
      break;

   case EXECMODE_BROKERAGE:
      {
         boost::shared_ptr<GBrokerPS> p
            = Gem::Common::convertSmartPointer<GOptimizationAlgorithmT<GParameterSet>, GBrokerPS>(p_base);

         p->doLogging(GOptimizationAlgorithmFactoryT<GOptimizationAlgorithmT<GParameterSet> >::doLogging_);
         p->setWaitFactor(GOptimizationAlgorithmFactoryT<GOptimizationAlgorithmT<GParameterSet> >::waitFactor_);
         p->setParameterSpecs(this->getParameterSpecs());
      }
      break;
   }

   // Call our parent class'es function
   GOptimizationAlgorithmFactoryT<GOptimizationAlgorithmT<GParameterSet> >::postProcess_(p_base);
}

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */
