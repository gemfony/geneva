/**
 * @file GExternalEvaluatorIndividual.cpp
 */

/*
 * Copyright (C) Authors of the Geneva library collection and Karlsruhe
 * Institute of Technology (University of the State of Baden-Wuerttemberg
 * and National Laboratory of the Helmholtz Association).
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: info [at] gemfony (dot) com
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

#include "geneva-individuals/GExternalEvaluatorIndividual.hpp"

/* Included here so no conflicts occur */
#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(Gem::Geneva::GExternalEvaluatorIndividual)

namespace Gem
{
namespace Geneva
{

/********************************************************************************************/
/**
 * A constructor which initializes the individual with the name of the external program
 * that should be executed. The external program is asked for the desired structure of
 * the individual, and the corresponding data sets are added. Only one individual
 * needs to be constructed using this method. All other individuals of the population
 * should be created as copies of this first individual.
 *
 * @param program The filename (including path) of the external program that should be executed
 * @param arguments Additional user-defined arguments to be handed to the external program
 * @param random A boolean indicating whether template data should be filled randomly
 * @param gdbl_ad_ptr An adaptor for double values
 * @param glong_ad_ptr An adaptor for boost::int32_t values
 * @param gbool_ad_ptr An adaptor for bool values
 */
GExternalEvaluatorIndividual::GExternalEvaluatorIndividual(
		const std::string& program
		, const std::string& arguments
		, const bool& random
		, const Gem::Dataexchange::dataExchangeMode& exchangeMode
		, boost::shared_ptr<GAdaptorT<double> > gdbl_ad_ptr
		, boost::shared_ptr<GAdaptorT<boost::int32_t> > glong_ad_ptr
		, boost::shared_ptr<GAdaptorT<bool> > gbool_ad_ptr
)
: program_(program)
, arguments_(arguments)
, exchangeMode_(exchangeMode)
, parameterFile_("./parameterData")
{
	//-----------------------------------------------------------------------------------------------
	// Create the required, empty collections.
	boost::shared_ptr<GConstrainedDoubleObjectCollection> gbdc_ptr(new GConstrainedDoubleObjectCollection());
	boost::shared_ptr<GConstrainedInt32ObjectCollection> gbic_ptr(new GConstrainedInt32ObjectCollection());
	boost::shared_ptr<GBooleanCollection> gbc_ptr(new GBooleanCollection());

	// Set up the local adaptor templates and collection items
	//-----------------------------------------------------------------------------------------------
	gdbl_ptr_=boost::shared_ptr<GConstrainedDoubleObject>(new GConstrainedDoubleObject());
	if(gdbl_ad_ptr) {
		gdbl_ptr_->addAdaptor(gdbl_ad_ptr->GObject::clone<GAdaptorT<double> >());
	}
	else {
		gdbl_ptr_->addAdaptor(boost::shared_ptr<GAdaptorT<double> >(new GDoubleGaussAdaptor())); // uses default values
	}

	//-----------------------------------------------------------------------------------------------
	glong_ptr_=boost::shared_ptr<GConstrainedInt32Object>(new GConstrainedInt32Object());
	if(glong_ad_ptr) {
		glong_ptr_->addAdaptor(glong_ad_ptr->GObject::clone<GAdaptorT<boost::int32_t> >());
	}
	else {
		glong_ptr_->addAdaptor(boost::shared_ptr<GAdaptorT<boost::int32_t> >(new GInt32FlipAdaptor())); // uses default values
	}

	//-----------------------------------------------------------------------------------------------
	// GBooleanCollection is special in that it always directly contains adaptors
	if(gbool_ad_ptr) {
		gbc_ptr->addAdaptor(gbool_ad_ptr->GObject::clone<GAdaptorT<bool> >());
	}
	else {
		gbc_ptr->addAdaptor(boost::shared_ptr<GAdaptorT<bool> >(new GBooleanAdaptor())); // uses default values
	}

	//-----------------------------------------------------------------------------------------------

	// Add the collections to the class
	this->push_back(gbdc_ptr);
	this->push_back(gbic_ptr);
	this->push_back(gbc_ptr);

	//-----------------------------------------------------------------------------------------------------------------------------
	// Tell the external program to send us a template with the structure of the individual

	GExternalEvaluatorIndividual::checkStringIsValid(program_); // Do some error checking

	std::string commandLine;
	if(exchangeMode_ == Gem::Dataexchange::BINARYEXCHANGE) {
		commandLine = program_ + " -m 0 -t " +(random?std::string(" -R"):std::string("")) + " -p " + parameterFile_;
	}
	else {
		commandLine = program_ + " -m 1 -t " +(random?std::string(" -R"):std::string("")) + " -p " + parameterFile_;
	}
	if(!arguments.empty() && arguments != "empty") commandLine += (" " + arguments_);

	Gem::Common::runExternalCommand(commandLine);

	//-----------------------------------------------------------------------------------------------------------------------------
	// Finally fill this class up with the external template data. Make sure the data doesn't get sorted
	bool hasValue;
	this->readParametersFromFile(parameterFile_, hasValue, false);

	// Erase the parameter file - not needed anymore.
	bf::path p(parameterFile_.c_str());
#ifdef DEBUG
	if(!bf::exists(p)) {
		std::ostringstream error;
		error << "In GExternalEvaluatorIndividual constructor: Error!" << std::endl
				<< "Tried to erase non-existant parameter file " << parameterFile_ << std::endl;
		throw(Gem::Common::gemfony_error_condition(error.str()));
	}
#endif /* DEBUG */
	bf::remove(p);
}

/********************************************************************************************/
/**
 * A standard copy constructor.
 */
GExternalEvaluatorIndividual::GExternalEvaluatorIndividual(const GExternalEvaluatorIndividual& cp)
: GParameterSet(cp) // copies all local collections
, program_(cp.program_)
, arguments_(cp.arguments_)
, exchangeMode_(cp.exchangeMode_)
, parameterFile_(cp.parameterFile_)
{
	gdbl_ptr_ = cp.gdbl_ptr_->GObject::clone<GConstrainedDoubleObject>();
	glong_ptr_ = cp.glong_ptr_->GObject::clone<GConstrainedInt32Object>();
}

/********************************************************************************************/
/**
 * The standard destructor
 */
GExternalEvaluatorIndividual::~GExternalEvaluatorIndividual()
{ /* nothing */	}


/********************************************************************************************/
/**
 * A standard assignment operator
 */
const GExternalEvaluatorIndividual& GExternalEvaluatorIndividual::operator=(const GExternalEvaluatorIndividual& cp){
	GExternalEvaluatorIndividual::load_(&cp);
	return *this;
}

/********************************************************************************************/
/**
 * Checks for equality with another GExternalEvaluatorIndividual object
 *
 * @param  cp A constant reference to another GExternalEvaluatorIndividual object
 * @return A boolean indicating whether both objects are equal
 */
bool GExternalEvaluatorIndividual::operator==(const GExternalEvaluatorIndividual& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GExternalEvaluatorIndividual::operator==","cp", CE_SILENT);
}

/********************************************************************************************/
/**
 * Checks for inequality with another GExternalEvaluatorIndividual object
 *
 * @param  cp A constant reference to another GExternalEvaluatorIndividual object
 * @return A boolean indicating whether both objects are inequal
 */
bool GExternalEvaluatorIndividual::operator!=(const GExternalEvaluatorIndividual& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GExternalEvaluatorIndividual::operator!=","cp", CE_SILENT);
}

/********************************************************************************************/
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
boost::optional<std::string> GExternalEvaluatorIndividual::checkRelationshipWith(
		const GObject& cp,
		const Gem::Common::expectation& e,
		const double& limit,
		const std::string& caller,
		const std::string& y_name,
		const bool& withMessages) const
{
	using namespace Gem::Common;

	// Check that we are indeed dealing with a reference to an object of the same type
	const GExternalEvaluatorIndividual *p_load = conversion_cast<GExternalEvaluatorIndividual>(&cp);

	// Will hold possible deviations from the expectation, including explanations
	std::vector<boost::optional<std::string> > deviations;

	// Check our parent class'es data ...
	deviations.push_back(GParameterSet::checkRelationshipWith(cp, e, limit, "GExternalEvaluatorIndividual", y_name, withMessages));

	// ... and then our local data
	deviations.push_back(checkExpectation(withMessages, "GExternalEvaluatorIndividual", program_, p_load->program_, "program_", "p_load->program_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GExternalEvaluatorIndividual", arguments_, p_load->arguments_, "arguments_", "p_load->arguments_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GExternalEvaluatorIndividual", exchangeMode_, p_load->exchangeMode_, "exchangeMode_", "p_load->exchangeMode_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GExternalEvaluatorIndividual", parameterFile_, p_load->parameterFile_, "parameterFile_", "p_load->parameterFile_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GExternalEvaluatorIndividual", gdbl_ptr_, p_load->gdbl_ptr_, "gdbl_ptr_", "p_load->gdbl_ptr_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GExternalEvaluatorIndividual", glong_ptr_, p_load->glong_ptr_, "glong_ptr_", "p_load->glong_ptr_", e , limit));

	return evaluateDiscrepancies("GExternalEvaluatorIndividual", caller, deviations, e);
}

/********************************************************************************************/
/**
 * Sets the exchange mode between this individual and the external program. Allowed
 * values are defined in the Gem::Dataexchange::dataExchangeMode enum. As this allows a comile-time check
 * of supplied arguments, no further error checks are needed.
 *
 * @param The current exchange mode for data
 * @return The previous exchange mode
 */
Gem::Dataexchange::dataExchangeMode GExternalEvaluatorIndividual::setDataExchangeMode(const Gem::Dataexchange::dataExchangeMode& exchangeMode) {
	Gem::Dataexchange::dataExchangeMode current = exchangeMode_;
	exchangeMode_ = exchangeMode;
	return current;
}

/********************************************************************************************/
/**
 * Retrieves the current value of the exchangeMode_ variable.
 *
 * @return The current exchange mode
 */
Gem::Dataexchange::dataExchangeMode GExternalEvaluatorIndividual::getDataExchangeMode() const {
	return exchangeMode_;
}

/********************************************************************************************/
/**
 * Sets the base name of the data exchange file. Note that the individual might add additional
 * characters in order to distinguish between the exchange files of different individuals.
 *
 * @param parameterFile The desired new base name of the exchange file
 */
void GExternalEvaluatorIndividual::setExchangeFileName(const std::string& parameterFile) {
	if(parameterFile.empty() || parameterFile == "empty") {
		std::ostringstream error;
		error << "In GExternalEvaluatorIndividual::setExchangeFileName(): Error!" << std::endl
				<< "Invalid file name \"" << parameterFile << "\"" << std::endl;
		throw Gem::Common::gemfony_error_condition(error.str());
	}

	parameterFile_ = parameterFile;
}

/********************************************************************************************/
/**
 * Retrieves the current value of the parameterFile_ variable.
 *
 * @return The current base name of the exchange file
 */
std::string GExternalEvaluatorIndividual::getExchangeFileName() const {
	return parameterFile_;
}

/********************************************************************************************/
/**
 * Initiates the printing of the best individual
 */
void GExternalEvaluatorIndividual::printResult(const std::string& identifier) {
	std::string commandLine;

	// Determine the output file name
	std::string bestParameterSetFile = "bestParameterSet";

	// Emit our data
	this->writeParametersToFile(bestParameterSetFile);

	// Check that we have a valid program_ ...
	GExternalEvaluatorIndividual::checkStringIsValid(program_); // Do some error checking

	// Assemble command line and run the external program
	if(exchangeMode_ == Gem::Dataexchange::BINARYEXCHANGE)
		commandLine = program_ + " -m 0  -r -p " + bestParameterSetFile;
	else
		commandLine = program_ + " -m 1  -r -p " + bestParameterSetFile;;
	if(identifier != "empty" && !identifier.empty()) commandLine += (" -g \"" + identifier + "\"");
	if(arguments_ != "empty" && !arguments_.empty()) commandLine +=  (" " + arguments_);

	// Initiate the result calculation
	Gem::Common::runExternalCommand(commandLine);

	// Erase the result file
	bf::path p(bestParameterSetFile.c_str());
#ifdef DEBUG
	if(!bf::exists(p)) {
		std::ostringstream error;
		error << "In GExternalEvaluatorIndividual::printResult(): Error!" << std::endl
				<< "Tried to erase non-existent file " << bestParameterSetFile << std::endl;
		throw(Gem::Common::gemfony_error_condition(error.str()));
	}
#endif /* DEBUG */
	bf::remove(p);
}

/********************************************************************************************/
/**
 * Loads the data of another GExternalEvaluatorIndividual, camouflaged as a GObject
 *
 * @param cp A copy of another GExternalEvaluatorIndividual, camouflaged as a GObject
 */
void GExternalEvaluatorIndividual::load_(const GObject* cp){
	// Convert to a local representation
	const GExternalEvaluatorIndividual *p_load = conversion_cast<GExternalEvaluatorIndividual>(cp);

	// First load the data of our parent class ...
	GParameterSet::load_(cp);

	// ... and then our own
	program_ = p_load->program_;
	arguments_ = p_load->arguments_;
	exchangeMode_ = p_load->exchangeMode_;
	parameterFile_ = p_load->parameterFile_;

	gdbl_ptr_ = p_load->gdbl_ptr_->GObject::clone<GConstrainedDoubleObject>();
	glong_ptr_ = p_load->glong_ptr_->GObject::clone<GConstrainedInt32Object>();
}

/********************************************************************************************/
/**
 * Creates a deep clone of this object
 *
 * @return A deep clone of this object, camouflaged as a GObject
 */
GObject* GExternalEvaluatorIndividual::clone_() const{
	return new GExternalEvaluatorIndividual(*this);
}

/********************************************************************************************/
/**
 * The actual fitness calculation takes place in an external program. Here we just
 * write a file with the required parameters to disk and execute the program.
 *
 * @return The value of this object
 */
double GExternalEvaluatorIndividual::fitnessCalculation() {
	// Check that we have a valid program_ name ...
	GExternalEvaluatorIndividual::checkStringIsValid(program_); // Do some error checking

	// Make the parameters known externally
	std::string parFile = parameterFile_ + "_" + boost::lexical_cast<std::string>(getParentAlgIteration());

	if(this->getPersonality() == EA) parFile +=  ("_" +  boost::lexical_cast<std::string>( this->getEAPersonalityTraits()->getPopulationPosition()));

	// Write out the required data
	this->writeParametersToFile(parFile);

	// Assemble command line and run the external program
	std::string commandLine;

	if(exchangeMode_ == Gem::Dataexchange::BINARYEXCHANGE)
		commandLine = program_ + " -m 0  -p " + parFile;
	else
		commandLine = program_ + " -m 1  -p " + parFile;;


	if(arguments_ != "empty" && !arguments_.empty())
		commandLine += (" " + arguments_);

	Gem::Common::runExternalCommand(commandLine); // It is not clear whether this is thread-safe

	bool hasValue = false;
	double result = this->readParametersFromFile(parFile, hasValue, true);

	// Check whether a result value was received
	if(!hasValue) {
		std::ostringstream error;
		error << "In GExternalEvaluatorIndividual::fitnessCalculation(): Error!" << std::endl
				<< "Received no value from the external calculation" << std::endl;

		throw Gem::Common::gemfony_error_condition(error.str());
	}

	// Erase the parameter file - not needed anymore.
	bf::path p(parFile.c_str());
#ifdef DEBUG
	if(!bf::exists(p)) {
		std::ostringstream error;
		error << "In GExternalEvaluatorIndividual::fitnessCalculation(): Error!" << std::endl
				<< "Tried to erase non-existent parameter file " << parFile << std::endl;
		throw(Gem::Common::gemfony_error_condition(error.str()));
	}
#endif /* DEBUG */
	bf::remove(p);

	// Let the audience know
	return result;
}

/********************************************************************************************/
/**
 * The default constructor. Only needed for serialization purposes
 */
GExternalEvaluatorIndividual::GExternalEvaluatorIndividual()
: program_("unknown")
, arguments_("empty")
, exchangeMode_(Gem::Dataexchange::BINARYEXCHANGE)
, parameterFile_("empty")
, gdbl_ptr_(boost::shared_ptr<GConstrainedDoubleObject>((GConstrainedDoubleObject *)NULL))
, glong_ptr_(boost::shared_ptr<GConstrainedInt32Object>((GConstrainedInt32Object *)NULL))
{ /* nothing */ }

/********************************************************************************************/
/**
 * Writes the class'es data to a file.
 *
 * The structure of this individual is:
 * GConstrainedDoubleObjectCollection
 * GConstrainedInt32ObjectCollection
 * GBooleanCollection
 *
 * @param fileName The name of the file to write to
 */
void GExternalEvaluatorIndividual::writeParametersToFile(const std::string& fileName) {
	// Make sure we are dealing with a clean exchange module
	gde_.resetAll();

	// Retrieve pointers to the four containers and add their data to the GDataExchange module

	// A GConstrainedDoubleObjectCollection can mostly be treated like a std::vector<boost::shared_ptr<GConstrainedDoubleObject> >
	boost::shared_ptr<GConstrainedDoubleObjectCollection> gbdc = at<GConstrainedDoubleObjectCollection>(0);

	GConstrainedDoubleObjectCollection::iterator gbdc_it;
	for(gbdc_it=gbdc->begin(); gbdc_it!=gbdc->end(); ++gbdc_it) {
		boost::shared_ptr<Gem::Dataexchange::GDoubleParameter> dpar(new Gem::Dataexchange::GDoubleParameter((*gbdc_it)->value(), (*gbdc_it)->getLowerBoundary(), (*gbdc_it)->getUpperBoundary()));
		gde_.append(dpar);
	}

	boost::shared_ptr<GConstrainedInt32ObjectCollection> gbic = at<GConstrainedInt32ObjectCollection>(1);
	GConstrainedInt32ObjectCollection::iterator gbic_it;
	for(gbic_it=gbic->begin(); gbic_it!=gbic->end(); ++gbic_it) {
		boost::shared_ptr<Gem::Dataexchange::GLongParameter> ipar(new Gem::Dataexchange::GLongParameter((*gbic_it)->value(), (*gbic_it)->getLowerBoundary(), (*gbic_it)->getUpperBoundary()));
		gde_.append(ipar);
	}

	boost::shared_ptr<GBooleanCollection> gbc = at<GBooleanCollection>(2);
	GBooleanCollection::iterator gbc_it;
	for(gbc_it=gbc->begin(); gbc_it!=gbc->end(); ++gbc_it) {
		boost::shared_ptr<Gem::Dataexchange::GBoolParameter> bpar(new Gem::Dataexchange::GBoolParameter(*gbc_it)); // no boundaries for booleans
		gde_.append(bpar);
	}

	// At this point all necessary data should have been stored in the GDataExchange module. We can now write it to file.
	if(exchangeMode_ == Gem::Dataexchange::BINARYEXCHANGE) gde_.writeToFile(fileName, true);
	else gde_.writeToFile(fileName, false); // TEXTEXCHANGE
}

/********************************************************************************************/
/**
 * Reads the class'es data from a file and loads the best data set into the local
 * structures.
 *
 * @param fileName The name of the file to read from
 * @return The value of the data set in the file (if available), or 0.
 */
double GExternalEvaluatorIndividual::readParametersFromFile(const std::string& fileName, bool& hasValue, const bool& sort) {
	hasValue = false;

	// Make sure gde_ is empty
	gde_.resetAll();

	// Read in the data
	if(exchangeMode_ == Gem::Dataexchange::BINARYEXCHANGE)	gde_.readFromFile(fileName,  true);
	else gde_.readFromFile(fileName, false); // TEXTEXCHANGE

	if(sort) {
		// Switch to the best data set in the collection
		if(this->getMaxMode() == true) gde_.switchToBestDataSet(false); // descending order
		else gde_.switchToBestDataSet(true); // ascending order
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	// Retrieve our "double" collection items
	boost::shared_ptr<GConstrainedDoubleObjectCollection> gbdc = at<GConstrainedDoubleObjectCollection>(0);

	// Get the size of the  "foreign" container ...
	std::size_t exchangeSize = gde_.size<double>();

	// ... and adjust the collection size, as needed . This will erase items
	// or add copies of the second argument, as needed.
	gbdc->resize(exchangeSize, gdbl_ptr_);

	// Now copy the items over
	GConstrainedDoubleObjectCollection::iterator gbdc_it;
	std::size_t pos;
	for(pos=0, gbdc_it=gbdc->begin(); gbdc_it!=gbdc->end(); ++pos, ++gbdc_it) {
		boost::shared_ptr<Gem::Dataexchange::GDoubleParameter> gdp_ptr = gde_.parameterSet_at<double>(pos);
		(*gbdc_it)->resetBoundaries();
		**gbdc_it = gdp_ptr->value();

		if(gdp_ptr->hasBoundaries()) {
			(*gbdc_it)->setBoundaries(gdp_ptr->getLowerBoundary(), gdp_ptr->getUpperBoundary());
		}
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	// Retrieve our "long" collection items
	boost::shared_ptr<GConstrainedInt32ObjectCollection> gbic = at<GConstrainedInt32ObjectCollection>(1);

	// Make sure we have (template-)items in the local collection
	if(gbic->empty()) {
		boost::shared_ptr<GConstrainedInt32Object> gbi_templ(new GConstrainedInt32Object());
		boost::shared_ptr<GInt32FlipAdaptor> gifa_templ(new GInt32FlipAdaptor());
		gbi_templ->addAdaptor(gifa_templ);
		gbic->push_back(gbi_templ);
	}

	// Get the size of the "foreign" container ...
	exchangeSize = gde_.size<boost::int32_t>();

	// ... and adjust the population size, as needed . This will erase items
	// or add copies of the second argument, as needed.
	gbic->resize(exchangeSize, glong_ptr_);

	// Now copy the items over
	GConstrainedInt32ObjectCollection::iterator gbic_it;
	for(pos=0, gbic_it=gbic->begin(); gbic_it!=gbic->end(); ++pos, ++gbic_it) {
		boost::shared_ptr<Gem::Dataexchange::GLongParameter> glp_ptr = gde_.parameterSet_at<boost::int32_t>(pos);
		(*gbic_it)->resetBoundaries();
		**gbic_it = glp_ptr->value();

		if(glp_ptr->hasBoundaries()) {
			(*gbic_it)->setBoundaries(glp_ptr->getLowerBoundary(), glp_ptr->getUpperBoundary());
		}
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	// Retrieve our "bool" collection items
	boost::shared_ptr<GBooleanCollection> gbc = at<GBooleanCollection>(2);

	// Get the size of the "foreign" container ...
	exchangeSize = gde_.size<bool>();

	// ... and adjust the population size, as needed . This will erase items
	// or add copies of the second argument, as needed.
	gbc->resize(exchangeSize, false);

	// Now copy the items over
	GBooleanCollection::iterator gbc_it;
	for(pos=0, gbc_it=gbc->begin(); gbc_it!=gbc->end(); ++pos, ++gbc_it) {
		boost::shared_ptr<Gem::Dataexchange::GBoolParameter> gbp_ptr = gde_.parameterSet_at<bool>(pos);
		*gbc_it = gbp_ptr->value();
	}

	//--------------------------------------------------------------------------------------------------------------------------------------

	// Finally return the value of this data set (if any), or 0.
	if(gde_.hasValue()) {
		hasValue = true;
		return gde_.value();
	}
	return 0.;
}

/********************************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */


// Needed for testing purposes
/*************************************************************************************************/
///////////////////////////////////////////////////////////////////////////////////////////////////
/*************************************************************************************************/

#ifdef GENEVATESTING

/**
 * As the Gem::Geneva::GExternalEvaluatorIndividual has a private default constructor, we need to provide a
 * specialization of the factory function that creates GStartProjectIndividual objects
 */
template <>
boost::shared_ptr<Gem::Geneva::GExternalEvaluatorIndividual> TFactory_GUnitTests<Gem::Geneva::GExternalEvaluatorIndividual>() {
	return boost::shared_ptr<Gem::Geneva::GExternalEvaluatorIndividual>(new Gem::Geneva::GExternalEvaluatorIndividual("../../SampleEvaluator/sampleEvaluator"));
}

#endif /* GENEVATESTING */

/*************************************************************************************************/
///////////////////////////////////////////////////////////////////////////////////////////////////
/*************************************************************************************************/

