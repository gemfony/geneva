/**
 * @file GStartIndividual.hpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt) and Karlsruhe
 * Institute of Technology (University of the State of Baden-Wuerttemberg
 * and National Laboratory of the Helmholtz Association).
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) com
 *
 * This file is part of the Geneva library collection
 *
 * Geneva is free software: you can redistribute it and/or modify
 * it under the terms of version 3 of the GNU Affero General Public License
 * as published by the Free Software Foundation.
 *
 * Geneva is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with the Geneva library.  If not, see <http://www.gnu.org/licenses/>.
 *
 * For further information on Gemfony scientific and Geneva, visit
 * http://www.gemfony.com .
 */


// Standard header files go here
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <sstream>
#include <vector>
#include <list>
#include <algorithm> // for std::sort
#include <utility> // For std::pair

// Includes check for correct Boost version(s)
#include <common/GGlobalDefines.hpp>

// Boost header files go here
#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/cast.hpp>

#ifndef GSTARTINDIVIDUAL_HPP_
#define GSTARTINDIVIDUAL_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

// Geneva header files go here
#include <hap/GRandomT.hpp>
#include <common/GCommonEnums.hpp>
#include <common/GExceptions.hpp>
#include <geneva/GConstrainedDoubleObject.hpp>
#include <geneva/GConstrainedDoubleObjectCollection.hpp>
#include <geneva/GDoubleGaussAdaptor.hpp>
#include <geneva/GObjectExpectationChecksT.hpp>
#include <geneva/GParameterObjectCollection.hpp>
#include <geneva/GParameterSet.hpp>

#ifdef GENEVATESTING
#include <common/GUnitTestFrameworkT.hpp>
#endif /* GENEVATESTING */

namespace Gem
{
namespace Geneva
{
/************************************************************************************************/
/**
 * This individual searches for the minimum of a parabola of a given dimension,
 * It is part of a complete example that lets users adapt their optimization
 * problems more easily to the Geneva conventions.
 */
class GStartIndividual :public GParameterSet
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int) {
		using boost::serialization::make_nvp;

		ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GParameterSet);

		/* Add your own class-variables here in the following way:
			ar & BOOST_SERIALIZATION_NVP(myVar);
			or
			ar & make_nvp("myVar", myVar); // The latter form can be necessary when dealing with templates
		 */
	}
	///////////////////////////////////////////////////////////////////////

public:
	/********************************************************************************************/
	/**
	 * A simple constructor that initializes this object with a collection of bounded
	 * double variables.
	 *
	 * @param dim The amount of variables
	 * @param min The lower boundary of the variables
	 * @param max The upper boundary of the variables
	 */
	GStartIndividual(const std::size_t& dim,
			const double& min,
			const double& max)
		: GParameterSet()
	  {
		// Set up a GConstrainedDoubleObjectCollection
		// boost::shared_ptr<GConstrainedDoubleObjectCollection> gbdc_ptr(new GConstrainedDoubleObjectCollection());
		// boost::shared_ptr<GParameterObjectCollection> gpoc_ptr(new GParameterObjectCollection());

		// Add bounded double objects
		for(std::size_t i=0; i<dim; i++) {
			// GConstrainedDoubleObject will start with random values in the range [min:max]
			boost::shared_ptr<GConstrainedDoubleObject> gbd_ptr(new GConstrainedDoubleObject(gr.uniform_real<double>(min, max), min, max) );

			// Create a suitable adaptor (sigma=0.1, sigma-adaption=0.5, min sigma=0, max sigma=0,5)
			boost::shared_ptr<GDoubleGaussAdaptor> gdga_ptr(new GDoubleGaussAdaptor(0.1, 0.5, 0., 0.5));
			gdga_ptr->setAdaptionThreshold(1); // Adaption parameters are modified after each adaption
			gdga_ptr->setAdaptionProbability(0.05); // The likelihood for a parameter to be adapted

			// Register the adaptor with GConstrainedDoubleObject objects
			gbd_ptr->addAdaptor(gdga_ptr);

			// Add a GConstrainedDoubleObject object to the collection
			// gbdc_ptr->push_back(gbd_ptr);
			// gpoc_ptr->push_back(gbd_ptr);
			this->push_back(gbd_ptr);
		}

		// Add the collection to this object
		// this->push_back(gbdc_ptr);
		// this->push_back(gpoc_ptr);
	  }

	/********************************************************************************************/
	/**
	 * A standard copy constructor
	 *
	 * @param cp A copy of another GStartIndividual
	 */
	GStartIndividual(const GStartIndividual& cp)
		: GParameterSet(cp)
	{ /* nothing */ }

	/********************************************************************************************/
	/**
	 * The standard destructor
	 */
	virtual ~GStartIndividual()
	{ /* nothing */	}

	/********************************************************************************************/
	/**
	 * A standard assignment operator
	 *
	 * @param cp A copy of another GStartIndividual object
	 * @return A constant reference to this object
	 */
	const GStartIndividual& operator=(const GStartIndividual& cp){
		GStartIndividual::load_(&cp);
		return *this;
	}

	/*******************************************************************************************/
	/**
	 * Checks for equality with another GStartIndividual object.
	 *
	 * NOTE: THIS FUNCTION IS OPTIONAL AND IS MAINLY USED IN CONJUNCTION WITH UNIT TESTS.
	 * You do not need it if you do not intend to perform unit tests.
	 *
	 * @param  cp A constant reference to another GStartIndividual object
	 * @return A boolean indicating whether both objects are equal
	 */
	bool operator==(const GStartIndividual& cp) const {
		using namespace Gem::Common;
		// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
		return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GStartIndividual::operator==","cp", CE_SILENT);
	}

	/*******************************************************************************************/
	/**
	 * Checks for inequality with another GStartIndividual object.
	 *
	 * NOTE: THIS FUNCTION IS OPTIONAL AND IS MAINLY USED IN CONJUNCTION WITH UNIT TESTS.
	 * You do not need it if you do not intend to perform unit tests.
	 *
	 * @param  cp A constant reference to another GStartIndividual object
	 * @return A boolean indicating whether both objects are inequal
	 */
	bool operator!=(const GStartIndividual& cp) const {
		using namespace Gem::Common;
		// Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
		return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GStartIndividual::operator!=","cp", CE_SILENT);
	}

	/*******************************************************************************************/
	/**
	 * Checks whether a given expectation for the relationship between this object and another object
	 * is fulfilled.
	 *
	 * NOTE: THIS FUNCTION IS OPTIONAL AND IS MAINLY USED IN CONJUNCTION WITH UNIT TESTS.
	 * You do not need it if you do not intend to perform unit tests.
	 *
	 * @param cp A constant reference to another object, camouflaged as a GObject
	 * @param e The expected outcome of the comparison
	 * @param limit The maximum deviation for floating point values (important for similarity checks)
	 * @param caller An identifier for the calling entity
	 * @param y_name An identifier for the object that should be compared to this one
	 * @param withMessages Whether or not information should be emitted in case of deviations from the expected outcome
	 * @return A boost::optional<std::string> object that holds a descriptive string if expectations were not met
	 */
	boost::optional<std::string> checkRelationshipWith(const GObject& cp,
			const Gem::Common::expectation& e,
			const double& limit,
			const std::string& caller,
			const std::string& y_name,
			const bool& withMessages) const
	{
	    using namespace Gem::Common;

		// Check that we are not accidently assigning this object to itself
		selfAssignmentCheck<GStartIndividual>(&cp);
		// Use this call instead when local data needs to be loaded:
		// const GStartIndividual *p_load = GObject::conversion_cast<GStartIndividual>(&cp);

		// Will hold possible deviations from the expectation, including explanations
	    std::vector<boost::optional<std::string> > deviations;

		// Check our parent class'es data ...
		deviations.push_back(GParameterSet::checkRelationshipWith(cp, e, limit, "GStartIndividual", y_name, withMessages));

		// Check local data like this:
		// deviations.push_back(checkExpectation(withMessages, "GStartIndividual", val_, p_load->val_, "val_", "p_load->val_", e , limit));

		return evaluateDiscrepancies("GStartIndividual", caller, deviations, e);
	}


#ifdef GENEVATESTING

	/*******************************************************************************************/
	/**
	 * Applies modifications to this object. This is needed for testing purposes
	 *
	 * @return A boolean which indicates whether modifications were made
	 */
	virtual bool modify_GUnitTests() {
		bool result;

		// Call the parent class'es function
		if(GParameterSet::modify_GUnitTests()) result = true;

		// Check that this individual actually contains data to be modified
		if(this->size() != 0) {
			adapt(); // Perform modifications
			result = true;
		}

		return result;
	}

	/*******************************************************************************************/
	/**
	 * Performs self tests that are expected to succeed. This is needed for testing purposes
	 */
	virtual void specificTestsNoFailureExpected_GUnitTests() {
		const boost::uint32_t NITERATIONS=100;

		// Call the parent class'es function
		GParameterSet::specificTestsNoFailureExpected_GUnitTests();

		// Create an individual
		boost::shared_ptr<Gem::Geneva::GStartIndividual> p
			= boost::shared_ptr<Gem::Geneva::GStartIndividual>(new GStartIndividual(1000, -10, 10));

		// Adapt a number of times and check that there were changes
		double oldfitness = p->fitness();
		for(boost::uint32_t i=0; i<NITERATIONS; i++) {
			p->adapt();
			double newfitness = p->fitness();
			BOOST_CHECK_MESSAGE(newfitness != oldfitness, "Rare failures are normal for this test / " << i << "/" << NITERATIONS);
			oldfitness = newfitness;
		}
	}

	/*******************************************************************************************/
	/**
	 * Performs self tests that are expected to fail. This is needed for testing purposes
	 */
	virtual void specificTestsFailuresExpected_GUnitTests() {
		// Call the parent class'es function
		GParameterSet::specificTestsFailuresExpected_GUnitTests();
	}

#endif /* GENEVATESTING */


protected:
	/********************************************************************************************/
	/**
	 * Loads the data of another GStartIndividual, camouflaged as a GObject.
	 *
	 * @param cp A copy of another GStartIndividual, camouflaged as a GObject
	 */
	virtual void load_(const GObject* cp)
	{
		// Check that we are not accidently assigning this object to itself
		selfAssignmentCheck<GStartIndividual>(cp);
		// Use this call instead when local data needs to be loaded:
		// const GStartIndividual *p_load = GObject::conversion_cast<GStartIndividual>(cp);

		// Load our parent's data
		GParameterSet::load_(cp);

		// Load local data here like this:
		// myVar = p_load->myVar;
	}

	/********************************************************************************************/
	/**
	 * Creates a deep clone of this object
	 *
	 * @return A deep clone of this object, camouflaged as a GObject
	 */
	virtual GObject* clone_() const {
		return new GStartIndividual(*this);
	}

	/********************************************************************************************/
	/**
	 * The actual fitness calculation takes place here.
	 *
	 * @return The value of this object
	 */
	virtual double fitnessCalculation(){
		double result = 0.;

		// Extract the GConstrainedDoubleObjectCollection object. In a realistic scenario, you might want
		// to add error checks here upon first invocation.
		/*
		boost::shared_ptr<GConstrainedDoubleObjectCollection> vC = at<GConstrainedDoubleObjectCollection>(0);

		// Calculate the value of the parabola
		for(std::size_t i=0; i<vC->size(); i++)
			result += vC->at(i)->value() * vC->at(i)->value();
		 */

		// boost::shared_ptr<GParameterObjectCollection> vC = at<GParameterObjectCollection>(0);
		// GParameterObjectCollection::conversion_iterator<GConstrainedDoubleObject> it(vC->end());
		GStartIndividual::conversion_iterator<GConstrainedDoubleObject> it(this->end());
		// for(it=vC->begin(); it!=vC->end(); ++it) {
		for(it=this->begin(); it!=this->end(); ++it) {
			result += (*it)->value() * (*it)->value();
		}

		return result;
	}

private:
	/********************************************************************************************/
	/**
	 * The default constructor. Intentionally private and empty, as it is only needed for
	 * serialization purposes.
	 */
	GStartIndividual() :GParameterSet()
	{	/* nothing */ }

	/********************************************************************************************/
	// You can add other variables here. Do not forget to serialize them if necessary
	// int myVar;
};

/*************************************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

// Needed for serialization purposes
#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(Gem::Geneva::GStartIndividual)

// Needed for testing purposes
/*************************************************************************************************/
///////////////////////////////////////////////////////////////////////////////////////////////////
/*************************************************************************************************/

#ifdef GENEVATESTING

/**
 * As the Gem::Geneva::Gem::Geneva::GStartIndividual has a private default constructor, we need to provide a
 * specialization of the factory function that creates GStartProjectIndividual objects
 */
template <>
boost::shared_ptr<Gem::Geneva::GStartIndividual> TFactory_GUnitTests<Gem::Geneva::GStartIndividual>() {
	return boost::shared_ptr<Gem::Geneva::GStartIndividual>(new Gem::Geneva::GStartIndividual(1000,-10.,10.));
}

#endif /* GENEVATESTING */

/*************************************************************************************************/
///////////////////////////////////////////////////////////////////////////////////////////////////
/*************************************************************************************************/

#endif /* GSTARTINDIVIDUAL_HPP_ */
