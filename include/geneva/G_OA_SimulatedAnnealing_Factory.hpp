/**
 * @file G_OA_SimulatedAnnealing_Factory.hpp
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
#include <boost/filesystem.hpp>

#ifndef G_OA_SIMULATEDANNEALINGFACTORY_HPP
#define G_OA_SIMULATEDANNEALINGFACTORY_HPP

// Geneva headers go here
#include "courtier/GCourtierEnums.hpp"
#include "geneva/G_OA_FactoryT.hpp"
#include "geneva/G_OA_BaseT.hpp"
#include "geneva/GParameterSet.hpp"
#include "geneva/G_OA_SimulatedAnnealing.hpp"
#include "geneva/G_OA_InitializerT.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class is a specialization of the GFactoryT<> class for simulated annealing.
 * It will only return objects which perform all evaluation through the broker.
 */
class GSimulatedAnnealingFactory
	: public G_OA_FactoryT<G_OA_BaseT<Gem::Courtier::GBrokerExecutorT<GParameterSet>>>
{
public:
	 /** @brief The default constructor */
	 G_API_GENEVA GSimulatedAnnealingFactory();
	 /** @brief Initialization with the name of the config file */
	 explicit G_API_GENEVA GSimulatedAnnealingFactory(const std::string&);
	 /** @brief Initialization with the name of the config file and a content creator */
	 G_API_GENEVA GSimulatedAnnealingFactory(
		 const std::string&
		 , std::shared_ptr <Gem::Common::GFactoryT<GParameterSet>>
	 );
	 /** @brief The copy constructor */
	 G_API_GENEVA GSimulatedAnnealingFactory(const GSimulatedAnnealingFactory&);
	 /** @brief The destructor */
	 virtual G_API_GENEVA ~GSimulatedAnnealingFactory();

	 /** @brief Gives access to the mnemonics / nickname describing an algorithm */
	 virtual G_API_GENEVA std::string getMnemonic() const override;
	 /** @brief Gives access to a clear-text description of the algorithm */
	 virtual G_API_GENEVA std::string getAlgorithmName() const override;

protected:
	 /** @brief Creates individuals of this type */
	 virtual G_API_GENEVA std::shared_ptr<G_OA_BaseT<Gem::Courtier::GBrokerExecutorT<GParameterSet>>> getObject_(
		 Gem::Common::GParserBuilder&
		 , const std::size_t&
	 ) override;
	 /** @brief Allows to act on the configuration options received from the configuration file */
	 virtual G_API_GENEVA void postProcess_(std::shared_ptr<G_OA_BaseT<Gem::Courtier::GBrokerExecutorT<GParameterSet>>>&) override;
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class is a specialization of the GFactoryT<> class for simulated annealing.
 * It will only return objects which perform all evaluation through the broker.
 */
class G_MT_SimulatedAnnealingFactory
	: public G_OA_FactoryT<G_OA_BaseT<Gem::Courtier::GMTExecutorT<GParameterSet>>>
{
public:
	 /** @brief The default constructor */
	 G_API_GENEVA G_MT_SimulatedAnnealingFactory();
	 /** @brief Initialization with the name of the config file */
	 explicit G_API_GENEVA G_MT_SimulatedAnnealingFactory(const std::string&);
	 /** @brief Initialization with the name of the config file and a content creator */
	 G_API_GENEVA G_MT_SimulatedAnnealingFactory(
		 const std::string&
		 , std::shared_ptr <Gem::Common::GFactoryT<GParameterSet>>
	 );
	 /** @brief The copy constructor */
	 G_API_GENEVA G_MT_SimulatedAnnealingFactory(const G_MT_SimulatedAnnealingFactory&);
	 /** @brief The destructor */
	 virtual G_API_GENEVA ~G_MT_SimulatedAnnealingFactory();

	 /** @brief Gives access to the mnemonics / nickname describing an algorithm */
	 virtual G_API_GENEVA std::string getMnemonic() const override;
	 /** @brief Gives access to a clear-text description of the algorithm */
	 virtual G_API_GENEVA std::string getAlgorithmName() const override;

protected:
	 /** @brief Creates individuals of this type */
	 virtual G_API_GENEVA std::shared_ptr<G_OA_BaseT<Gem::Courtier::GMTExecutorT<GParameterSet>>> getObject_(
		 Gem::Common::GParserBuilder&
		 , const std::size_t&
	 ) override;
	 /** @brief Allows to act on the configuration options received from the configuration file */
	 virtual G_API_GENEVA void postProcess_(std::shared_ptr<G_OA_BaseT<Gem::Courtier::GMTExecutorT<GParameterSet>>>&) override;
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class is a specialization of the GFactoryT<> class for simulated annealing.
 * It will only return objects which perform all evaluation through the broker.
 */
class G_Serial_SimulatedAnnealingFactory
	: public G_OA_FactoryT<G_OA_BaseT<Gem::Courtier::GSerialExecutorT<GParameterSet>>>
{
public:
	 /** @brief The default constructor */
	 G_API_GENEVA G_Serial_SimulatedAnnealingFactory();
	 /** @brief Initialization with the name of the config file */
	 explicit G_API_GENEVA G_Serial_SimulatedAnnealingFactory(const std::string&);
	 /** @brief Initialization with the name of the config file and a content creator */
	 G_API_GENEVA G_Serial_SimulatedAnnealingFactory(
		 const std::string&
		 , std::shared_ptr <Gem::Common::GFactoryT<GParameterSet>>
	 );
	 /** @brief The copy constructor */
	 G_API_GENEVA G_Serial_SimulatedAnnealingFactory(const G_Serial_SimulatedAnnealingFactory&);
	 /** @brief The destructor */
	 virtual G_API_GENEVA ~G_Serial_SimulatedAnnealingFactory();

	 /** @brief Gives access to the mnemonics / nickname describing an algorithm */
	 virtual G_API_GENEVA std::string getMnemonic() const override;
	 /** @brief Gives access to a clear-text description of the algorithm */
	 virtual G_API_GENEVA std::string getAlgorithmName() const override;

protected:
	 /** @brief Creates individuals of this type */
	 virtual G_API_GENEVA std::shared_ptr<G_OA_BaseT<Gem::Courtier::GSerialExecutorT<GParameterSet>>> getObject_(
		 Gem::Common::GParserBuilder&
		 , const std::size_t&
	 ) override;
	 /** @brief Allows to act on the configuration options received from the configuration file */
	 virtual G_API_GENEVA void postProcess_(std::shared_ptr<G_OA_BaseT<Gem::Courtier::GSerialExecutorT<GParameterSet>>>&) override;
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

#endif /* G_OA_SIMULATEDANNEALINGFACTORY_HPP */