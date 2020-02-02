/**
 * @file
 */

/********************************************************************************
 *
 * This file is part of the Geneva library collection. The following license
 * applies to this file:
 *
 * ------------------------------------------------------------------------------
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * ------------------------------------------------------------------------------
 *
 * Note that other files in the Geneva library collection may use a different
 * license. Please see the licensing information in each file.
 *
 ********************************************************************************
 *
 * Geneva was started by Dr. Rüdiger Berlich and was later maintained together
 * with Dr. Ariel Garcia under the auspices of Gemfony scientific. For further
 * information on Gemfony scientific, see http://www.gemfomy.eu .
 *
 * The majority of files in Geneva was released under the Apache license v2.0
 * in February 2020.
 *
 * See the NOTICE file in the top-level directory of the Geneva library
 * collection for a list of contributors and copyright information.
 *
 ********************************************************************************/

#include "geneva-individuals/GDelayIndividual.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GDelayIndividual)

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * The default constructor. Intentionally private -- needed only for (de-)serialization.
 */
GDelayIndividual::GDelayIndividual()
	: m_fixedSleepTime(1.)
{ /* nothing */ }

/******************************************************************************/
/**
 * A standard copy constructor
 *
 * @param cp A copy of another GDelayIndividual
 */
GDelayIndividual::GDelayIndividual(const GDelayIndividual& cp)
	: Gem::Geneva::GParameterSet(cp)
	, m_fixedSleepTime(cp.m_fixedSleepTime)
	, m_mayCrash(cp.m_mayCrash)
	, m_throwLikelihood(cp.m_throwLikelihood)
	, m_sleepRandomly(cp.m_sleepRandomly)
	, m_randSleepBoundaries(cp.m_randSleepBoundaries)
{ /* nothing */ }

/******************************************************************************/
/**
 * The standard destructor
 */
GDelayIndividual::~GDelayIndividual()
{ /* nothing */	}

/******************************************************************************/
/**
 * Searches for compliance with expectations with respect to another object
 * of the same type
 *
 * @param cp A constant reference to another GObject object
 * @param e The expected outcome of the comparison
 * @param limit The maximum deviation for floating point values (important for similarity checks)
 */
void GDelayIndividual::compare_(
	const GObject& cp
	, const Gem::Common::expectation& e
	, const double& limit
) const {
	using namespace Gem::Common;

	// Check that we are dealing with a GDelayIndividual reference independent of this object and convert the pointer
	const GDelayIndividual *p_load = Gem::Common::g_convert_and_compare<GObject, GDelayIndividual>(cp, this);

	Gem::Common::GToken token("GDelayIndividual", e);

	// Compare our parent data ...
	Gem::Common::compare_base_t<Gem::Geneva::GParameterSet>(*this, *p_load, token);

	// ... and then the local data
	Gem::Common::compare_t(IDENTITY(m_fixedSleepTime, p_load->m_fixedSleepTime), token);
	Gem::Common::compare_t(IDENTITY(m_mayCrash, p_load->m_mayCrash), token);
	Gem::Common::compare_t(IDENTITY(m_throwLikelihood, p_load->m_throwLikelihood), token);
	Gem::Common::compare_t(IDENTITY(m_sleepRandomly, p_load->m_sleepRandomly), token);
	Gem::Common::compare_t(IDENTITY(m_randSleepBoundaries, p_load->m_randSleepBoundaries), token);

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
	m_fixedSleepTime = p_load->m_fixedSleepTime;
	m_mayCrash = p_load->m_mayCrash;
	m_throwLikelihood = p_load->m_throwLikelihood;
	m_sleepRandomly = p_load->m_sleepRandomly;
	m_randSleepBoundaries = p_load->m_randSleepBoundaries;
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
std::size_t GDelayIndividual::customAdaptions()
{ return std::size_t(1); }

/******************************************************************************/
/**
 * The actual fitness calculation takes place here.
 *
 * @return The value of this object
 */
double GDelayIndividual::fitnessCalculation() {
	std::uniform_real_distribution<double> uniform_real_distribution;

	if(m_sleepRandomly) {
	   // Calculate the sleep time
		double sleepTime = uniform_real_distribution(m_gr, std::uniform_real_distribution<double>::param_type(std::get<0>(m_randSleepBoundaries), std::get<1>(m_randSleepBoundaries)));

		std::chrono::duration<double> random_sleepTime(sleepTime);

		// Sleep for a random amount of time in a given time window
		std::this_thread::sleep_for(random_sleepTime);
	} else {
		// Sleep for a fixed amount of time
		std::this_thread::sleep_for(std::chrono::duration<double>(m_fixedSleepTime));
	}

	// Throw if we were asked to do so
	if(m_mayCrash) {
		if(uniform_real_distribution(m_gr, std::uniform_real_distribution<double>::param_type(0.,1.)) < m_throwLikelihood) {
			throw fitnessException();
		}
	}

	// Return a random value - we do not perform any real optimization
	return uniform_real_distribution(m_gr, std::uniform_real_distribution<double>::param_type(0.,1.));
}

/******************************************************************************/
/**
 * Retrieval of the current value of the m_fixedSleepTime variable
 *
 * @return The current value of the m_fixedSleepTime variable
 */
std::chrono::duration<double> GDelayIndividual::getFixedSleepTime() const {
	return std::chrono::duration<double>(m_fixedSleepTime);
}

/******************************************************************************/
/**
 * Sets the sleep-time to a user-defined value
 */
void GDelayIndividual::setFixedSleepTime(const std::chrono::duration<double>& sleepTime) {
	m_fixedSleepTime = sleepTime.count();
}

/******************************************************************************/
/**
 * Indicate that the fitness function may crash at the end of the sleep time,
 * and set the likelihood for such a crash. The likelihood may assume values
 * between (and including) 0 (no crash) and 1 (always crash).
 */
void GDelayIndividual::setMayCrash(
	bool mayCrash
	, double throwLikelihood
) {
	m_mayCrash = mayCrash;

	// Enforce a throwLikelihood in the allowed value range
	m_throwLikelihood = Gem::Common::enforceRangeConstraint(
		throwLikelihood
		, 0.
		, 1.
	   , "GDelayIndividual::setMayCrash()"
	);
}

/******************************************************************************/
/**
 * Check whether the fitness function may crash at the end of the sleep time
 */
bool GDelayIndividual::getMayCrash() const {
	return m_mayCrash;
}

/******************************************************************************/
/**
 * Check the likelihood for a crash at the end of the sleep time
 */
double GDelayIndividual::getCrashLikelihood() const {
	return m_throwLikelihood;
}

/******************************************************************************/
/**
 * Indicates that the fitness function should sleep for a random time. The lower
 * and upper boundaries for the sleep period are passed as a std::tuple, double
 * values indicate seconds (and fractions thereof).
 */
void GDelayIndividual::setRandomSleep(
	bool sleepRandomly
	, std::tuple<double,double> randSleepBoundaries
) {
	m_sleepRandomly = sleepRandomly;

	// Enforce that the sanity of the lower and upper boundaries
	if(std::get<0>(randSleepBoundaries) < 0. || std::get<0>(randSleepBoundaries) >= std::get<1>(randSleepBoundaries)) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GDelayIndividual::setRandomSleep(): Error!" << std::endl
				<< "Got invalid boundaries for the sleep time: " << std::get<0>(randSleepBoundaries) << " / " << std::get<1>(randSleepBoundaries) << std::endl
		);
	}

	m_randSleepBoundaries = randSleepBoundaries;
}

/******************************************************************************/
/**
 * Checks whether the fitness function has a random sleep schedule
 */
bool GDelayIndividual::getMaySleepRandomly() const {
	return m_sleepRandomly;
}

/******************************************************************************/
/**
 * Retrieves the time window for random sleeps
 */
std::tuple<double,double> GDelayIndividual::getSleepWindow() const {
	return m_randSleepBoundaries;
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * The standard constructor for this class
 */
GDelayIndividualFactory::GDelayIndividualFactory(boost::filesystem::path const& cF)
	: Gem::Common::GFactoryT<Gem::Geneva::GParameterSet>(cF)
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
	return m_resultFile;
}

/******************************************************************************/
/**
 * Allows to retrieve the name of the file holding the short measurement results
 *
 * @return The file name holding short measurement results
 */
std::string GDelayIndividualFactory::getShortResultFileName() const {
	return m_shortResultFile;
}

/******************************************************************************/
/**
 * Allows to retrieve the number of delays provided by the user
 *
 * @return The number of delays provided by the user
 */
std::size_t GDelayIndividualFactory::getNDelays() const {
	return m_sleepTimes.size();
}

/******************************************************************************/
/**
 * Allows to retrieve the number of measurements to be made for each delay
 *
 * @return The number of measurements to be made for each delay
 */
std::uint32_t GDelayIndividualFactory::getNMeasurements() const {
	return m_nMeasurements;
}

/******************************************************************************/
/**
 * Retrieves the amount of seconds main() should wait between two measurements
 */
std::uint32_t GDelayIndividualFactory::getInterMeasurementDelay() const {
	return m_interMeasurementDelay;
}

/******************************************************************************/
/**
 * Retrieves the sleep times
 *
 * @return The sleep times, as determined by this object
 */
std::vector<std::tuple<unsigned int, unsigned int>> GDelayIndividualFactory::getSleepTimes() const {
	return m_sleepTimes;
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
	gpb.registerFileParameter(
		"nVariables"
		, m_nVariables
		, m_nVariables // The default value
	) << "The number of variables to act on";

	gpb.registerFileParameter(
		"delays"
		, m_delays
		, m_delays // The default value
	) << "A list of delays through which main() should cycle. Format: seconds:milliseconds";

	gpb.registerFileParameter(
		"sleepRandomly"
		, m_sleepRandomly
		, m_sleepRandomly // The default value
	)
		<< "Indicates whether the individual should sleep for a random amount of time" << std::endl
		<< "rather than a fixed amount of time";

	gpb.registerFileParameter(
		"lowerRandSleepBoundary"
		, m_lowerRandSleepBoundary
		, m_lowerRandSleepBoundary // The default value
	)
		<< "The lower boundary for random sleep times in the" << std::endl
		<< "fitness function (seconds, double value)";

	gpb.registerFileParameter(
		"upperRandSleepBoundary"
		, m_upperRandSleepBoundary
		, m_upperRandSleepBoundary // The default value
	)
		<< "The upper boundary for random sleep times in the" << std::endl
		<< "fitness function (seconds, double value)";

	gpb.registerFileParameter(
		"resultFile"
		, m_resultFile
		, m_resultFile // The default value
	) << "The name of a file to which results should be stored";

	gpb.registerFileParameter(
		"shortResultFile"
		, m_shortResultFile
		, m_shortResultFile // The default value
	) << "The name of a file to which short results should be stored";

	gpb.registerFileParameter(
		"nMeasurements"
		, m_nMeasurements
		, m_nMeasurements // The default value
	) << "The number of measurements for each delay";

	gpb.registerFileParameter(
		"interMeasurementDelay"
		, m_interMeasurementDelay
		, m_interMeasurementDelay // The default value
	) << "The amount of seconds to wait between two measurements";

	gpb.registerFileParameter<bool, double>(
		"mayThrow" // The name of the variable
		, "throwLikelihood"
		, m_mayCrash // The default value
		, m_throwLikelihood
		, [this](bool mayCrash, double throwLikelihood){
			m_mayCrash = mayCrash;
			// Enforce a throwLikelihood in the allowed value range
			m_throwLikelihood = Gem::Common::enforceRangeConstraint(
				throwLikelihood
				, 0.
				, 1.
				, "GDelayIndividual::describeLocalOptions_()"
			);
		}
		, "throwBehaviour"
	)
		<< "Indicates whether the fitness function may throw after the sleep time"
		<< Gem::Common::nextComment()
		<< "Indicates the likelihood that the fitness function throws";
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
	m_sleepTimes = Gem::Common::stringToUIntTupleVec(m_delays);

	// Convert the base pointer to the target type
	std::shared_ptr<GDelayIndividual> p
		= Gem::Common::convertSmartPointer<Gem::Geneva::GParameterSet, GDelayIndividual>(p_raw);

	if(Gem::Common::GFACTORYWRITEID==id) {
		// Calculate the current sleep time
		std::chrono::duration<double> sleepTime = this->tupleToTime(m_sleepTimes.at(0));

		std::cout
		<< "Producing individual in write mode with sleep time = " << sleepTime.count() << " s" << std::endl;

		p->setFixedSleepTime(sleepTime);

		p->setMayCrash(m_mayCrash, m_throwLikelihood);
		p->setRandomSleep(m_sleepRandomly, std::tuple<double,double>(m_lowerRandSleepBoundary, m_upperRandSleepBoundary));

		// Set up a GDoubleObjectCollection
		std::shared_ptr<Gem::Geneva::GDoubleObjectCollection> gbdc_ptr(new Gem::Geneva::GDoubleObjectCollection());

		// Set up nVariables GConstrainedDoubleObject objects in the desired value range,
		// and register them with the collection. The configuration parameters don't matter for this use case
		for(std::size_t var=0; var<m_nVariables; var++) {
			std::shared_ptr<Gem::Geneva::GDoubleObject> gbd_ptr(new Gem::Geneva::GDoubleObject(0.5));
			std::shared_ptr<Gem::Geneva::GDoubleGaussAdaptor> gdga_ptr(new Gem::Geneva::GDoubleGaussAdaptor(0.025, 0.1, 0., 1.));
			gdga_ptr->setAdaptionThreshold(1);
			gbd_ptr->addAdaptor(gdga_ptr);

			// Make the GDoubleObject known to the collection
			gbdc_ptr->push_back(gbd_ptr);
		}

		// Make the GDoubleObjectCollection known to the individual
		p->push_back(gbdc_ptr);
	} else if((id - Gem::Common::GFACTTORYFIRSTID) < m_sleepTimes.size()) {
		// Calculate the current sleep time
		std::chrono::duration<double> sleepTime = this->tupleToTime(m_sleepTimes.at(id - Gem::Common::GFACTTORYFIRSTID));

		std::cout
		<< "Producing individual " << (id-Gem::Common::GFACTTORYFIRSTID) << " with sleep time = " << sleepTime.count() << " s" << std::endl;

		p->setFixedSleepTime(sleepTime);

		p->setMayCrash(m_mayCrash, m_throwLikelihood);
		p->setRandomSleep(m_sleepRandomly, std::tuple<double,double>(m_lowerRandSleepBoundary, m_upperRandSleepBoundary));

		// Set up a GDoubleObjectCollection
		std::shared_ptr<Gem::Geneva::GDoubleObjectCollection> gbdc_ptr(new Gem::Geneva::GDoubleObjectCollection());

		// Set up nVariables GConstrainedDoubleObject objects in the desired value range,
		// and register them with the collection. The configuration parameters don't matter for this use case
		for(std::size_t var=0; var<m_nVariables; var++) {
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
std::chrono::duration<double> GDelayIndividualFactory::tupleToTime(const std::tuple<unsigned int, unsigned int>& timeTuple) {
	std::chrono::duration<double> t =
		std::chrono::seconds(boost::numeric_cast<long>(std::get<0>(timeTuple))) +
		std::chrono::milliseconds(boost::numeric_cast<long>(std::get<1>(timeTuple)));

	return t;
}

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */
