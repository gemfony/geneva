/**
 * @file GNeuralNetworkIndividual.cpp
 */

/* Copyright (C) Dr. Ruediger Berlich and Karlsruhe Institute of Technology
 * (University of the State of Baden-Wuerttemberg and National Laboratory
 * of the Helmholtz Association)
 *
 * Contact: info [at] gemfony (dot) com
 *
 * This file is part of the Geneva library, Gemfony scientific's optimization
 * library.
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

#include "GNeuralNetworkIndividual.hpp"

// Make the class known to the boost serialization framework
#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(Gem::GenEvA::GNeuralNetworkIndividual<Gem::GenEvA::SIGMOID>)
BOOST_CLASS_EXPORT(Gem::GenEvA::GNeuralNetworkIndividual<Gem::GenEvA::RBF>)

namespace Gem {
namespace GenEvA {

/************************************************************************************************/
//////////////////////////////////////////////////////////////////////////////////////////////////
/************************************************************************************************/
/**
 * The destructor -- empty, as there is no local, dynamically
 * allocated data. A virtual destructor is needed due to a
 * serialization problem in Boost 1.41
 */
trainingSet::~trainingSet()
{ /* nothing */ }

/************************************************************************************************/
/**
 * Assigns another trainingSet's data to this object
 *
 * @param cp A copy of another trainingSet object
 * @return A constant reference to this object
 */
const trainingSet& trainingSet::operator=(const trainingSet& cp) {
	Input = cp.Input;
	Output = cp.Output;

	return *this;
}

/************************************************************************************************/
/**
 * Checks for equality with another trainingSet object
 *
 * @param  cp A constant reference to another trainingSet object
 * @return A boolean indicating whether both objects are equal
 */
bool trainingSet::operator==(const trainingSet& cp) const {
	using namespace Gem::Util;
	// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"trainingSet::operator==","cp", CE_SILENT);
}

/************************************************************************************************/
/**
 * Checks for inequality with another trainingSet object
 *
 * @param  cp A constant reference to another trainingSet object
 * @return A boolean indicating whether both objects are inequal
 */
bool trainingSet::operator!=(const trainingSet& cp) const {
	using namespace Gem::Util;
	// Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"trainingSet::operator!=","cp", CE_SILENT);
}
/************************************************************************************************/
/**
 * Checks whether a given expectation for the relationship between this object and another object
 * is fulfilled.
 *
 * @param cp A constant reference to another object, camouflaged as a GObject
 * @param e The expected outcome of the comparison
 * @param limit The maximum deviation for floating point values (important for similarity checks)
 * @param caller An identifier for the calling entity
 * @param y_name An identifier for the object that should be compared to this one
 * @param withMessages Whether or not information should be emitted in case of deviations from the expected outcome
 * @return A boost::optional<std::string> object that holds a descriptive string if expectations were not met
 */
boost::optional<std::string> trainingSet::checkRelationshipWith(const trainingSet& cp,
		const Gem::Util::expectation& e,
		const double& limit,
		const std::string& caller,
		const std::string& y_name,
		const bool& withMessages) const
{
    using namespace Gem::Util;
    using namespace Gem::Util::POD;

	// Will hold possible deviations from the expectation, including explanations
    std::vector<boost::optional<std::string> > deviations;

	// Check local data
	deviations.push_back(checkExpectation(withMessages, "trainingSet", Input, cp.Input, "Input", "cp.Input", e , limit));
	deviations.push_back(checkExpectation(withMessages, "trainingSet", Output, cp.Output, "Output", "cp.Output", e , limit));

	return evaluateDiscrepancies("trainingSet", caller, deviations, e);
}

/************************************************************************************************/
//////////////////////////////////////////////////////////////////////////////////////////////////
/************************************************************************************************/
/**
 * Initializes the object with data from a file
 *
 * @param trainingDataFile The name of a file holding the training data
 */
trainingData::trainingData(const std::string& trainingDataFile) {
	this->loadFromDisk(trainingDataFile);
}

/************************************************************************************************/
/**
 * Initializes with data from another trainingData object
 *
 * @param cp A copy of another trainingData object
 */
trainingData::trainingData(const trainingData& cp) {
	Gem::Util::copySmartPointerVector(cp.data, data);
}

/************************************************************************************************/
/**
 * A standard destructor. Declared virtual in the header
 * due to a serialization problem in Boost 1.41.
 */
trainingData::~trainingData()
{ /* nothing */ }

/************************************************************************************************/
/**
 * Copies the data of another trainingData object into this object, using one of Gemfony's
 * utility functions.
 *
 * @param cp A copy of another trainingData object
 * @return A constant reference to this object
 */
const trainingData& trainingData::operator=(const trainingData& cp) {
	Gem::Util::copySmartPointerVector(cp.data, data);
	return *this;
}

/************************************************************************************************/
/**
 * Checks for equality with another trainingData object
 *
 * @param  cp A constant reference to another trainingData object
 * @return A boolean indicating whether both objects are equal
 */
bool trainingData::operator==(const trainingData& cp) const {
	using namespace Gem::Util;
	// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"trainingData::operator==","cp", CE_SILENT);
}

/************************************************************************************************/
/**
 * Checks for inequality with another trainingData object
 *
 * @param  cp A constant reference to another trainingData object
 * @return A boolean indicating whether both objects are inequal
 */
bool trainingData::operator!=(const trainingData& cp) const {
	using namespace Gem::Util;
	// Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"trainingData::operator!=","cp", CE_SILENT);
}
/************************************************************************************************/
/**
 * Checks whether a given expectation for the relationship between this object and another object
 * is fulfilled.
 *
 * @param cp A constant reference to another object, camouflaged as a GObject
 * @param e The expected outcome of the comparison
 * @param limit The maximum deviation for floating point values (important for similarity checks)
 * @param caller An identifier for the calling entity
 * @param y_name An identifier for the object that should be compared to this one
 * @param withMessages Whether or not information should be emitted in case of deviations from the expected outcome
 * @return A boost::optional<std::string> object that holds a descriptive string if expectations were not met
 */
boost::optional<std::string> trainingData::checkRelationshipWith(const trainingData& cp,
		const Gem::Util::expectation& e,
		const double& limit,
		const std::string& caller,
		const std::string& y_name,
		const bool& withMessages) const
{
    using namespace Gem::Util;
    using namespace Gem::Util::POD;

	// Will hold possible deviations from the expectation, including explanations
    std::vector<boost::optional<std::string> > deviations;

    // Check vector sizes
    if(data.size() != cp.data.size()) {
    	std::ostringstream error;
    	error << "Vector sizes did not match in trainingData::checkRelationshipWith(): " << data.size() << " / " << cp.data.size();
    	deviations.push_back(boost::optional<std::string>(error.str()));
    }
    else {
    	// Check local data
    	std::vector<boost::shared_ptr<trainingSet> >::iterator it;
    	std::vector<boost::shared_ptr<trainingSet> >::const_iterator cit;
    	for(it=data.begin(), cit=cp.data.begin(); it!=data.end(); ++it, ++cit) {
    		deviations.push_back((*it)->checkRelationshipWith(**cit, e, limit, y_name, withMessages));
    	}
    }

	return evaluateDiscrepancies("trainingData", caller, deviations, e);
}

/************************************************************************************************/
/**
 * Saves the data of this struct to disc
 *
 * @param fileName The name of the file that data should be saved to
 */
void trainingData::saveToDisk(const std::string& trainingDataFile) const {
	std::ofstream trDat(trainingDataFile.c_str());

	if(!trDat){
		std::ostringstream error;
		error << "In trainingData::saveToDisk(const std::string&) : Error!" << std::endl
			  << "Data file " << trainingDataFile << " could not be opened for writing." << std::endl;

		throw geneva_error_condition(error.str());
	}

	// Load the data, using the Boost.Serialization library
	{
		boost::archive::xml_oarchive oa(trDat);
		oa << boost::serialization::make_nvp("trainingData", this);
	} // Explicit scope at this point is essential so that ia's destructor is called

	trDat.close();
}

/************************************************************************************************/
/**
 * Loads training data from the disc
 *
 * @param fileName The name of the file from which data should be loaded
 */
void trainingData::loadFromDisk(const std::string& trainingDataFile) {
	trainingData *tD;
	std::ifstream trDat(trainingDataFile.c_str());

	if(!trDat){
		std::ostringstream error;
		error << "In trainingData::loadFromDisk(const std::string&) : Error!" << std::endl
			  << "Data file " << trainingDataFile << " could not be opened for reading." << std::endl;

		throw geneva_error_condition(error.str());
	}

	// Load the data into tD, using the Boost.Serialization library
	{
		boost::archive::xml_iarchive ia(trDat);
		ia >> boost::serialization::make_nvp("trainingData", tD);
	} // Explicit scope at this point is essential so that ia's destructor is called

	trDat.close();

	// Copy the data over, using our own operator=()
	*this = *tD;

	// Clean up
	delete tD;
}

/************************************************************************************************/
//////////////////////////////////////////////////////////////////////////////////////////////////
/************************************************************************************************/
/**
 * Specialization of the transfer function for case tF==SIGMOID
 *
 * @param value The value to be transferred
 * @return The transferred value
 */
template <>
double GNeuralNetworkIndividual<SIGMOID>::transfer(const double& value) const {
	return 1./(1.+exp(-value));
}

/************************************************************************************************/
/**
 * Specialization of the transfer function for case tF==RBF
 *
 * @param value The value to be transferred
 * @return The transferred value
 */
template <>
double GNeuralNetworkIndividual<RBF>::transfer(const double& value) const {
	return exp(-GSQUARED(value));
}

/************************************************************************************************/
//////////////////////////////////////////////////////////////////////////////////////////////////
/************************************************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */
