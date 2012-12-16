/**
 * @file GDelayIndividual.cpp
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

#include "GDelayIndividual.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Tests::GDelayIndividual)

namespace Gem
{
namespace Tests
{

/********************************************************************************************/
/**
 * The default constructor. Intentionally private -- needed only for (de-)serialization.
 */
GDelayIndividual::GDelayIndividual(){ /* nothing */ }

/********************************************************************************************/
/**
 * Initialization with the amount of time the fitness evaluation should sleep before continuing
 *
 * @param sleepTime The amount of time the fitness calculation should pause before continuing
 */
GDelayIndividual::GDelayIndividual(const boost::posix_time::time_duration& sleepTime)
: sleepTime_(sleepTime)
{/* nothing */}

/********************************************************************************************/
/**
 * A standard copy constructor
 *
 * @param cp A copy of another GDelayIndividual
 */
GDelayIndividual::GDelayIndividual(const GDelayIndividual& cp)
: Gem::Geneva::GParameterSet(cp)
, sleepTime_(cp.sleepTime_)
{ /* nothing */ }

/********************************************************************************************/
/**
 * The standard destructor
 */
GDelayIndividual::~GDelayIndividual()
{ /* nothing */	}

/********************************************************************************************/
/**
 * A standard assignment operator
 *
 * @param cp A copy of another GDelayIndividual
 * @return A constant reference to the function argument
 */
const GDelayIndividual& GDelayIndividual::operator=(const GDelayIndividual& cp){
	GDelayIndividual::load_(&cp);
	return *this;
}

/*******************************************************************************************/
/**
 * Checks for equality with another GDelayIndividual object
 *
 * @param  cp A constant reference to another GDelayIndividual object
 * @return A boolean indicating whether both objects are equal
 */
bool GDelayIndividual::operator==(const GDelayIndividual& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GDelayIndividual::operator==","cp", CE_SILENT);
}

/*******************************************************************************************/
/**
 * Checks for inequality with another GDelayIndividual object
 *
 * @param  cp A constant reference to another GDelayIndividual object
 * @return A boolean indicating whether both objects are inequal
 */
bool GDelayIndividual::operator!=(const GDelayIndividual& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GDelayIndividual::operator!=","cp", CE_SILENT);
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
boost::optional<std::string> GDelayIndividual::checkRelationshipWith(const GObject& cp,
		const Gem::Common::expectation& e,
		const double& limit,
		const std::string& caller,
		const std::string& y_name,
		const bool& withMessages) const
{
	using namespace Gem::Common;

	// Check that we are indeed dealing with a GParamterBase reference
	const GDelayIndividual *p_load = Gem::Geneva::GObject::gobject_conversion<GDelayIndividual>(&cp);

	// Will hold possible deviations from the expectation, including explanations
	std::vector<boost::optional<std::string> > deviations;

	// Check our parent class'es data ...
	deviations.push_back(GParameterSet::checkRelationshipWith(cp, e, limit, "GDelayIndividual", y_name, withMessages));

	// ... and then our local data
	deviations.push_back(checkExpectation(withMessages, "GDelayIndividual", sleepTime_, p_load->sleepTime_, "sleepTime_", "p_load->sleepTime_", e , limit));

	return evaluateDiscrepancies("GDelayIndividual", caller, deviations, e);
}

/********************************************************************************************/
/**
 * Loads the data of another GDelayIndividual, camouflaged as a GObject
 *
 * @param cp A copy of another GDelayIndividual, camouflaged as a GObject
 */
void GDelayIndividual::load_(const Gem::Geneva::GObject* cp){
	const GDelayIndividual *p_load = gobject_conversion<GDelayIndividual>(cp);

	// Load our parent class'es data ...
	Gem::Geneva::GParameterSet::load_(cp);

	// ... and then our own.
	sleepTime_ = p_load->sleepTime_;
}

/********************************************************************************************/
/**
 * Creates a deep clone of this object
 *
 * @return A deep clone of this object, camouflaged as a GObject
 */
Gem::Geneva::GObject* GDelayIndividual::clone_() const {
	return new GDelayIndividual(*this);
}

/********************************************************************************************/
/**
 * The actual adaption operations. We want to avoid spending time on adaptions, as
 * all we want to do is measure the overhead of the parallelization. We thus simply
 * provide an empty replacement for the default behavior.
 */
void GDelayIndividual::customAdaptions(){ /* nothing */ }


/********************************************************************************************/
/**
 * The actual fitness calculation takes place here.
 *
 * @return The value of this object
 */
double GDelayIndividual::fitnessCalculation(){
	// Sleep for the desired amount of time
	boost::this_thread::sleep(sleepTime_);

	// Return a random value - we do not perform a real optimization
	return gr.uniform_01<double>();
}

/********************************************************************************************/
/**
 * Manual setting of the sleepTime_ variable
 *
 * @param sleepTime The desired new value of the sleepTime_ variable
 */
void GDelayIndividual::setSleepTime(const boost::posix_time::time_duration& sleepTime) {
	sleepTime_ = sleepTime;
}

/********************************************************************************************/
/**
 * Retrieval of the current value of the sleepTime_ variable
 *
 * @return The current value of the sleepTime_ variable
 */
boost::posix_time::time_duration GDelayIndividual::getSleepTime() const {
	return sleepTime_;
}

/********************************************************************************************/
//////////////////////////////////////////////////////////////////////////////////////////////
/********************************************************************************************/
/**
 * The standard constructor for this class
 */
GDelayIndividualFactory::GDelayIndividualFactory(const std::string& cF)
	: Gem::Geneva::GIndividualFactoryT<GDelayIndividual>(cF)
	, nVariables_(100)
	, resultFile_("networkResults.C")
	, shortResultFile_("shortDelayResults.txt")
	, interMeasurementDelay_(20)
	, nMeasurements_(10)
{ /* nothing */ }

/********************************************************************************************/
/**
 * The destructor
 */
GDelayIndividualFactory::~GDelayIndividualFactory()
{ /* nothing */ }

/********************************************************************************************/
/**
 * Allows to retrieve the name of the result file
 *
 * @return The Name of the result file
 */
std::string GDelayIndividualFactory::getResultFileName() const {
	return resultFile_;
}

/********************************************************************************************/
/**
 * Allows to retrieve the name of the file holding the short measurement results
 *
 * @return The file name holding short measurement results
 */
std::string GDelayIndividualFactory::getShortResultFileName() const {
	return shortResultFile_;
}

/********************************************************************************************/
/**
 * Allows to retrieve the number of delays provided by the user
 *
 * @return The number of delays provided by the user
 */
std::size_t GDelayIndividualFactory::getNDelays() const {
	return sleepSeconds_.size();
}

/********************************************************************************************/
/**
 * Allows to retrieve the amount of seconds in between two measurements
 *
 * @return The amount of seconds the process will sleeb in between two measurements
 */
boost::uint32_t GDelayIndividualFactory::getInterMeasurementDelay() const {
	return interMeasurementDelay_;
}

/********************************************************************************************/
/**
 * Allows to retrieve the number of measurements to be made for each delay
 *
 * @return The number of measurements to be made for each delay
 */
boost::uint32_t GDelayIndividualFactory::getNMeasurements() const {
	return nMeasurements_;
}

/********************************************************************************************/
/**
 * Necessary initialization work. Here we divide the delays_ string into seconds and milliseconds
 */
void GDelayIndividualFactory::init_() {
	// Parse the sleep string and break it into timing values
	std::vector<std::string> sleepTokens;
	typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
	boost::char_separator<char> space_sep(" ");
	tokenizer sleepTokenizer(delays_, space_sep);
	tokenizer::iterator s;
	for(s=sleepTokenizer.begin(); s!=sleepTokenizer.end(); ++s){
		std::cout << "Sleep token is " << *s << std::endl;
		sleepTokens.push_back(*s);
	}
	if(sleepTokens.empty()) { // No sleep tokens were provided
	   glogger
	   << "In GDelayIndividualFactory::init_(): Error!" << std::endl
      << "You did not provide any delay timings" << std::endl
      << GEXCEPTION;
	}

	sleepSeconds_.clear();
	sleepMilliSeconds_.clear();

	std::vector<std::string>::iterator t;
	for(t=sleepTokens.begin(); t!=sleepTokens.end(); ++t) {
		// Split the string
		boost::char_separator<char> slash_sep("/");
		tokenizer delayTokenizer(*t, slash_sep);

		// Loop over the results (there should be exactly two) and assign to the corresponding vectors
		tokenizer::iterator d;
		boost::uint32_t tokenCounter=0;
		for(d=delayTokenizer.begin(); d!=delayTokenizer.end(); ++d){
			switch(tokenCounter++){
			case 0:
				try{
					sleepSeconds_.push_back(boost::lexical_cast<long>(*d));
				}
				catch(...) {
				   glogger
				   << "In GDelayIndividualFactory::init_(): Error (1)!" << std::endl
               << "Could not transfer string " << *d << " to a numeric value" << std::endl
               << GEXCEPTION;
				}
				break;

			case 1:
				try{
					sleepMilliSeconds_.push_back(boost::lexical_cast<long>(*d));
				}
				catch(...) {
				   glogger
				   << "In GDelayIndividualFactory::init_(): Error (2)!" << std::endl
               << "Could not transfer string " << *d << " to a numeric value" << std::endl
               << GEXCEPTION;
				}
				break;

			default:
				{
				   glogger
				   << "In GDelayIndividualFactory::init_(): Error!" << std::endl
               << "tokenCounter has reached invalid value " << tokenCounter << std::endl
               << GEXCEPTION;
				}
				break;
			}
		}
	}
}

/********************************************************************************************/
/**
 * Allows to describe configuration options of GDelayIndividual objects
 */
void GDelayIndividualFactory::describeConfigurationOptions_() {
	// Default values for the delay string
	std::string default_delays = "0/1 0/10 0/100 0/500 1/0 2/0 3/0 4/0 5/0 6/0 7/0 8/0 9/0 10/0 15/0 20/0 25/0 30/0 40/0 50/0 60/0";

	gpb.registerFileParameter("nVariables", nVariables_, nVariables_);
	gpb.registerFileParameter("delays", delays_, default_delays);
	gpb.registerFileParameter("resultFile", resultFile_, resultFile_);
	gpb.registerFileParameter("shortResultFile", shortResultFile_, shortResultFile_);
	gpb.registerFileParameter("interMeasurementDelay", interMeasurementDelay_, interMeasurementDelay_);
	gpb.registerFileParameter("nMeasurements", nMeasurements_, nMeasurements_);
}

/********************************************************************************************/
/**
 * Creates individuals of the desired type. We return valid individuals as often as there
 * are test cases in the delays_ string. Otherwise an empty shared_ptr is returned.
 *
 * @param id An enumerator to the number of calls to this function
 * @return An object of the desired type
 */
boost::shared_ptr<GDelayIndividual> GDelayIndividualFactory::getIndividual_(const std::size_t& id) {
	boost::shared_ptr<GDelayIndividual> result;

	if(id < sleepSeconds_.size()) {
		// Calculate the current sleep time
		boost::posix_time::time_duration sleepTime =
				boost::posix_time::seconds(sleepSeconds_.at(id)) +
				boost::posix_time::milliseconds(sleepMilliSeconds_.at(id));

		std::cout << "Producing an individual with sleep time = " << sleepTime.total_milliseconds() << std::endl;

		boost::shared_ptr<GDelayIndividual> gdi_ptr(new GDelayIndividual(sleepTime));

		// Set up a GDoubleObjectCollection
		boost::shared_ptr<Gem::Geneva::GDoubleObjectCollection> gbdc_ptr(new Gem::Geneva::GDoubleObjectCollection());

		// Set up nVariables GConstrainedDoubleObject objects in the desired value range,
		// and register them with the collection. The configuration parameters don't matter for this use case
		for(std::size_t var=0; var<nVariables_; var++) {
			boost::shared_ptr<Gem::Geneva::GDoubleObject> gbd_ptr(new Gem::Geneva::GDoubleObject(0.5));
			boost::shared_ptr<Gem::Geneva::GDoubleGaussAdaptor> gdga_ptr(new Gem::Geneva::GDoubleGaussAdaptor(0.1, 0.5, 0., 1.));
			gdga_ptr->setAdaptionThreshold(1);
			gbd_ptr->addAdaptor(gdga_ptr);

			// Make the GDoubleObject known to the collection
			gbdc_ptr->push_back(gbd_ptr);
		}

		// Make the GDoubleObjectCollection known to the individual
		gdi_ptr->push_back(gbdc_ptr);

		// Assign to the result item
		result = gdi_ptr;
	}

	return result;
}

/********************************************************************************************/
} /* namespace Tests */
} /* namespace Gem */
