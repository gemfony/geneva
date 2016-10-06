/**
 * @file GPostOptimizers.hpp
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

// Boost headers go here

#ifndef GENEVA_LIBRARY_COLLECTION_GPOSTOPTIMIZERS_HPP
#define GENEVA_LIBRARY_COLLECTION_GPOSTOPTIMIZERS_HPP

// Geneva headers go here
#include "common/GSerializableFunctionObjectT.hpp"
#include "geneva/GParameterSet.hpp"
#include "geneva/GEvolutionaryAlgorithmFactory.hpp"

namespace Gem {
namespace Common {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

class GEvolutionaryAlgorithmPostOptimizer
	: public GSerializableFunctionObjectT<GParameterSet>
{
	 ///////////////////////////////////////////////////////////////////////
	 friend class boost::serialization::access;

	 template<typename Archive>
	 void serialize(Archive &ar, const unsigned int) {
		 using boost::serialization::make_nvp;

		 ar
		 & make_nvp("GSerializableFunctionObjectT_GParameterSet"
						, boost::serialization::base_object<GSerializableFunctionObjectT<GParameterSet>>(*this))
		 & BOOST_SERIALIZATION_NVP(m_configFile)
		 & BOOST_SERIALIZATION_NVP(executionMode);

		 // TODO: How to initialize the ea factory
	 }

	 ///////////////////////////////////////////////////////////////////////

public:
	 /**
	  * Initialization with the execution mode and configuration file
	  */
	 GEvolutionaryAlgorithmPostOptimizer(
		 execMode executionMode
	 	 , std::string configFile
	 )
	 	: GSerializableFunctionObjectT()
	   , m_configFile(configFile)
	 	, m_executionMode((executionMode==execMode::EXECMODE_SERIAL || executionMode==execMode::EXECMODE_MULTITHREADED)?executionMode:execMode::EXECMODE_SERIAL)
	 {
		 switch(executionMode) {
			 case execMode::EXECMODE_SERIAL:
			 case execMode::EXECMODE_MULTITHREADED:
				 /* nothing */
				 break;

			 case EXECMODE_BROKERAGE:
			 default: {
				 glogger
					 << "In GEvolutionaryAlgorithmPostOptimizer::GEvolutionaryAlgorithmPostOptimizer(execMode): Error!" << std::endl
					 << "Got invalid execution mode " << executionMode << std::endl
					 << "The mode was reset to execMode::EXECMODE_SERIAL" << std::endl
					 << GWARNING;
			 }
		 }
	 }

	 /**
	  * The copy constructor
	  */
	 GEvolutionaryAlgorithmPostOptimizer(const GEvolutionaryAlgorithmPostOptimizer& cp)
	 	: GSerializableFunctionObjectT(cp)
	   , m_configFile(cp.m_configFile)
	 	, m_executionMode(cp.m_executionMode) // We assume that a valid execution mode is stored here
	 { /* nothing */ }

	 /**
	  * The destructor
     */
	 virtual ~GEvolutionaryAlgorithmPostOptimizer()
	 { /* nothing */ }

	 /**
	  * Checks for equality with another GEvolutionaryAlgorithmPostOptimizer object
	  *
	  * @param  cp A constant reference to another GEvolutionaryAlgorithmPostOptimizer object
	  * @return A boolean indicating whether both objects are equal
	  */
	 bool operator==(const GEvolutionaryAlgorithmPostOptimizer& cp) const {
		 using namespace Gem::Common;
		 try {
			 this->compare(cp, Gem::Common::expectation::CE_EQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
			 return true;
		 } catch(g_expectation_violation&) {
			 return false;
		 }
	 }

	 /**
	  * Checks for inequality with another GEvolutionaryAlgorithmPostOptimizer object
	  *
	  * @param  cp A constant reference to another GEvolutionaryAlgorithmPostOptimizer object
	  * @return A boolean indicating whether both objects are inequal
	  */
	 bool operator!=(const GEvolutionaryAlgorithmPostOptimizer& cp) const {
		 using namespace Gem::Common;
		 try {
			 this->compare(cp, Gem::Common::expectation::CE_INEQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
			 return true;
		 } catch(g_expectation_violation&) {
			 return false;
		 }
	 }

	 /**
	  * Returns the name of this class
	  */
	 virtual std::string name() const override {
		 return std::string("GEvolutionaryAlgorithmPostOptimizer");
	 }

	 /**
	  * Checks for compliance with expectations with respect to another object
	  * of the same type
	  *
	  * @param cp A constant reference to another GEvolutionaryAlgorithmPostOptimizer object
	  * @param e The expected outcome of the comparison
	  * @param limit The maximum deviation for floating point values (important for similarity checks)
	  */
	 virtual void compare(
		 const GEvolutionaryAlgorithmPostOptimizer &cp
		 , const Gem::Common::expectation &e
		 , const double &limit
	 ) const override {
		 using namespace Gem::Common;

		 // Check that we are dealing with a GSerializableFunctionObjectT<processable_type> reference independent of this object and convert the pointer
		 const GEvolutionaryAlgorithmPostOptimizer *p_load
			 = Gem::Common::g_convert_and_compare<GEvolutionaryAlgorithmPostOptimizer, GEvolutionaryAlgorithmPostOptimizer>(cp, this);

		 GToken token("GEvolutionaryAlgorithmPostOptimizer", e);

		 // Compare our parent data ...
		 Gem::Common::compare_base<GSerializableFunctionObjectT<GParameterSet>>(IDENTITY(*this, *p_load), token);

		 // ... and then our local data
		 compare_t(IDENTITY(m_configFile, p_load->m_configFile), token);
		 compare_t(IDENTITY(m_executionMode, p_load->m_executionMode), token);

		 // React on deviations from the expectation
		 token.evaluate();
	 }

	 /**
	  * Allows to set the execution mode for this post-processor (serial vs. multi-threaded)
	  */
	 void setExecMode(execMode executionMode) {
		 switch(executionMode) {
			 case execMode::EXECMODE_SERIAL:
			 case execMode::EXECMODE_MULTITHREADED: {
				 m_executionMode = executionMode;
			 } break;

			 case EXECMODE_BROKERAGE:
			 default: {
				 glogger
				 << "In GEvolutionaryAlgorithmPostOptimizer::setExecMode(): Error!" << std::endl
			    << "Got invalid execution mode " << executionMode << std::endl
				 << GEXCEPTION;
			 }
		 }
	 }

	 /**
	  * Allows to retrieve the current execution mode
	  */
	 execMode getExecMode() const {
		 return m_executionMode;
	 }

	 /**
	  * Allows to specify the name of a configuration file
	  */
	 void setConfigFile(std::string configFile) {
		 m_configFile = configFile;
	 }

	 /**
	  * Allows to retrieve the configuration file
	  */
	 std::string getConfigFile() const {
		 return m_configFile;
	 }

protected:
	 /**
	  * Loads the data of another GEvolutionaryAlgorithmPostOptimizer object
	  */
	 virtual void load_(const GEvolutionaryAlgorithmPostOptimizer *cp) override {
		 // Check that we are dealing with a GSerializableFunctionObjectT<processable_type> reference independent of this object and convert the pointer
		 const GEvolutionaryAlgorithmPostOptimizer *p_load = Gem::Common::g_convert_and_compare<GEvolutionaryAlgorithmPostOptimizer, GEvolutionaryAlgorithmPostOptimizer>(cp, this);

		 // Load our parent class'es data ...
		 GSerializableFunctionObjectT<GParameterSet>::load_(cp);

		 // ... and then our local data
		 m_configFile = p_load->m_configFile;
		 m_executionMode = p_load->m_executionMode;
	 }

	 /**
	  * Creates a deep clone of this object
     */
	 virtual GEvolutionaryAlgorithmPostOptimizer * clone_() const override {
		 return new GEvolutionaryAlgorithmPostOptimizer(*this);
	 }

	 /**
	  * The actual post-processing takes place here
	  */
	 virtual bool process_(GParameterSet& p) override {
	 	 // Create a factory for evolutionary algorithm objects
	 	 GEvolutionaryAlgorithmFactory eaFactory(m_configFile, m_executionMode);

	    // TODO: Move this to class-scope

		 // Obtain a new evolutionary algorithm from the factory. It will be
		 // equipped with all settings from the config file
		 std::shared_ptr<GBaseEA> ea_ptr = eaFactory();

		 // Make sure p is clean
		 if(p.isDirty()) {
			 glogger
			 << "In GEvolutionaryAlgorithmPostOptimizer: Error!" << std::endl
		    << "Provided GParameterSet has dirty flag set." << std::endl
			 << GEXCEPTION;
		 }

		 // Add our individual to the algorithm
		 ea_ptr.push_back(p.clone());

		 // Perform the actual optimization and retrieve the best individual
		 std::shared_ptr<GParameterSet> p_opt_ptr = ea_ptr->optimize<GParameterSet>();

	    // Load the individual into the argument GParameterSet
		 p.load(p_opt_ptr);
	 }

private:
	 /**
	  * The standard constructor. Intentionally private, as it is only needed
	  * for de-serialization purposes.
	  */
	 GEvolutionaryAlgorithmPostOptimizer()
	 { /* nothing */ }

	 // Data
	 std::string m_configFile; ///< The name of the configuration file for this evolutionary algorithm
	 execMode m_executionMode = execMode::EXECMODE_SERIAL; ///< Whether to run the post-optimizer in serial or multi-threaded mode
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Common */
} /* namespace Gem */

#endif /* GENEVA_LIBRARY_COLLECTION_GPOSTOPTIMIZERS_HPP */
