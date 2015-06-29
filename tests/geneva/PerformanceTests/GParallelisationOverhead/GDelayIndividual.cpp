/**
 * @file GDelayIndividual.cpp
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

#include "GDelayIndividual.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Tests::GDelayIndividual)

namespace Gem {
namespace Tests {

/******************************************************************************/
/**
 * The default constructor. Intentionally private -- needed only for (de-)serialization.
 */
GDelayIndividual::GDelayIndividual()
	: sleepTime_(boost::posix_time::seconds(1))
{ /* nothing */ }

/******************************************************************************/
/**
 * A standard copy constructor
 *
 * @param cp A copy of another GDelayIndividual
 */
GDelayIndividual::GDelayIndividual(const GDelayIndividual& cp)
	: Gem::Geneva::GParameterSet(cp)
	, sleepTime_(cp.sleepTime_)
{ /* nothing */ }

/******************************************************************************/
/**
 * The standard destructor
 */
GDelayIndividual::~GDelayIndividual()
{ /* nothing */	}

/******************************************************************************/
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
 * @param cp A constant reference to another GDelayIndividual object
 * @return A boolean indicating whether both objects are equal
 */
bool GDelayIndividual::operator==(const GDelayIndividual& cp) const {
	using namespace Gem::Common;
	try {
		this->compare(cp, CE_EQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
		return true;
	} catch(g_expectation_violation&) {
		return false;
	}
}

/*******************************************************************************************/
/**
 * Checks for inequality with another GDelayIndividual object
 *
 * @param cp A constant reference to another GDelayIndividual object
 * @return A boolean indicating whether both objects are inequal
 */
bool GDelayIndividual::operator!=(const GDelayIndividual& cp) const {
	using namespace Gem::Common;
	try {
		this->compare(cp, CE_INEQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
		return true;
	} catch(g_expectation_violation&) {
		return false;
	}
}

/******************************************************************************/
/**
 * Searches for compliance with expectations with respect to another object
 * of the same type
 *
 * @param cp A constant reference to another GObject object
 * @param e The expected outcome of the comparison
 * @param limit The maximum deviation for floating point values (important for similarity checks)
 */
void GDelayIndividual::compare(
	const GObject& cp
	, const Gem::Common::expectation& e
	, const double& limit
) const {
	using namespace Gem::Common;

	// Check that we are dealing with a GDelayIndividual reference independent of this object and convert the pointer
	const GDelayIndividual *p_load = Gem::Common::g_convert_and_compare<GObject, GDelayIndividual>(cp, this);

	Gem::Common::GToken token("GDelayIndividual", e);

	// Compare our parent data ...
	Gem::Common::compare_base<Gem::Geneva::GParameterSet>(IDENTITY(*this, *p_load), token);

	// ... and then the local data
	Gem::Common::compare_t(IDENTITY(sleepTime_, p_load->sleepTime_), token);

	// React on deviations from the expectation
	token.evaluate();
}

/******************************************************************************/
/**
 * Loads the data of another GDelayIndividual, camouflaged as a GObject
 *
 * @param cp A copy of another GDelayIndividual, camouflaged as a GObject
 */
void GDelayIndividual::load_(const Gem::Geneva::GObject* cp){
	// Check that we are dealing with a GDelayIndividual reference independent of this object and convert the pointer
	const GDelayIndividual *p_load = Gem::Common::g_convert_and_compare<GObject, GDelayIndividual>(cp, this);

	// Load our parent class'es data ...
	Gem::Geneva::GParameterSet::load_(cp);

	// ... and then our own.
	sleepTime_ = p_load->sleepTime_;
}

/******************************************************************************/
/**
 * Creates a deep clone of this object
 *
 * @return A deep clone of this object, camouflaged as a GObject
 */
Gem::Geneva::GObject* GDelayIndividual::clone_() const {
	return new GDelayIndividual(*this);
}

/******************************************************************************/
/**
 * The actual adaption operations. We want to avoid spending time on adaptions, as
 * all we want to do is measure the overhead of the parallelization. We thus simply
 * provide an empty replacement for the default behavior and "fake" an adaption.
 */
std::size_t GDelayIndividual::customAdaptions() { return std::size_t(1); }

/******************************************************************************/
/**
 * The actual fitness calculation takes place here.
 *
 * @return The value of this object
 */
double GDelayIndividual::fitnessCalculation() {
	// Sleep for the desired amount of time
	boost::this_thread::sleep(sleepTime_);

	// Return a random value - we do not perform any real optimization
	return gr.uniform_01<double>();
}

/******************************************************************************/
/**
 * Retrieval of the current value of the sleepTime_ variable
 *
 * @return The current value of the sleepTime_ variable
 */
boost::posix_time::time_duration GDelayIndividual::getSleepTime() const {
	return sleepTime_;
}

/******************************************************************************/
/**
 * Sets the sleep-time to a user-defined value
 */
void GDelayIndividual::setSleepTime(const boost::posix_time::time_duration& sleepTime) {
	sleepTime_ = sleepTime;
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * The standard constructor for this class
 */
GDelayIndividualFactory::GDelayIndividualFactory(const std::string& cF)
	: Gem::Common::GFactoryT<Gem::Geneva::GParameterSet>(cF)
	, nVariables_(100)
	, resultFile_("networkResults.C")
	, shortResultFile_("shortDelayResults.txt")
	, nMeasurements_(10)
	, interMeasurementDelay_(1)
{ /* nothing */ }

/******************************************************************************/
/**
 * The destructor
 */
GDelayIndividualFactory::~GDelayIndividualFactory()
{ /* nothing */ }

/******************************************************************************/
/**
 * Allows to retrieve the name of the result file
 *
 * @return The Name of the result file
 */
std::string GDelayIndividualFactory::getResultFileName() const {
	return resultFile_;
}

/******************************************************************************/
/**
 * Allows to retrieve the name of the file holding the short measurement results
 *
 * @return The file name holding short measurement results
 */
std::string GDelayIndividualFactory::getShortResultFileName() const {
	return shortResultFile_;
}

/******************************************************************************/
/**
 * Allows to retrieve the number of delays provided by the user
 *
 * @return The number of delays provided by the user
 */
std::size_t GDelayIndividualFactory::getNDelays() const {
	return sleepTimes_.size();
}

/******************************************************************************/
/**
 * Allows to retrieve the number of measurements to be made for each delay
 *
 * @return The number of measurements to be made for each delay
 */
std::uint32_t GDelayIndividualFactory::getNMeasurements() const {
	return nMeasurements_;
}

/******************************************************************************/
/**
 * Retrieves the amount of seconds main() should wait between two measurements
 */
std::uint32_t GDelayIndividualFactory::getInterMeasurementDelay() const {
	return interMeasurementDelay_;
}

/******************************************************************************/
/**
 * Retrieves the sleep times
 *
 * @return The sleep times, as determined by this object
 */
std::vector<std::tuple<unsigned int, unsigned int>> GDelayIndividualFactory::getSleepTimes() const {
	return sleepTimes_;
}

/******************************************************************************/
/**
 * Creates items of this type
 *
 * @return Items of the desired type
 */
std::shared_ptr<Gem::Geneva::GParameterSet> GDelayIndividualFactory::getObject_(
	Gem::Common::GParserBuilder& gpb
	, const std::size_t& id
) {
	// Will hold the result
	std::shared_ptr<GDelayIndividual> target(new GDelayIndividual());

	// Make the object's local configuration options known
	target->addConfigurationOptions(gpb );

	return target;
}

/******************************************************************************/
/**
 * Allows to describe configuration options of GDelayIndividual objects
 */
void GDelayIndividualFactory::describeLocalOptions_(
	Gem::Common::GParserBuilder& gpb
) {
	// Default values for the delay string
	std::string default_delays = "(0,1), (0,10), (0,100), (0,500), (1,0)";

	gpb.registerFileParameter(
		"nVariables"
		, nVariables_
		, nVariables_
	) << "The number of variables to act on";

	gpb.registerFileParameter(
		"delays"
		, delays_
		, default_delays
	) << "A list of delays through which main() should cycle. Format: seconds:milliseconds";

	gpb.registerFileParameter(
		"resultFile"
		, resultFile_
		, resultFile_
	) << "The name of a file to which results should be stored";

	gpb.registerFileParameter(
		"shortResultFile"
		, shortResultFile_
		, shortResultFile_
	) << "The name of a file to which short results should be stored";

	gpb.registerFileParameter(
		"nMeasurements"
		, nMeasurements_
		, nMeasurements_
	) << "The number of measurements for each delay";

	gpb.registerFileParameter(
		"interMeasurementDelay"
		, interMeasurementDelay_
		, interMeasurementDelay_
	) << "The amount of seconds to wait between two measurements";
}

/******************************************************************************/
/**
 * Allows to act on the configuration options received from the configuration file. Here
 * we can add the options described in describeLocalOptions to the object. In practice,
 * we add the parameter objects here
 *
 * @param p A smart-pointer to be acted on during post-processing
 */
void GDelayIndividualFactory::postProcess_(
	std::shared_ptr<Gem::Geneva::GParameterSet>& p_raw
) {
	// Retrieve information about our id
	std::size_t id = this->getId();

	// Make sure the textual delays are converted to time measurements
	sleepTimes_ = Gem::Common::stringToUIntTupleVec(delays_);

	// Convert the base pointer to the target type
	std::shared_ptr<GDelayIndividual> p
		= Gem::Common::convertSmartPointer<Gem::Geneva::GParameterSet, GDelayIndividual>(p_raw);

	if(Gem::Common::GFACTORYWRITEID==id) {
		// Calculate the current sleep time
		boost::posix_time::time_duration sleepTime = this->tupleToTime(sleepTimes_.at(0));

		std::cout
		<< "Producing individual in write mode with sleep time = " << sleepTime.total_milliseconds() << std::endl;

		p->setSleepTime(sleepTime);

		// Set up a GDoubleObjectCollection
		std::shared_ptr<Gem::Geneva::GDoubleObjectCollection> gbdc_ptr(new Gem::Geneva::GDoubleObjectCollection());

		// Set up nVariables GConstrainedDoubleObject objects in the desired value range,
		// and register them with the collection. The configuration parameters don't matter for this use case
		for(std::size_t var=0; var<nVariables_; var++) {
			std::shared_ptr<Gem::Geneva::GDoubleObject> gbd_ptr(new Gem::Geneva::GDoubleObject(0.5));
			std::shared_ptr<Gem::Geneva::GDoubleGaussAdaptor> gdga_ptr(new Gem::Geneva::GDoubleGaussAdaptor(0.025, 0.1, 0., 1.));
			gdga_ptr->setAdaptionThreshold(1);
			gbd_ptr->addAdaptor(gdga_ptr);

			// Make the GDoubleObject known to the collection
			gbdc_ptr->push_back(gbd_ptr);
		}

		// Make the GDoubleObjectCollection known to the individual
		p->push_back(gbdc_ptr);
	} else if((id-Gem::Common::GFACTTORYFIRSTID) < sleepTimes_.size()) {
		// Calculate the current sleep time
		boost::posix_time::time_duration sleepTime = this->tupleToTime(sleepTimes_.at(id-Gem::Common::GFACTTORYFIRSTID));

		std::cout
		<< "Producing individual " << (id-Gem::Common::GFACTTORYFIRSTID) << " with sleep time = " << sleepTime.total_milliseconds() << std::endl;

		p->setSleepTime(sleepTime);

		// Set up a GDoubleObjectCollection
		std::shared_ptr<Gem::Geneva::GDoubleObjectCollection> gbdc_ptr(new Gem::Geneva::GDoubleObjectCollection());

		// Set up nVariables GConstrainedDoubleObject objects in the desired value range,
		// and register them with the collection. The configuration parameters don't matter for this use case
		for(std::size_t var=0; var<nVariables_; var++) {
			std::shared_ptr<Gem::Geneva::GDoubleObject> gbd_ptr(new Gem::Geneva::GDoubleObject(0.5));
			std::shared_ptr<Gem::Geneva::GDoubleGaussAdaptor> gdga_ptr(new Gem::Geneva::GDoubleGaussAdaptor(0.025, 0.1, 0., 1.));
			gdga_ptr->setAdaptionThreshold(1);
			gbd_ptr->addAdaptor(gdga_ptr);

			// Make the GDoubleObject known to the collection
			gbdc_ptr->push_back(gbd_ptr);
		}

		// Make the GDoubleObjectCollection known to the individual
		p->push_back(gbdc_ptr);
	} else {
		// Return an empty pointer
		p_raw.reset();
	}
}

/******************************************************************************/
/**
 * Converts a tuple to a time format
 *
 * @param timeTuple A tuple of seconds and milliseconds in unsigned int format, to be converted to a time_duration object
 */
boost::posix_time::time_duration GDelayIndividualFactory::tupleToTime(const std::tuple<unsigned int, unsigned int>& timeTuple) {
	boost::posix_time::time_duration t =
		boost::posix_time::seconds(boost::numeric_cast<long>(std::get<0>(timeTuple))) +
		boost::posix_time::milliseconds(boost::numeric_cast<long>(std::get<1>(timeTuple)));

	return t;
}

/******************************************************************************/

} /* namespace Tests */
} /* namespace Gem */
