/**
 * @file
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

#pragma once

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard headers go here
#include <set>
#include <string>

// Boost headers go here
#include <boost/serialization/serialization.hpp> // See last comment at https://svn.boost.org/trac/boost/ticket/12126 . Fixes "sole" inclusion of set.hpp
#include <boost/serialization/set.hpp>

// Geneva headers go here
#include "courtier/GExecutorT.hpp"
#include "common/GSerializableFunctionObjectT.hpp"
#include "geneva/GParameterSet.hpp"
#include "geneva/G_OptimizationAlgorithm_EvolutionaryAlgorithm_Factory.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A base class for post-optimizers holding information about which optimization
 * algorithms are eligible for post-processing of individuals. By default, all
 * algorithms are forbidden. Once allowed algorithms are registered through their
 * mnemonics, optimzation algorithms with these mnemonics are allowed. Registering
 * a mnemonic "all" will lead to all restrictions being lifted. This functionality
 * is required as not all optimization algorithms will react gracefully to a silent
 * change of their individuals (think e.g. of gradient descents). The mnemonics
 * must be accessible via the base_type-object through the member-function getMnemonic().
 */
template <typename base_type>
class GPostProcessorBaseT
	: public Gem::Common::GSerializableFunctionObjectT<base_type>
{
	 ///////////////////////////////////////////////////////////////////////
	 friend class boost::serialization::access;

	 template<typename Archive>
	 void serialize(Archive &ar, const unsigned int) {
		 using boost::serialization::make_nvp;

		 ar
		 & make_nvp("GSerializableFunctionObjectT_base_type"
						, boost::serialization::base_object<Gem::Common::GSerializableFunctionObjectT<base_type>>(*this))
		 & BOOST_SERIALIZATION_NVP(m_allowed_mnemonics);
	 }

	 ///////////////////////////////////////////////////////////////////////

public:
	 /**************************************************************************/
	 /**
	  * The default constructor
	  */
	 GPostProcessorBaseT() = default;

	 /**************************************************************************/
	 /**
	  * The copy constructor
	  */
	 GPostProcessorBaseT(const GPostProcessorBaseT<base_type>& cp) = default;

	 /**************************************************************************/
	 /**
	  * The destructor
	  */
	 ~GPostProcessorBaseT() override = default;

	 /**************************************************************************/
	 /**
	  * Permits postprocessing for a specific type
	  */
	 void allowPostProcessingFor(const std::string& oa_mnemonic) {
		 m_allowed_mnemonics.insert(oa_mnemonic);
	 }

	 /**************************************************************************/
	 /**
	  * Allows to check whether post-processing is allowed for a given base_type
	  */
	 bool postProcessingAllowedFor(const base_type& ind) const {
		 if(m_allowed_mnemonics.count("all") != 0) {
			 return true;
		 }

		 // Check whether the given mnemonic was registered with this class
		 return m_allowed_mnemonics.count(ind.getMnemonic()) != 0;
	 }

protected:
	 /**************************************************************************/
	 /** @brief Raw post-processing (no checks for eligibility); purely virtual */
	 virtual G_API_GENEVA bool raw_processing_(base_type& p_raw) BASE = 0;

	 /**************************************************************************/
	 /**
	  * Post-processing is triggered here
	  */
	 bool process_(base_type& p) override {
		 if(not this->postProcessingAllowedFor(p)) {
			 return true;
		 }

		 return raw_processing_(p);
	 }

	 /**************************************************************************/
	 /**
	  * Loads the data of another GEvolutionaryAlgorithmPostOptimizer object
	  */
	 void load_(const Gem::Common::GSerializableFunctionObjectT<base_type> *cp) override {
		 using namespace Gem::Common;

		 // Check that we are dealing with a Gem::Common::GSerializableFunctionObjectT<processable_type> reference independent of this object and convert the pointer
		 const GPostProcessorBaseT<base_type> *p_load
			 = g_convert_and_compare<GSerializableFunctionObjectT<base_type>, GPostProcessorBaseT<base_type>>(cp, this);

		 // Load our parent class'es data ...
		 Gem::Common::GSerializableFunctionObjectT<base_type>::load_(cp);

		 // ... and then our local data
		 m_allowed_mnemonics = p_load->m_allowed_mnemonics;
	 }

	/**************************************************************************/
	/** @brief Allow access to this classes compare_ function */
	friend void Gem::Common::compare_base_t<GPostProcessorBaseT<base_type>>(
		GPostProcessorBaseT<base_type> const &
		, GPostProcessorBaseT<base_type> const &
		, Gem::Common::GToken &
	);

	/**************************************************************************/
	/**
     * Checks for compliance with expectations with respect to another object
     * of the same type
     *
     * @param cp A constant reference to another GEvolutionaryAlgorithmPostOptimizer object
     * @param e The expected outcome of the comparison
     * @param limit The maximum deviation for floating point values (important for similarity checks)
     */
	void compare_(
		const Gem::Common::GSerializableFunctionObjectT<base_type> &cp
		, const Gem::Common::expectation &e
		, const double &limit
	) const override {
		using namespace Gem::Common;

		// Check that we are dealing with a Gem::Common::GSerializableFunctionObjectT<processable_type> reference independent of this object and convert the pointer
		const GPostProcessorBaseT<base_type> *p_load
			= Gem::Common::g_convert_and_compare<GSerializableFunctionObjectT<base_type>, GPostProcessorBaseT<base_type>>(cp, this);

		GToken token("GPostProcessorBaseT", e);

		// Compare our parent data ...
		Gem::Common::compare_base_t<GSerializableFunctionObjectT<base_type>>(*this, *p_load, token);

		// ... and then our local data
		compare_t<std::set<std::string>>(IDENTITY(m_allowed_mnemonics, p_load->m_allowed_mnemonics), token);

		// React on deviations from the expectation
		token.evaluate();
	}

private:
	 /**************************************************************************/
	 /**
	  * Returns the name of this class
	  */
	 std::string name_() const override {
		 return std::string("GPostProcessorBaseT");
	 }

	 /**************************************************************************/
	 /** @brief Creates a deep clone of this object; purely virtual */
	 G_API_GENEVA Gem::Common::GSerializableFunctionObjectT<base_type> * clone_() const override = 0;

	 /**************************************************************************/
	 // Data

	 std::set<std::string> m_allowed_mnemonics; ///< A list of mnemonics for which optimization is allowed
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This post-processor runs an evolutionary algorithm, trying to improve the
 * quality of a given individual.
 */
class GEvolutionaryAlgorithmPostOptimizer
	: public GPostProcessorBaseT<GParameterSet>
{
	 ///////////////////////////////////////////////////////////////////////
	 friend class boost::serialization::access;

	 template<typename Archive>
	 void serialize(Archive &ar, const unsigned int) {
		 using boost::serialization::make_nvp;

		 ar
		 & make_nvp("GPostProcessorBaseT_GParameterSet"
						, boost::serialization::base_object<GPostProcessorBaseT<GParameterSet>>(*this))
		 & BOOST_SERIALIZATION_NVP(m_oa_configFile)
		 & BOOST_SERIALIZATION_NVP(m_executor_configFile)
		 & BOOST_SERIALIZATION_NVP(m_executionMode);

		 // TODO: How to initialize the ea factory
	 }

	 ///////////////////////////////////////////////////////////////////////

public:
	 /**************************************************************************/
	 /** @brief Initialization with the execution mode and configuration file */
	 G_API_GENEVA GEvolutionaryAlgorithmPostOptimizer(
		 execMode executionMode
		 , const std::string& oa_configFile
		 , const std::string& executor_configFile
	 );
	 /** @brief The copy constructor */
	 G_API_GENEVA GEvolutionaryAlgorithmPostOptimizer(const GEvolutionaryAlgorithmPostOptimizer& cp) = default;
	 /** @brief The destructor */
	 G_API_GENEVA ~GEvolutionaryAlgorithmPostOptimizer() override = default;

	 /** @brief Allows to set the execution mode for this post-processor (serial vs. multi-threaded) */
	 G_API_GENEVA void setExecMode(execMode executionMode);
	 /** @brief Allows to retrieve the current execution mode */
	 G_API_GENEVA execMode getExecMode() const;

	 /** @brief Allows to specify the name of a configuration file for the optimization algorithm */
	 G_API_GENEVA void setOAConfigFile(const std::string& oaConfigFile);
	 /** @brief Allows to retrieve the configuration file for the optimization algorithm */
	 G_API_GENEVA std::string getOAConfigFile() const;

	 /** @brief Allows to specify the name of a configuration file for the executor */
	 G_API_GENEVA void setExecutorConfigFile(const std::string& executorConfigFile);
	 /** @brief Allows to retrieve the configuration file for the executor */
	 G_API_GENEVA std::string getExecutorConfigFile() const;

protected:
	 /**************************************************************************/
	 /** @brief Loads the data of another GEvolutionaryAlgorithmPostOptimizer object */
	 G_API_GENEVA void load_(const Gem::Common::GSerializableFunctionObjectT<GParameterSet> *cp) override;

	/** @brief Allow access to this classes compare_ function */
	friend void Gem::Common::compare_base_t<GEvolutionaryAlgorithmPostOptimizer>(
		GEvolutionaryAlgorithmPostOptimizer const &
		, GEvolutionaryAlgorithmPostOptimizer const &
		, Gem::Common::GToken &
	);

	/** @brief Checks for compliance with expectations with respect to another object of the same type */
	G_API_GENEVA void compare_(
		const Gem::Common::GSerializableFunctionObjectT<GParameterSet> &cp
		, const Gem::Common::expectation &e
		, const double &limit
	) const override;

	/** @brief The actual post-processing takes place here (no further checks) */
	 G_API_GENEVA bool raw_processing_(GParameterSet& p) override;

private:
	 /** @brief Returns the name of this class */
	 G_API_GENEVA std::string name_() const override;
	 /** @brief Creates a deep clone of this object */
	 G_API_GENEVA Gem::Common::GSerializableFunctionObjectT<GParameterSet> * clone_() const override;

	 /** @brief The standard constructor */
	 G_API_GENEVA GEvolutionaryAlgorithmPostOptimizer();

	 /**************************************************************************/
	 // Data
	 std::string m_oa_configFile; ///< The name of the configuration file for this evolutionary algorithm
	 std::string m_executor_configFile; ///< The name of the configuration file for the executor
	 execMode m_executionMode = execMode::SERIAL; ///< Whether to run the post-optimizer in serial or multi-threaded mode
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
// Mark this class as abstract.
BOOST_SERIALIZATION_ASSUME_ABSTRACT(Gem::Geneva::GPostProcessorBaseT<GParameterSet>)

// Export of GEvolutionaryAlgorithmPostOptimizer
BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GEvolutionaryAlgorithmPostOptimizer)

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

