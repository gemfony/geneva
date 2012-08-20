/**
 * @file GNeuralNetworkIndividual.cpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) com
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
 * http://www.gemfony.com .
 */


#include <geneva-individuals/GNeuralNetworkIndividual.hpp>

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::trainingSet)
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::networkData)
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GNeuralNetworkIndividual<Gem::Geneva::SIGMOID>)
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GNeuralNetworkIndividual<Gem::Geneva::RBF>)

namespace Gem {
namespace Geneva {

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
	using namespace Gem::Common;
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
	using namespace Gem::Common;
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
		const Gem::Common::expectation& e,
		const double& limit,
		const std::string& caller,
		const std::string& y_name,
		const bool& withMessages) const
{
    using namespace Gem::Common;

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
 * The default constructor
 */
networkData::networkData()
{ /* nothing */ }

/************************************************************************************************/
/**
 * Initializes the object with data from a file
 *
 * @param networkDataFile The name of a file holding the training data
 */
networkData::networkData(const std::string& networkDataFile) {
	this->loadFromDisk(networkDataFile);
}

/************************************************************************************************/
/**
 * Initializes with data from another networkData object
 *
 * @param cp A copy of another networkData object
 */
networkData::networkData(const networkData& cp)
	: GStdSimpleVectorInterfaceT<std::size_t>(cp)
	, currentIndex_(cp.currentIndex_)
{
	Gem::Common::copySmartPointerVector(cp.data_, data_);
}

/************************************************************************************************/
/**
 * A standard destructor. Declared virtual in the header
 * due to a serialization problem in Boost 1.41.
 */
networkData::~networkData()
{ /* nothing */ }

/************************************************************************************************/
/**
 * Copies the data of another networkData object into this object, using one of Gemfony's
 * utility functions.
 *
 * @param cp A copy of another networkData object
 * @return A constant reference to this object
 */
const networkData& networkData::operator=(const networkData& cp) {
	GStdSimpleVectorInterfaceT<std::size_t>::operator=(cp);
	Gem::Common::copySmartPointerVector(cp.data_, data_);
	currentIndex_=cp.currentIndex_;
	return *this;
}

/************************************************************************************************/
/**
 * Checks for equality with another networkData object
 *
 * @param  cp A constant reference to another networkData object
 * @return A boolean indicating whether both objects are equal
 */
bool networkData::operator==(const networkData& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"networkData::operator==","cp", CE_SILENT);
}

/************************************************************************************************/
/**
 * Checks for inequality with another networkData object
 *
 * @param  cp A constant reference to another networkData object
 * @return A boolean indicating whether both objects are inequal
 */
bool networkData::operator!=(const networkData& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"networkData::operator!=","cp", CE_SILENT);
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
boost::optional<std::string> networkData::checkRelationshipWith(const networkData& cp,
		const Gem::Common::expectation& e,
		const double& limit,
		const std::string& caller,
		const std::string& y_name,
		const bool& withMessages) const
{
    using namespace Gem::Common;

	// Will hold possible deviations from the expectation, including explanations
    std::vector<boost::optional<std::string> > deviations;

	// Check the parent class'es data
	deviations.push_back(GStdSimpleVectorInterfaceT<std::size_t>::checkRelationshipWith(cp, e, limit, "networkData", y_name, withMessages));

    // Check vector sizes
    if(data_.size() != cp.data_.size()) {
    	std::ostringstream error;
    	error << "Vector sizes did not match in networkData::checkRelationshipWith(): " << data_.size() << " / " << cp.data_.size();
    	deviations.push_back(boost::optional<std::string>(error.str()));
    }
    else {
    	// Check local data
    	std::vector<boost::shared_ptr<trainingSet> >::const_iterator it;
    	std::vector<boost::shared_ptr<trainingSet> >::const_iterator cit;
    	for(it=data_.begin(), cit=cp.data_.begin(); it!=data_.end(); ++it, ++cit) {
    		deviations.push_back((*it)->checkRelationshipWith(**cit, e, limit, "networkData", y_name, withMessages));
    	}
    }

	return evaluateDiscrepancies("networkData", caller, deviations, e);
}

/************************************************************************************************/
/**
 * Saves the data of this struct to disc
 *
 * @param fileName The name of the file that data should be saved to
 */
void networkData::saveToDisk(const std::string& networkDataFile) const {
	std::ofstream trDat(networkDataFile.c_str());

	if(!trDat.good()){
		raiseException(
				"In networkData::saveToDisk(const std::string&) : Error!" << std::endl
				<< "Data file " << networkDataFile << " could not be opened for writing."
		);
	}

	// Load the data, using the Boost.Serialization library
	{
		const networkData *local = this;
		boost::archive::xml_oarchive oa(trDat);
		oa << boost::serialization::make_nvp("networkData", local);
	} // Explicit scope at this point is essential so that ia's destructor is called

	trDat.close();
}

/************************************************************************************************/
/**
 * Loads training data from the disc
 *
 * @param fileName The name of the file from which data should be loaded
 */
void networkData::loadFromDisk(const std::string& networkDataFile) {
	networkData *nD;

	std::ifstream trDat(networkDataFile.c_str());

	if(!trDat.good()){
		std::ostringstream error;
		error << "In networkData::loadFromDisk(const std::string&):" << std::endl
			  << "Data file " << networkDataFile << " could not be opened for reading." << std::endl;

		if(!boost::filesystem::exists(networkDataFile.c_str())) {
			error << "File does not exist." << std::endl;
		}

		raiseException(error.str());
	}

	// Load the data into nD, using the Boost.Serialization library
	{
		boost::archive::xml_iarchive ia(trDat);
		ia >> boost::serialization::make_nvp("networkData", nD);
	} // Explicit scope at this point is essential so that ia's destructor is called

	trDat.close();

	// Copy the data over, using our own operator=()
	*this = *nD;

	// Clean up
	delete nD;
}

/************************************************************************************************/
/**
 * Adds a new training set to the collection, Requires for the network architecture to be
 * defined already
 *
 * @param tS A boost::shared_ptr<trainingSet> object, pointing to a training set
 */
void networkData::addTrainingSet(boost::shared_ptr<trainingSet> tS) {
	data_.push_back(tS);
}

/************************************************************************************************/
/**
 * Retrieves the next training set
 *
 * @return The next training set in the list
 */
boost::optional<boost::shared_ptr<trainingSet> > networkData::getNextTrainingSet() const {
	std::vector<boost::shared_ptr<trainingSet> >::const_iterator currentIterator = data_.begin() + currentIndex_;
	if(currentIterator != data_.end()) {
		boost::optional<boost::shared_ptr<trainingSet> > o = *currentIterator;
		currentIndex_++;
		return o;
	}
	else {
		resetCurrentIndex();
		return boost::optional<boost::shared_ptr<trainingSet> >();
	}
}

/************************************************************************************************/
/**
 * Resets the index of the current training set, so that upon next call to getNextTrainingSet()
 * the first training set in the list is returned.
 */
void networkData::resetCurrentIndex() const {
	currentIndex_ = 0;
}


/************************************************************************************************/
/**
 * Retrieves the number of input nodes of this network
 *
 * @return The number of input nodes of this network
 */
std::size_t networkData::getNInputNodes() const {
	return this->front();
}

/************************************************************************************************/
/**
 * Retrieves the number of output nodes of this network
 *
 * @return The number of output nodes of this network
 */
std::size_t networkData::getNOutputNodes() const {
	return this->back();
}

/************************************************************************************************/
/**
 * Saves this data set in ROOT format for visual inspection. It assumes that the input dimension
 * is 2 and the output dimension is 1. It will generate two distributions that will be coloured
 * differently -- one with output < 0.5, the other with output >= 0.5.
 *
 * @param outputFile The name of the file used for the visualization of the input data
 * @param min The minimum value of the distribution to be displayed
 * @param max The maximum value of the distribution to be displayed
 */
void networkData::toRoot(
		  const std::string& outputFile
		, const double& min
		, const double& max
) {
	// Check that we have a matching number of input nodes
	if(getNInputNodes() != 2 || getNOutputNodes() != 1) {
		std::cerr << "In networkData::toRoot(): Warning!" << std::endl
  		          << "Got inappropriate number of input and/or output nodes: " << getNInputNodes() << "/" << getNOutputNodes() << std::endl
  		          << "We need 2/1. The function will return without further action." << std::endl;
		return;
	}

	std::ofstream of(outputFile.c_str());

	of << "{" << std::endl
	   << "  std::vector<double> x1_vec, y1_vec, x2_vec, y2_vec;" << std::endl
	   << "  TH2F *h2_one = new TH2F(\"h2_one\",\"h2_one\",100," << min << ", " << max << ", 100, " << min << ", " << max << ");" << std::endl
	   << "  TH2F *h2_two = new TH2F(\"h2_two\",\"h2_two\",100," << min << ", " << max << ", 100, " << min << ", " << max << ");" << std::endl
	   << std::endl
	   << "  // Filling the data sets" << std::endl;

	for(std::size_t i=0; i<data_.size(); i++) {
		if(data_[i]->Output[0] < 0.5) {
			of << "  x1_vec.push_back(" << data_[i]->Input[0] << ");" << std::endl
			   << "  y1_vec.push_back(" << data_[i]->Input[1] << ");" << std::endl
			   << "  h2_one->Fill(" << data_[i]->Input[0] << ", " << data_[i]->Input[1] << ");" << std::endl;
		}
		else {
			of << "  x2_vec.push_back(" << data_[i]->Input[0] << ");" << std::endl
			   << "  y2_vec.push_back(" << data_[i]->Input[1] << ");" << std::endl
			   << "  h2_two->Fill(" << data_[i]->Input[0] << ", " << data_[i]->Input[1] << ");" << std::endl;
		}
	}

	of << std::endl
	   << "  // Transfer into arrays suitable for printing with a TGraph" << std::endl
	   << "  std::size_t n1Entries = x1_vec.size();" << std::endl
	   << "  std::size_t n2Entries = x2_vec.size();" << std::endl
	   << std::endl
	   << "  double x1[n1Entries], y1[n1Entries], x2[n2Entries], y2[n2Entries];" << std::endl
	   << std::endl
	   << "  for(std::size_t i=0; i<n1Entries; i++) {" << std::endl
	   << "    x1[i] = x1_vec[i];" << std::endl
	   << "    y1[i] = y1_vec[i];" << std::endl
	   << "  }" << std::endl
	   << std::endl
	   << "  for(std::size_t i=0; i<n2Entries; i++) {" << std::endl
	   << "    x2[i] = x2_vec[i];" << std::endl
	   << "    y2[i] = y2_vec[i];" << std::endl
	   << "  }" << std::endl
	   << std::endl
	   << "  // Creation of suitable TGraph objects" << std::endl
	   << "  TGraph *gr1 = new TGraph(n1Entries, x1, y1);" << std::endl
	   << "  TGraph *gr2 = new TGraph(n2Entries, x2, y2);" << std::endl
	   << std::endl
	   << "  gr1->SetMarkerColor(17);" << std::endl
	   << "  gr2->SetMarkerColor(12);" << std::endl
	   << std::endl
	   << "  gr1->SetMarkerStyle(21);" << std::endl
	   << "  gr2->SetMarkerStyle(21);" << std::endl
	   << std::endl
	   << "  gr1->SetMarkerSize(0.3);" << std::endl
	   << "  gr2->SetMarkerSize(0.35);" << std::endl
	   << std::endl
	   << "  gr2->GetXaxis()->SetRangeUser(" << min << ", " << max << ");" << std::endl
	   << "  gr2->GetYaxis()->SetRangeUser(" << min << ", " << max << ");" << std::endl
	   << std::endl
	   << "  // Do the drawing" << std::endl
	   << "  gr2->Draw(\"AP\");" << std::endl
	   << "  gr1->Draw(\"P,same\");" << std::endl
	   << "}" << std::endl;

	of.close();
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


/*************************************************************************************************/
///////////////////////////////////////////////////////////////////////////////////////////////////
/*************************************************************************************************/
// For testing purposes

#ifdef GEM_TESTING

/**
 * As the Gem::Geneva::Gem::Geneva::GNeuralNetworkIndividual<Gem::Geneva::SIGMOID> has a private default constructor, we need to provide a
 * specialization of the factory function that creates GStartProjectIndividual objects
 */
template <>
boost::shared_ptr<Gem::Geneva::GNeuralNetworkIndividual<Gem::Geneva::SIGMOID> > TFactory_GUnitTests<Gem::Geneva::GNeuralNetworkIndividual<Gem::Geneva::SIGMOID> >() {
	return boost::shared_ptr<Gem::Geneva::GNeuralNetworkIndividual<Gem::Geneva::SIGMOID> >(new Gem::Geneva::GNeuralNetworkIndividual<Gem::Geneva::SIGMOID>("../../DataSets/training.dat",-1.,1., 2.,0.8,0.001, 2., 0.05));
}

/*************************************************************************************************/
/**
 * As the Gem::Geneva::Gem::Geneva::GNeuralNetworkIndividual<Gem::Geneva::RBF> has a private default constructor, we need to provide a
 * specialization of the factory function that creates GStartProjectIndividual objects
 */
template <>
boost::shared_ptr<Gem::Geneva::GNeuralNetworkIndividual<Gem::Geneva::RBF> > TFactory_GUnitTests<Gem::Geneva::GNeuralNetworkIndividual<Gem::Geneva::RBF> >() {
	return boost::shared_ptr<Gem::Geneva::GNeuralNetworkIndividual<Gem::Geneva::RBF> >(new Gem::Geneva::GNeuralNetworkIndividual<Gem::Geneva::RBF>("../../DataSets/training.dat",-1.,1., 2.,0.8,0.001, 2., 0.05));
}

#endif /* GEM_TESTING */

/*************************************************************************************************/
///////////////////////////////////////////////////////////////////////////////////////////////////
/*************************************************************************************************/
/**
 * Reads a Gem::Geneva::trainingDataType item from a stream. Needed so we
 * can use boost::program_options to read trainingDataType data.
 *
 * @param i The stream the item should be read from
 * @param tdt The item read from the stream
 * @return The std::istream object used to read the item from
 */
std::istream& operator>>(std::istream& i, Gem::Geneva::trainingDataType& tdt){
	boost::uint16_t tmp;
	i >> tmp;

#ifdef DEBUG
	tdt = boost::numeric_cast<Gem::Geneva::trainingDataType>(tmp);
#else
	tdt = static_cast<Gem::Geneva::trainingDataType>(tmp);
#endif /* DEBUG */

	return i;
}

/************************************************************************************************/
/**
 * Puts a Gem::Geneva::trainingDataType item into a stream. Needed so we
 * can use boost::program_options to output trainingDataType data.
 *
 * @param o The ostream the item should be added to
 * @param tdt the item to be added to the stream
 * @return The std::ostream object used to add the item to
 */
std::ostream& operator<<(std::ostream& o, const Gem::Geneva::trainingDataType& tdt){
	boost::uint16_t tmp = static_cast<boost::uint16_t>(tdt);
	o << tmp;
	return o;
}

/************************************************************************************************/
/**
 * Reads a Gem::Geneva::transferFunction item from a stream. Needed so we
 * can use boost::program_options to read transferFunction data.
 *
 * @param i The stream the item should be read from
 * @param tF The item read from the stream
 * @return The std::istream object used to read the item from
 */
std::istream& operator>>(std::istream& i, Gem::Geneva::transferFunction& tF){
	boost::uint16_t tmp;
	i >> tmp;

#ifdef DEBUG
	tF = boost::numeric_cast<Gem::Geneva::transferFunction>(tmp);
#else
	tF = static_cast<Gem::Geneva::transferFunction>(tmp);
#endif /* DEBUG */

	return i;
}

/************************************************************************************************/
/**
 * Puts a Gem::Geneva::transferFunction item into a stream. Needed so we
 * can use boost::program_options to output transferFunction data.
 *
 * @param o The ostream the item should be added to
 * @param tF the item to be added to the stream
 * @return The std::ostream object used to add the item to
 */
std::ostream& operator<<(std::ostream& o, const Gem::Geneva::transferFunction& tF){
	boost::uint16_t tmp = static_cast<boost::uint16_t>(tF);
	o << tmp;
	return o;
}

/*************************************************************************************************/
///////////////////////////////////////////////////////////////////////////////////////////////////
/*************************************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */
