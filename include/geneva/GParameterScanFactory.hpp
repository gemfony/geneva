/**
 * @file GParameterScanFactory.hpp
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

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard header files go here
#include <string>

// Boost header files go here
#include <boost/date_time.hpp>
#include <boost/filesystem.hpp>
#include <boost/function.hpp>

#ifndef GPARAMETERSCANFACTORY_HPP_
#define GPARAMETERSCANFACTORY_HPP_

// Geneva headers go here
#include "courtier/GCourtierEnums.hpp"
#include "geneva/GOptimizationAlgorithmFactoryT.hpp"
#include "geneva/GBasePS.hpp"
#include "geneva/GSerialPS.hpp"
#include "geneva/GMultiThreadedPS.hpp"
#include "geneva/GBrokerPS.hpp"
#include "geneva/GOAInitializerT.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * This class is a specialization of the GFactoryT<> class for gradient descents.
 */
class GParameterScanFactory
   : public GOptimizationAlgorithmFactoryT<GOptimizationAlgorithmT<GParameterSet> >
{
public:
   /** @brief An easy identifier for the class */
   static G_API_GENEVA const std::string nickname; // Initialized in the .cpp definition file

   /** @brief The default constructor */
   G_API_GENEVA GParameterScanFactory();
   /** @brief Initialization with the name of the config file and the default parallelization mode */
   explicit G_API_GENEVA GParameterScanFactory(const std::string&);
   /** @brief The standard constructor */
   G_API_GENEVA GParameterScanFactory(
      const std::string&
      , const execMode&
   );
   /** @brief A constructor that also adds a content creation function */
   G_API_GENEVA GParameterScanFactory(
      const std::string&
      , const execMode&
      , boost::shared_ptr<Gem::Common::GFactoryT<GParameterSet> >
   );
   /** @brief The destructor */
   virtual G_API_GENEVA ~GParameterScanFactory();

   /** @brief Adds local command line options to boost::program_options::options_description objects */
   virtual G_API_GENEVA void addCLOptions(
      boost::program_options::options_description&
      , boost::program_options::options_description&
   ) override;

   /** @brief Gives access to the mnemonics / nickname describing an algorithm */
   virtual G_API_GENEVA std::string getMnemonic() const override;
   /** @brief Gives access to a clear-text description of the algorithm */
   virtual G_API_GENEVA std::string getAlgorithmName() const override;

   /** @brief Allows to specify the parameter settings manually for variables to be scanned */
   G_API_GENEVA void setParameterSpecs(std::string parStr);
   /** @brief Allows to retrieve the parameter settings for variables to be scanned */
   G_API_GENEVA std::string getParameterSpecs() const;
   /** @brief Allows to reset the parameter specs */
   G_API_GENEVA void resetParameterSpecs();

protected:
   /** @brief Creates individuals of this type */
   virtual G_API_GENEVA boost::shared_ptr<GOptimizationAlgorithmT<GParameterSet> > getObject_(Gem::Common::GParserBuilder&, const std::size_t&) override;
   /** @brief Allows to describe local configuration options in derived classes */
   virtual G_API_GENEVA void describeLocalOptions_(Gem::Common::GParserBuilder&) override;
   /** @brief Allows to act on the configuration options received from the configuration file */
   virtual G_API_GENEVA void postProcess_(boost::shared_ptr<GOptimizationAlgorithmT<GParameterSet> >&) override;

private:
   std::string parameterSpec_;   ///< Holds information on the variables to be optimized -- set through the configuration file
   std::string parameterSpecCL_; ///< Holds information on the variables to be optimized -- set through the corresponding member function or on the command line
};

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

#endif /* GPARAMETERSCANFACTORY_HPP_ */
