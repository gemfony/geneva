/**
 * @file G_OA_AlgorithmTemplateT.hpp
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
#include <memory>

// Boost headers go here

#ifndef G_OA_ALGORITHTEMPLATET_HPP_
#define G_OA_ALGORITHTEMPLATET_HPP_

// Geneva headers go here
#include "common/GExceptions.hpp"
#include "common/GPlotDesigner.hpp"
#include "geneva/GOptimizableEntity.hpp"
#include "geneva/GParameterSet.hpp"
#include "geneva/GOptimizationAlgorithmT.hpp"
#include "geneva/GOptimizationEnums.hpp"
#include "geneva/GGDPersonalityTraits.hpp"
#include "geneva/GOptimizationAlgorithmT.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * The GAlgorithmTemplateT class template demomstrates which functions need
 * to be implemented for a new optimization algorithm. Please note that you
 * do not need to implement this class as a template. For most users deriving
 * from GOptimizationAlgorithmT<Gem::Courtier::GBrokerExecutor<GParameterSet>>
 * will suffice. This will simplify some of the code, as you do not need to
 * care for some of the C++ template oddities, such as having to preface iterators
 * over individuals with the "template" keyword. When compiling on Microsoft
 * Windows, preface each function with the G_API_GENEVA macro.
 */
template <typename executor_type>
class GAlgorithmTemplateT
	: public GOptimizationAlgorithmT<executor_type>
{
	 ///////////////////////////////////////////////////////////////////////
	 friend class boost::serialization::access;

	 template<typename Archive>
	 void serialize(Archive & ar, const unsigned int) {
		 using boost::serialization::make_nvp;

		 ar
		 & make_nvp("GOptimizationAlgorithmT_GBrokerExecutorT",
			 boost::serialization::base_object<GOptimizationAlgorithmT<executor_type>>(*this));
	 }

	 ///////////////////////////////////////////////////////////////////////

public:
	 /***************************************************************************/
	 /**
	  * The default constructor
	  */
	 GAlgorithmTemplateT() {
		 // nothing
	 }

	 /***************************************************************************/
	 /**
	  * A standard copy constructor
	  *
	  * @param cp A copy of another GAlgorithmTemplateT<executor_type> object
	  */
	 GAlgorithmTemplateT(const GAlgorithmTemplateT<executor_type>& cp) {
		 // nothing
	 }

	 /***************************************************************************/
	 /**
	  * The destructor
	  */
	 virtual ~GAlgorithmTemplateT() {
		 // nothing
	 }

	 /***************************************************************************/
	 /**
	  * The standard assignment operator
	  *
	  * @param cp A copy of another GAlgorithmTemplateT<executor_type> object
	  */
	 const GAlgorithmTemplateT& operator=(const GAlgorithmTemplateT<executor_type>& cp) {
		 //  nothing
	 }

	 /***************************************************************************/
	 /**
	  * Checks for equality with another GAlgorithmTemplateT object
	  *
	  * @param cp A copy of another GAlgorithmTemplateT<executor_type> object
	  */
	 virtual bool operator==(const GAlgorithmTemplateT<executor_type>& cp) const {
		 // nothing
	 }

	 /***************************************************************************/
	 /**
	  * Checks for inequality with another GAlgorithmTemplateT object
	  *
	  * @param cp A copy of another GAlgorithmTemplateT<executor_type> object
	  */
	 virtual bool operator!=(const GAlgorithmTemplateT<executor_type>& cp) const {
		 // nothing
	 }

	 /***************************************************************************/
	 /**
	  * Searches for compliance with expectations with respect to another
	  * object of the same type.
	  *
	  * @param cp A copy of another GAlgorithmTemplateT<executor_type> object, camouflaged as a GObject
	  * @param e The expectation for the comparison (see e.g. operator ==)
	  * @param limit Determines, below which difference the comparison of two doubles is considered as equality
	  */
	 virtual void compare(
		 const GObject& cp // the other object
		 , const Gem::Common::expectation& e // the expectation for this object, e.g. equality
		 , const double& limit // the limit for allowed deviations of floating point types
	 ) const override {
		 // nothing
	 }

	 /***************************************************************************/
	 /**
	  * Returns information about the type of optimization algorithm
	  */
	 virtual std::string getOptimizationAlgorithm() const override {

	 }

	 /***************************************************************************/
	 /**
	  * Retrieves the number of processable items for the current iteration
	  */
	 virtual std::size_t getNProcessableItems() const override {

	 }

	 /***************************************************************************/
	 /**
	  * Returns the name of this optimization algorithm
	  */
	 virtual std::string getAlgorithmName() const override {

	 }

	 /***************************************************************************/
	 /**
	  * Adds local configuration options to a GParserBuilder object
	  */
	 virtual void addConfigurationOptions (
		 Gem::Common::GParserBuilder& gpb
	 ) override {

	 }

	 /***************************************************************************/
	 /**
	  * Emits a name for this class / object
	  */
	 virtual std::string name() const override {

	 }

protected:
	 /***************************************************************************/
	 /**
	  * Loads the data of another population
	  */
	 virtual void load_(const GObject *) override {

	 }

	 /***************************************************************************/
	 /**
	  * Creates a deep clone of this object
	  */
	 virtual GObject *clone_() const override {

	 }

	 /***************************************************************************/
	 /**
	  * The actual business logic to be performed during each iteration. Returns the best achieved fitness
	  */
	 virtual std::tuple<double, double> cycleLogic() override {

	 };

	 /***************************************************************************/
	 /**
	  * Does some preparatory work before the optimization starts
	  */
	 virtual void init() override {

	 }

	 /***************************************************************************/
	 /**
	  * Does any necessary finalization work
	  */
	 virtual void finalize() override {

	 }

	 /***************************************************************************/
	 /**
	  * Retrieve a GPersonalityTraits object belonging to this algorithm
	  */
	 virtual std::shared_ptr<GPersonalityTraits> getPersonalityTraits() const override {

	 }

	 /***************************************************************************/
	 /**
	  * Resizes the population to the desired level and does some error checks
	  */
	 virtual void adjustPopulation() override {

	 }

	 /***************************************************************************/
	 /**
	  * Triggers fitness calculation of a number of individuals
	  */
	 virtual void runFitnessCalculation() override {

	 }

private:
	 /***************************************************************************/
	 // ...

	 /***************************************************************************/

public
	 /***************************************************************************/
	 /**
	  * Applies modifications to this object. This is needed for testing purposes
	  */
	 virtual bool modify_GUnitTests() override {

	 }

	 /***************************************************************************/
	 /**
	  * Performs self tests that are expected to succeed. This is needed for testing purposes
	  */
	 virtual void specificTestsNoFailureExpected_GUnitTests() override {

	 }

	 /***************************************************************************/
	 /**
	  * Performs self tests that are expected to fail. This is needed for testing purposes
	  */
	 virtual void specificTestsFailuresExpected_GUnitTests() override {

	 }

	 /***************************************************************************/
};

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */


// TODO: Add specializations for different executors
// BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GAlgorithmTemplateT)

#endif /* G_OA_ALGORITHTEMPLATET_HPP_ */
