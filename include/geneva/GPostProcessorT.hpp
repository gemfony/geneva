/**
 * @file GPostProcessorT.hpp
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

// Standard headers go here
#include <set>
#include <string>

// Boost headers go here
#include <boost/serialization/set.hpp>

#ifndef GENEVA_LIBRARY_COLLECTION_GPOSTOPTIMIZERS_HPP
#define GENEVA_LIBRARY_COLLECTION_GPOSTOPTIMIZERS_HPP

// Geneva headers go here
#include "common/GSerializableFunctionObjectT.hpp"
#include "geneva/GParameterSet.hpp"
#include "geneva/GEvolutionaryAlgorithmFactory.hpp"

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
template <typename ind_type, typename base_type>
class GPostProcessorBaseT
	: public Gem::Common::GSerializableFunctionObjectT<base_type>
{
	/**************************************************************************/
	// Make sure this class can only be instantiated if ind_type is a derivative of base_type
	static_assert(
		std::is_base_of<base_type, ind_type>::value
		, "GPostProcessorBaseT<>: base_type is no base class of ind_type"
	);

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
	GPostProcessorBaseT()
	{ /* nothing */ }

	/**************************************************************************/
	/**
	 * The copy constructor
	 */
	GPostProcessorBaseT(const GPostProcessorBaseT<ind_type, base_type>& cp)
		: m_allowed_mnemonics(cp.m_allowed_mnemonics)
	{ /* nothing */ }

	/**************************************************************************/
	/**
	 * The destructor
	 */
	virtual ~GPostProcessorBaseT()
	{
		m_allowed_mnemonics.clear();
	}

	/**************************************************************************/
	/**
	 * Checks for equality with another GPostProcessorBaseT<ind_type, base_type> object
	 *
	 * @param  cp A constant reference to another GPostProcessorBaseT<ind_type, base_type> object
	 * @return A boolean indicating whether both objects are equal
	 */
	bool operator==(const GPostProcessorBaseT<ind_type, base_type>& cp) const {
		using namespace Gem::Common;
		try {
			this->compare(cp, Gem::Common::expectation::CE_EQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
			return true;
		} catch(g_expectation_violation&) {
			return false;
		}
	}

	/**************************************************************************/
	/**
	 * Checks for inequality with another GPostProcessorBaseT<ind_type, base_type> object
	 *
	 * @param  cp A constant reference to another GPostProcessorBaseT<ind_type, base_type> object
	 * @return A boolean indicating whether both objects are inequal
	 */
	bool operator!=(const GPostProcessorBaseT<ind_type, base_type>& cp) const {
		using namespace Gem::Common;
		try {
			this->compare(cp, Gem::Common::expectation::CE_INEQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
			return true;
		} catch(g_expectation_violation&) {
			return false;
		}
	}

	/**************************************************************************/
	/**
	 * Returns the name of this class
	 */
	virtual std::string name() const override {
		return std::string("GPostProcessorBaseT");
	}

	/**************************************************************************/
	/**
	 * Checks for compliance with expectations with respect to another object
	 * of the same type
	 *
	 * @param cp A constant reference to another GEvolutionaryAlgorithmPostOptimizer object
	 * @param e The expected outcome of the comparison
	 * @param limit The maximum deviation for floating point values (important for similarity checks)
	 */
	virtual void compare(
		const Gem::Common::GSerializableFunctionObjectT<base_type> &cp
		, const Gem::Common::expectation &e
		, const double &limit
	) const override {
		using namespace Gem::Common;

		// Check that we are dealing with a Gem::Common::GSerializableFunctionObjectT<processable_type> reference independent of this object and convert the pointer
		const GPostProcessorBaseT<ind_type, base_type> *p_load
			= Gem::Common::g_convert_and_compare<GSerializableFunctionObjectT<base_type>, GPostProcessorBaseT<ind_type, base_type>>(cp, this);

		GToken token("GPostProcessorBaseT", e);

		// Compare our parent data ...
		Gem::Common::compare_base<GSerializableFunctionObjectT<base_type>>(IDENTITY(*this, *p_load), token);

		// ... and then our local data
		compare_t<std::set<std::string>>(IDENTITY(m_allowed_mnemonics, p_load->m_allowed_mnemonics), token);

		// React on deviations from the expectation
		token.evaluate();
	}


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
		if(m_allowed_mnemonics.count(ind.getMnemonic()) != 0) {
			return true;
		}

		// Element was not found -- the algorithm is not eligible for post-processing
		return false;
	}

protected:
	/**************************************************************************/
	/**
	 * Loads the data of another GEvolutionaryAlgorithmPostOptimizer object
	 */
	virtual void load_(const Gem::Common::GSerializableFunctionObjectT<base_type> *cp) override {
		using namespace Gem::Common;

		// Check that we are dealing with a Gem::Common::GSerializableFunctionObjectT<processable_type> reference independent of this object and convert the pointer
		const GPostProcessorBaseT<ind_type, base_type> *p_load
			= g_convert_and_compare<GSerializableFunctionObjectT<base_type>, GPostProcessorBaseT<ind_type, base_type>>(cp, this);

		// Load our parent class'es data ...
		Gem::Common::GSerializableFunctionObjectT<base_type>::load_(cp);

		// ... and then our local data
		m_allowed_mnemonics = p_load->m_allowed_mnemonics;
	}

	/**************************************************************************/
	/** @brief Creates a deep clone of this object; purely virtual */
	virtual Gem::Common::GSerializableFunctionObjectT<base_type> * clone_() const override = 0;

private:
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
template <typename ind_type, typename base_type=GParameterSet>
class GEvolutionaryAlgorithmPostOptimizerT
	: public GPostProcessorBaseT<ind_type,base_type>
{
	 /**************************************************************************/
	 // Make sure this class can only be instantiated if ind_type is a derivative of base_type
	 static_assert(
		 std::is_base_of<base_type, ind_type>::value
		 , "base_type is no base class of ind_type"
	 );

	 ///////////////////////////////////////////////////////////////////////
	 friend class boost::serialization::access;

	 template<typename Archive>
	 void serialize(Archive &ar, const unsigned int) {
		 using boost::serialization::make_nvp;

		 ar
		 & make_nvp("GPostProcessorBaseT_ind_type_base_type"
						, boost::serialization::base_object<GPostProcessorBaseT<ind_type,base_type>>(*this))
		 & BOOST_SERIALIZATION_NVP(m_configFile)
		 & BOOST_SERIALIZATION_NVP(m_executionMode);

		 // TODO: How to initialize the ea factory
	 }

	 ///////////////////////////////////////////////////////////////////////

public:
	 /**************************************************************************/
	 /**
	  * Initialization with the execution mode and configuration file
	  */
	 GEvolutionaryAlgorithmPostOptimizerT(
		 execMode executionMode
	 	 , std::string configFile
	 )
	 	: GPostProcessorBaseT<ind_type,base_type>()
	   , m_configFile(configFile)
	 	, m_executionMode((executionMode==execMode::EXECMODE_SERIAL || executionMode==execMode::EXECMODE_MULTITHREADED)?executionMode:execMode::EXECMODE_SERIAL)
	 {
		 switch(executionMode) {
			 case execMode::EXECMODE_SERIAL:
			 case execMode::EXECMODE_MULTITHREADED:
				 /* nothing */
				 break;

			 case execMode::EXECMODE_BROKERAGE:
			 default: {
				 glogger
					 << "In GEvolutionaryAlgorithmPostOptimizerT::GEvolutionaryAlgorithmPostOptimizerT(execMode): Error!" << std::endl
					 << "Got invalid execution mode " << executionMode << std::endl
					 << "The mode was reset to execMode::EXECMODE_SERIAL" << std::endl
					 << GWARNING;
			 }
		 }
	 }

	 /**************************************************************************/
	 /**
	  * The copy constructor
	  */
	 GEvolutionaryAlgorithmPostOptimizerT(const GEvolutionaryAlgorithmPostOptimizerT<ind_type>& cp)
	 	: GPostProcessorBaseT<ind_type,base_type>(cp)
	   , m_configFile(cp.m_configFile)
	 	, m_executionMode(cp.m_executionMode) // We assume that a valid execution mode is stored here
	 { /* nothing */ }

	 /**************************************************************************/
	 /**
	  * The destructor
     */
	 virtual ~GEvolutionaryAlgorithmPostOptimizerT()
	 { /* nothing */ }

	 /**************************************************************************/
	 /**
	  * Checks for equality with another GEvolutionaryAlgorithmPostOptimizer object
	  *
	  * @param  cp A constant reference to another GEvolutionaryAlgorithmPostOptimizer object
	  * @return A boolean indicating whether both objects are equal
	  */
	 bool operator==(const GEvolutionaryAlgorithmPostOptimizerT<ind_type>& cp) const {
		 using namespace Gem::Common;
		 try {
			 this->compare(cp, Gem::Common::expectation::CE_EQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
			 return true;
		 } catch(g_expectation_violation&) {
			 return false;
		 }
	 }

	 /**************************************************************************/
	 /**
	  * Checks for inequality with another GEvolutionaryAlgorithmPostOptimizer object
	  *
	  * @param  cp A constant reference to another GEvolutionaryAlgorithmPostOptimizer object
	  * @return A boolean indicating whether both objects are inequal
	  */
	 bool operator!=(const GEvolutionaryAlgorithmPostOptimizerT<ind_type>& cp) const {
		 using namespace Gem::Common;
		 try {
			 this->compare(cp, Gem::Common::expectation::CE_INEQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
			 return true;
		 } catch(g_expectation_violation&) {
			 return false;
		 }
	 }

	 /**************************************************************************/
	 /**
	  * Returns the name of this class
	  */
	 virtual std::string name() const override {
		 return std::string("GEvolutionaryAlgorithmPostOptimizerT");
	 }

	 /**************************************************************************/
	 /**
	  * Checks for compliance with expectations with respect to another object
	  * of the same type
	  *
	  * @param cp A constant reference to another GEvolutionaryAlgorithmPostOptimizer object
	  * @param e The expected outcome of the comparison
	  * @param limit The maximum deviation for floating point values (important for similarity checks)
	  */
	 virtual void compare(
		 const Gem::Common::GSerializableFunctionObjectT<base_type> &cp
		 , const Gem::Common::expectation &e
		 , const double &limit
	 ) const override {
		 using namespace Gem::Common;

		 // Check that we are dealing with a Gem::Common::GSerializableFunctionObjectT<processable_type> reference independent of this object and convert the pointer
		 const GEvolutionaryAlgorithmPostOptimizerT<ind_type> *p_load
			 = Gem::Common::g_convert_and_compare<Gem::Common::GSerializableFunctionObjectT<base_type>, GEvolutionaryAlgorithmPostOptimizerT<ind_type>>(cp, this);

		 GToken token("GEvolutionaryAlgorithmPostOptimizerT", e);

		 // Compare our parent data ...
		 Gem::Common::compare_base<GPostProcessorBaseT<ind_type,base_type>>(IDENTITY(*this, *p_load), token);

		 // ... and then our local data
		 compare_t(IDENTITY(m_configFile, p_load->m_configFile), token);
		 compare_t(IDENTITY(m_executionMode, p_load->m_executionMode), token);

		 // React on deviations from the expectation
		 token.evaluate();
	 }

	 /**************************************************************************/
	 /**
	  * Allows to set the execution mode for this post-processor (serial vs. multi-threaded)
	  */
	 void setExecMode(execMode executionMode) {
		 switch(executionMode) {
			 case execMode::EXECMODE_SERIAL:
			 case execMode::EXECMODE_MULTITHREADED: {
				 m_executionMode = executionMode;
			 } break;

			 case execMode::EXECMODE_BROKERAGE:
			 default: {
				 glogger
				 << "In GEvolutionaryAlgorithmPostOptimizerT::setExecMode(): Error!" << std::endl
			    << "Got invalid execution mode " << executionMode << std::endl
				 << GEXCEPTION;
			 }
		 }
	 }

	 /**************************************************************************/
	 /**
	  * Allows to retrieve the current execution mode
	  */
	 execMode getExecMode() const {
		 return m_executionMode;
	 }

	 /**************************************************************************/
	 /**
	  * Allows to specify the name of a configuration file
	  */
	 void setConfigFile(std::string configFile) {
		 m_configFile = configFile;
	 }

	 /**************************************************************************/
	 /**
	  * Allows to retrieve the configuration file
	  */
	 std::string getConfigFile() const {
		 return m_configFile;
	 }

protected:
	 /**************************************************************************/
	 /**
	  * Loads the data of another GEvolutionaryAlgorithmPostOptimizer object
	  */
	 virtual void load_(const Gem::Common::GSerializableFunctionObjectT<base_type> *cp) override {
		 // Check that we are dealing with a GEvolutionaryAlgorithmPostOptimizerT reference independent of this object and convert the pointer
		 const GEvolutionaryAlgorithmPostOptimizerT<ind_type> *p_load
			 = Gem::Common::g_convert_and_compare<Gem::Common::GSerializableFunctionObjectT<base_type>, GEvolutionaryAlgorithmPostOptimizerT<ind_type>>(cp, this);

		 // Load our parent class'es data ...
		 GPostProcessorBaseT<ind_type,base_type>::load_(cp);

		 // ... and then our local data
		 m_configFile = p_load->m_configFile;
		 m_executionMode = p_load->m_executionMode;
	 }

	 /**************************************************************************/
	 /**
	  * Creates a deep clone of this object
     */
	 virtual Gem::Common::GSerializableFunctionObjectT<base_type> * clone_() const override {
		 return new GEvolutionaryAlgorithmPostOptimizerT<ind_type>(*this);
	 }

	 /**************************************************************************/
	 /**
	  * The actual post-processing takes place here
	  */
	 virtual bool process_(base_type& p_raw) override {
		 // Convert the base_type to the derived type
	    ind_type& p = dynamic_cast<ind_type&>(p_raw);

	 	 // Create a factory for evolutionary algorithm objects
		 // TODO: Move this to class-scope
	 	 GEvolutionaryAlgorithmFactory eaFactory(m_configFile, m_executionMode);

		 // Obtain a new evolutionary algorithm from the factory. It will be
		 // equipped with all settings from the config file
		 std::shared_ptr<GBaseEA> ea_ptr = eaFactory.get<GBaseEA>();

		 // Make sure p is clean
		 if(p.isDirty()) {
			 glogger
			 << "In GEvolutionaryAlgorithmPostOptimizerT: Error!" << std::endl
		    << "Provided base_type has dirty flag set." << std::endl
			 << GEXCEPTION;
		 }

		 // Clone the individual for post-processing
		 std::shared_ptr<ind_type> p_unopt_ptr = std::dynamic_pointer_cast<ind_type>(p.clone());

		 // Make sure the post-optimization does not trigger post-optimization ...
		 p_unopt_ptr->vetoPostProcessing(true);

		 // Add our individual to the algorithm
		 ea_ptr->push_back(p_unopt_ptr);

		 // Perform the actual (sub-)optimization
		 ea_ptr->optimize();

		 // Retrieve the best individual
		 std::shared_ptr<ind_type> p_opt_ptr = ea_ptr->getBestGlobalIndividual<ind_type>();

		 // Make sure subsequent optimization cycles may generally perform post-optimization again
		 p_unopt_ptr->vetoPostProcessing(false);

	    // Load the individual into the argument base_type
		 p.load(p_opt_ptr);

		 return true;
	 }

private:
	 /**************************************************************************/
	 /**
	  * The standard constructor. Intentionally private, as it is only needed
	  * for de-serialization purposes.
	  */
	 GEvolutionaryAlgorithmPostOptimizerT()
	 { /* nothing */ }

	 /**************************************************************************/
	 // Data
	 std::string m_configFile; ///< The name of the configuration file for this evolutionary algorithm
	 execMode m_executionMode = execMode::EXECMODE_SERIAL; ///< Whether to run the post-optimizer in serial or multi-threaded mode
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

#endif /* GENEVA_LIBRARY_COLLECTION_GPOSTOPTIMIZERS_HPP */
