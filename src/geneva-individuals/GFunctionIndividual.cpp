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

#include "geneva-individuals/GFunctionIndividual.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GFunctionIndividual) // NOLINT
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GFunctionIndividualFactory) // NOLINT
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GDoubleSumConstraint) // NOLINT
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GDoubleSumGapConstraint) // NOLINT
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GSphereConstraint) // NOLINT

namespace Gem {
namespace Geneva {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * Initialization with the constant
 */
GDoubleSumConstraint::GDoubleSumConstraint(const double &C)
	: C_(C)
{ /* nothing */ }

/******************************************************************************/
/**
 * Searches for compliance with expectations with respect to another object
 * of the same type
 *
 * @param cp A constant reference to another GObject object
 * @param e The expected outcome of the comparison
 * @param limit The maximum deviation for floating point values (important for similarity checks)
 */
void GDoubleSumConstraint::compare_(
	const GObject &cp, const Gem::Common::expectation &e, const double &limit
) const {
	using namespace Gem::Common;

	// Check that we are dealing with a GDoubleSumConstraint reference independent of this object and convert the pointer
	const GDoubleSumConstraint *p_load = Gem::Common::g_convert_and_compare<GObject, GDoubleSumConstraint>(cp, this);

	Gem::Common::GToken token("GDoubleSumConstraint", e);

	// Compare our parent data ...
	Gem::Common::compare_base_t<GParameterSetConstraint>(*this, *p_load, token);

	// ... and then the local data
	Gem::Common::compare_t(IDENTITY(C_, p_load->C_), token);

	// React on deviations from the expectation
	token.evaluate();
}

/******************************************************************************/
/**
 * Adds local configuration options to a GParserBuilder object
 */
void GDoubleSumConstraint::addConfigurationOptions_(
	Gem::Common::GParserBuilder &gpb
) {
	// Call our parent class'es function
	GParameterSetConstraint::addConfigurationOptions_(gpb);
}

/******************************************************************************/
/**
 * Checks whether a given individual is valid
 */
double GDoubleSumConstraint::check_(
	const GParameterSet *p
) const {
	std::vector<double> parVec;
	p->streamline(parVec);

	double sum = 0.;
	std::vector<double>::iterator it;
	for (it = parVec.begin(); it != parVec.end(); ++it) {
		sum += *it;
	}

	if (sum < C_) {
		return 0.;
	} else {
		return sum / C_;
	}
}

/******************************************************************************/
/**
 * Loads the data of another GDoubleSumConstraint
 */
void GDoubleSumConstraint::load_(const GObject *cp) {
	// Check that we are dealing with a GDoubleSumConstraint reference independent of this object and convert the pointer
	const GDoubleSumConstraint *p_load = Gem::Common::g_convert_and_compare<GObject, GDoubleSumConstraint>(cp, this);

	// Load our parent class'es data ...
	GParameterSetConstraint::load_(cp);

	// ... and then our local data
	C_ = p_load->C_;
}

/******************************************************************************/
/**
 * Creates a deep clone of this object
 */
GObject *GDoubleSumConstraint::clone_() const {
	return new GDoubleSumConstraint(*this);
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * Initialization with the constant
 */
GDoubleSumGapConstraint::GDoubleSumGapConstraint(const double &C, const double &gap)
	: C_(C), gap_(gap) { /* nothing */ }

/******************************************************************************/
/**
 * Searches for compliance with expectations with respect to another object
 * of the same type
 *
 * @param cp A constant reference to another GObject object
 * @param e The expected outcome of the comparison
 * @param limit The maximum deviation for floating point values (important for similarity checks)
 */
void GDoubleSumGapConstraint::compare_(
	const GObject &cp, const Gem::Common::expectation &e, const double &limit
) const {
	using namespace Gem::Common;

	// Check that we are dealing with a GDoubleSumGapConstraint reference independent of this object and convert the pointer
	const GDoubleSumGapConstraint *p_load = Gem::Common::g_convert_and_compare<GObject, GDoubleSumGapConstraint>(cp, this);

	Gem::Common::GToken token("GDoubleSumGapConstraint", e);

	// Compare our parent data ...
	Gem::Common::compare_base_t<GParameterSetConstraint>(*this, *p_load, token);

	// ... and then the local data
	Gem::Common::compare_t(IDENTITY(C_, p_load->C_), token);

	// React on deviations from the expectation
	token.evaluate();
}

/******************************************************************************/
/**
 * Adds local configuration options to a GParserBuilder object
 */
void GDoubleSumGapConstraint::addConfigurationOptions_(
	Gem::Common::GParserBuilder &gpb
) {
	// Call our parent class'es function
	GParameterSetConstraint::addConfigurationOptions_(gpb);
}

/******************************************************************************/
/**
 * Checks whether a given individual is valid
 */
double GDoubleSumGapConstraint::check_(
	const GParameterSet *p
) const {
	std::vector<double> parVec;
	p->streamline(parVec);

	double sum = 0.;
	std::vector<double>::iterator it;
	for (it = parVec.begin(); it != parVec.end(); ++it) {
		sum += *it;
	}

	// Is the sum in the allowed corridor ?
	if (sum >= (C_ - gap_) && sum <= (C_ + gap_)) {
		return 0.;
	} else {
		return 1. + fabs(sum - C_) / C_;
	}
}

/******************************************************************************/
/**
 * Loads the data of another GDoubleSumGapConstraint
 */
void GDoubleSumGapConstraint::load_(const GObject *cp) {
	// Check that we are dealing with a GDoubleSumGapConstraint reference independent of this object and convert the pointer
	const GDoubleSumGapConstraint *p_load = Gem::Common::g_convert_and_compare<GObject, GDoubleSumGapConstraint>(cp, this);

	// Load our parent class'es data ...
	GParameterSetConstraint::load_(cp);

	// ... and then our local data
	C_ = p_load->C_;
	gap_ = p_load->gap_;
}

/******************************************************************************/
/**
 * Creates a deep clone of this object
 */
GObject *GDoubleSumGapConstraint::clone_() const {
	return new GDoubleSumGapConstraint(*this);
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * Initialization with the diameter
 */
GSphereConstraint::GSphereConstraint(const double &diameter)
	: diameter_(diameter)
{ /* nothing */ }

/******************************************************************************/
/**
 * Searches for compliance with expectations with respect to another object
 * of the same type
 *
 * @param cp A constant reference to another GObject object
 * @param e The expected outcome of the comparison
 * @param limit The maximum deviation for floating point values (important for similarity checks)
 */
void GSphereConstraint::compare_(
	const GObject &cp, const Gem::Common::expectation &e, const double &limit
) const {
	// Check that we are dealing with a GSphereConstraint reference independent of this object and convert the pointer
	const GSphereConstraint *p_load = Gem::Common::g_convert_and_compare<GObject, GSphereConstraint>(cp, this);

	Gem::Common::GToken token("GSphereConstraint", e);

	// Compare our parent data ...
	Gem::Common::compare_base_t<GParameterSetConstraint>(*this, *p_load, token);

	// ... and then the local data
	Gem::Common::compare_t(IDENTITY(diameter_, p_load->diameter_), token);

	// React on deviations from the expectation
	token.evaluate();
}

/******************************************************************************/
/**
 * Adds local configuration options to a GParserBuilder object
 */
void GSphereConstraint::addConfigurationOptions_(
	Gem::Common::GParserBuilder &gpb
) {
	// Call our parent class'es function
	GParameterSetConstraint::addConfigurationOptions_(gpb);
}

/******************************************************************************/
/**
 * Checks whether a given individual is valid
 */
double GSphereConstraint::check_(
	const GParameterSet *p
) const {
	std::vector<double> parVec;
	p->streamline(parVec);

	double sum = 0.;
	std::vector<double>::iterator it;
	for (it = parVec.begin(); it != parVec.end(); ++it) {
		sum += GSQUARED(*it);
	}
	sum = sqrt(sum);

	if (sum <= diameter_) {
		return 0.;
	} else {
		return GSQUARED(sum / diameter_);
	}
}

/******************************************************************************/
/**
 * Loads the data of another GSphereConstraint
 */
void GSphereConstraint::load_(const GObject *cp) {
	// Check that we are dealing with a GSphereConstraint reference independent of this object and convert the pointer
	const GSphereConstraint *p_load = Gem::Common::g_convert_and_compare<GObject, GSphereConstraint>(cp, this);

	// Load our parent class'es data ...
	GParameterSetConstraint::load_(cp);

	// ... and then our local data
	diameter_ = p_load->diameter_;
}

/******************************************************************************/
/**
 * Creates a deep clone of this object
 */
GObject *GSphereConstraint::clone_() const {
	return new GSphereConstraint(*this);
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * Puts a Gem::Geneva::solverFunction item into a stream
 *
 * @param o The ostream the item should be added to
 * @param ur the item to be added to the stream
 * @return The std::ostream object used to add the item to
 */
std::ostream &operator<<(std::ostream &o, const Gem::Geneva::solverFunction &ur) {
	auto tmp = static_cast<Gem::Common::ENUMBASETYPE>(ur);
	o << tmp;
	return o;
}

/******************************************************************************/
/**
 * Reads a Gem::Geneva::solverFunction item from a stream
 *
 * @param i The stream the item should be read from
 * @param ur The item read from the stream
 * @return The std::istream object used to read the item from
 */
std::istream &operator>>(std::istream &i, Gem::Geneva::solverFunction &ur) {
	Gem::Common::ENUMBASETYPE tmp;
	i >> tmp;

#ifdef DEBUG
	ur = boost::numeric_cast<Gem::Geneva::solverFunction>(tmp);
#else
	ur = static_cast<Gem::Geneva::solverFunction>(tmp);
#endif /* DEBUG */

	return i;
}

/******************************************************************************/
/**
 * Puts a Gem::Geneva::parameterType item into a stream
 *
 * @param o The ostream the item should be added to
 * @param ur the item to be added to the stream
 * @return The std::ostream object used to add the item to
 */
std::ostream &operator<<(std::ostream &o, const Gem::Geneva::parameterType &ur) {
	auto tmp = static_cast<Gem::Common::ENUMBASETYPE>(ur);
	o << tmp;
	return o;
}

/******************************************************************************/
/**
 * Reads a Gem::Geneva::parameterType item from a stream
 *
 * @param i The stream the item should be read from
 * @param ur The item read from the stream
 * @return The std::istream object used to read the item from
 */
std::istream &operator>>(std::istream &i, Gem::Geneva::parameterType &ur) {
	Gem::Common::ENUMBASETYPE tmp;
	i >> tmp;

#ifdef DEBUG
	ur = boost::numeric_cast<Gem::Geneva::parameterType>(tmp);
#else
	ur = static_cast<Gem::Geneva::parameterType>(tmp);
#endif /* DEBUG */

	return i;
}

/******************************************************************************/
/**
 * Puts a Gem::Geneva::initMode item into a stream
 *
 * @param o The ostream the item should be added to
 * @param ur the item to be added to the stream
 * @return The std::ostream object used to add the item to
 */
std::ostream &operator<<(std::ostream &o, const Gem::Geneva::initMode &ur) {
	auto tmp = static_cast<Gem::Common::ENUMBASETYPE>(ur);
	o << tmp;
	return o;
}

/******************************************************************************/
/**
 * Reads a Gem::Geneva::initMode item from a stream
 *
 * @param i The stream the item should be read from
 * @param ur The item read from the stream
 * @return The std::istream object used to read the item from
 */
std::istream &operator>>(std::istream &i, Gem::Geneva::initMode &ur) {
	Gem::Common::ENUMBASETYPE tmp;
	i >> tmp;

#ifdef DEBUG
	ur = boost::numeric_cast<Gem::Geneva::initMode>(tmp);
#else
	ur = static_cast<Gem::Geneva::initMode>(tmp);
#endif /* DEBUG */

	return i;
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * Initialization with the desired demo function
 *
 * @param dF The id of the demo function
 */
GFunctionIndividual::GFunctionIndividual(const solverFunction &dF)
	: demoFunction_(dF)
{ /* nothing */ }

/******************************************************************************/
/**
 * Searches for compliance with expectations with respect to another object
 * of the same type
 *
 * @param cp A constant reference to another GObject object
 * @param e The expected outcome of the comparison
 * @param limit The maximum deviation for floating point values (important for similarity checks)
 */
void GFunctionIndividual::compare_(
	const GObject &cp, const Gem::Common::expectation &e, const double &limit
) const {
	// Check that we are dealing with a GFunctionIndividual reference independent of this object and convert the pointer
	const GFunctionIndividual *p_load = Gem::Common::g_convert_and_compare<GObject, GFunctionIndividual>(cp, this);

	Gem::Common::GToken token("GFunctionIndividual", e);

	// Compare our parent data ...
	Gem::Common::compare_base_t<GParameterSet>(*this, *p_load, token);

	// ... and then the local data
	Gem::Common::compare_t(IDENTITY(demoFunction_, p_load->demoFunction_), token);

	// React on deviations from the expectation
	token.evaluate();
}

/******************************************************************************/
/**
 * Adds local configuration options to a GParserBuilder object
 *
 * @param gpb The GParserBuilder object to which configuration options should be added
 */
void GFunctionIndividual::addConfigurationOptions_(
	Gem::Common::GParserBuilder &gpb
) {
	// Call our parent class'es function
	GParameterSet::addConfigurationOptions_(gpb);

	// Local data
	gpb.registerFileParameter<solverFunction>(
		"demoFunction" // The name of the variable
		, GO_DEF_EVALFUNCTION // The default value
		, [this](solverFunction sf) { this->setDemoFunction(sf); }
	)
		<< "Specifies which demo function should be used:" << std::endl
		<< "0: Parabola" << std::endl
		<< "1: Berlich" << std::endl
		<< "2: Rosenbrock" << std::endl
		<< "3: Ackley" << std::endl
		<< "4: Rastrigin" << std::endl
		<< "5: Schwefel" << std::endl
		<< "6: Salomon" << std::endl
		<< "7: Negative Parabola" << std::endl;
}

/******************************************************************************/
/**
 * Allows to set the demo function
 *
 * @param dF The id if the demo function
 */
void GFunctionIndividual::setDemoFunction(solverFunction dF) {
	demoFunction_ = dF;
}

/******************************************************************************/
/**
 * Allows to retrieve the demo function
 *
 * @return The id of the currently selected demo function
 */
solverFunction GFunctionIndividual::getDemoFunction() const {
	return demoFunction_;
}

/******************************************************************************/
/**
 * Allows to cross check the parameter size
 *
 * @return The number of doubles stored in this object
 */
std::size_t GFunctionIndividual::getParameterSize() const {
	// Retrieve the parameters
	std::vector<double> parVec;
	this->streamline(parVec);
	return parVec.size();
}

/******************************************************************************/
/**
 * Loads the data of another GFunctionIndividual, camouflaged as a GObject
 *
 * @param cp A copy of another GFunctionIndividual, camouflaged as a GObject
 */
void GFunctionIndividual::load_(const GObject *cp) {
	// Check that we are dealing with a GFunctionIndividual reference independent of this object and convert the pointer
	const GFunctionIndividual *p_load = Gem::Common::g_convert_and_compare<GObject, GFunctionIndividual>(cp, this);

	// Load our parent class'es data ...
	GParameterSet::load_(cp);

	// ... and then our local data
	demoFunction_ = p_load->demoFunction_;
}

/******************************************************************************/
/**
 * Creates a deep clone of this object
 *
 * @return A deep clone of this object, camouflaged as a GObject
 */
GObject *GFunctionIndividual::clone_() const {
	return new GFunctionIndividual(*this);
}

/******************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GFunctionIndividual::modify_GUnitTests_() {
#ifdef GEM_TESTING
	using boost::unit_test_framework::test_suite;
	using boost::unit_test_framework::test_case;

	bool result = false;

	// Call the parent classes' functions
	if (Gem::Geneva::GParameterSet::modify_GUnitTests_()) result = true;

	// Change the parameter settings
	result = true;

	return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
	Gem::Common::condnotset("GFunctionIndividual::modify_GUnitTests", "GEM_TESTING");
return false;
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GFunctionIndividual::specificTestsNoFailureExpected_GUnitTests_() {
#ifdef GEM_TESTING
	using namespace Gem::Geneva;

	using boost::unit_test_framework::test_suite;
	using boost::unit_test_framework::test_case;

	// Call the parent classes' functions
	Gem::Geneva::GParameterSet::specificTestsNoFailureExpected_GUnitTests_();

	//------------------------------------------------------------------------------
	//------------------------------------------------------------------------------

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
	Gem::Common::condnotset("GFunctionIndividual::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GFunctionIndividual::specificTestsFailuresExpected_GUnitTests_() {
#ifdef GEM_TESTING
	using namespace Gem::Geneva;

	using boost::unit_test_framework::test_suite;
	using boost::unit_test_framework::test_case;

	// Call the parent classes' functions
	Gem::Geneva::GParameterSet::specificTestsFailuresExpected_GUnitTests_();

	//------------------------------------------------------------------------------
	//------------------------------------------------------------------------------

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
	Gem::Common::condnotset("GFunctionIndividual::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * The actual value calculation takes place here
 *
 * @param The id of the target function (ignored here)
 * @return The value of this object, as calculated with the evaluation function
 */
double GFunctionIndividual::fitnessCalculation() {
	double result = 0;

	// Retrieve the parameters
	std::vector<double> parVec;
	this->streamline(parVec);
	std::size_t parameterSize = parVec.size();

	// Perform the actual calculation
	switch (demoFunction_) {
		//-----------------------------------------------------------
		// A simple, multi-dimensional parabola
		case solverFunction::PARABOLA: {
			for (std::size_t i = 0; i < parameterSize; i++) {
				result += GSQUARED(parVec[i]);
			}
		}
			break;

			//-----------------------------------------------------------
			// A "noisy" parabola, i.e. a parabola with a very large number of overlaid local optima
		case solverFunction::NOISYPARABOLA: {
			double xsquared = 0.;
			for (std::size_t i = 0; i < parameterSize; i++) {
				xsquared += GSQUARED(parVec[i]);
			}
			result = (cos(xsquared) + 2.) * xsquared;
		}
			break;

			//-----------------------------------------------------------
			// The generalized Rosenbrock function (see e.g. http://en.wikipedia.org/wiki/Rosenbrock_function)
			// or http://www.it.lut.fi/ip/evo/functions/node5.html .
		case solverFunction::ROSENBROCK: {
#ifdef DEBUG
			// Check the size of the parameter vector -- must be at least 2
			if(parameterSize < 2) {
				throw gemfony_exception(
					g_error_streamer(DO_LOG,  time_and_place)
						<< "In GFunctionIndividual::fitnessCalculation() / ROSENBROCK: Error!" << std::endl
						<< "Need to use at least two input dimensions, but got " << parameterSize << std::endl
				);
			}
#endif /* DEBUG */

			for (std::size_t i = 0; i < (parameterSize - 1); i++) {
				result += 100. * GSQUARED(GSQUARED(parVec[i]) - parVec[i + 1]) + GSQUARED(1. - parVec[i]);
			}
		}
			break;

			//-----------------------------------------------------------
			// The Ackeley function (see e.g. http://www.it.lut.fi/ip/evo/functions/node14.html)
		case solverFunction::ACKLEY: {
#ifdef DEBUG
			// Check the size of the parameter vector -- must be at least 2
			if(parameterSize < 2) {
				throw gemfony_exception(
					g_error_streamer(DO_LOG,  time_and_place)
						<< "In GFunctionIndividual::fitnessCalculation() / ACKLEY: Error!" << std::endl
						<< "Need to use at least two input dimensions, but got " << parameterSize << std::endl
				);
			}
#endif /* DEBUG */

			for (std::size_t i = 0; i < (parameterSize - 1); i++) {
				result += (exp(-0.2) * sqrt(GSQUARED(parVec[i]) + GSQUARED(parVec[i + 1])) +
							  3. * (cos(2. * parVec[i]) + sin(2. * parVec[i + 1])));
			}
		}
			break;

			//-----------------------------------------------------------
			// The Rastrigin function (see e.g. http://www.it.lut.fi/ip/evo/functions/node6.html)
		case solverFunction::RASTRIGIN: {
			result = 10 * double(parameterSize);

			for (std::size_t i = 0; i < parameterSize; i++) {
				result += (GSQUARED(parVec[i]) - 10. * cos(2 * boost::math::constants::pi<double>() * parVec[i]));
			}
		}
			break;

			//-----------------------------------------------------------
			// The Schwefel function (see e.g. http://www.it.lut.fi/ip/evo/functions/node10.html)
		case solverFunction::SCHWEFEL: {
			for (std::size_t i = 0; i < parameterSize; i++) {
				result += -parVec[i] * sin(sqrt(fabs(parVec[i])));
			}

			result /= parameterSize;
		}
			break;

			//-----------------------------------------------------------
			// The Salomon function (see e.g. http://www.it.lut.fi/ip/evo/functions/node12.html)
		case solverFunction::SALOMON: {
			double sum_root = 0.;
			for (std::size_t i = 0; i < parameterSize; i++) {
				sum_root += GSQUARED(parVec[i]);
			}
			sum_root = sqrt(sum_root);

			result = -cos(2 * boost::math::constants::pi<double>() * sum_root) + 0.1 * sum_root + 1;
		}
			break;

			//-----------------------------------------------------------
			// A "negative" parabola, used for maximization tests
		case solverFunction::NEGPARABOLA: {
			for (std::size_t i = 0; i < parameterSize; i++) {
				result += GSQUARED(parVec[i]);
			}
			result *= -1.;
		}
			break;

			//-----------------------------------------------------------
	};

	// Let the audience know
	return result;
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * Provide an easy way to print the individual's content
 */
std::ostream &operator<<(std::ostream &s, const Gem::Geneva::GFunctionIndividual &f) {
	std::vector<double> parVec;
	f.streamline(parVec);

	std::cout << std::endl << "Raw fitness: " << f.raw_fitness(0) << std::endl << std::endl;
	std::size_t pos = 0;
	std::cout << "Parameter values of best individual:" << std::endl;
	for(const auto& val: parVec) {
		std::cout << pos++ << ": " << val << std::endl;
	}
	std::cout << std::endl;

	return s;
}

/******************************************************************************/
/**
 * Provide an easy way to print the individual's content through a smart-pointer
 */
std::ostream &operator<<(std::ostream &s, std::shared_ptr <Gem::Geneva::GFunctionIndividual> f_ptr) {
	return operator<<(s, *f_ptr);
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A constructor with the ability to switch the parallelization mode. It initializes a
 * target item as needed.
 *
 * @param configFile The name of the configuration file
 */
GFunctionIndividualFactory::GFunctionIndividualFactory(std::filesystem::path const& configFile)
	: GParameterSetFactory(configFile)
{ /* nothing */ }

/******************************************************************************/
/**
 * The default constructor. Only needed for (de-)serialization purposes, hence empty.
 */
GFunctionIndividualFactory::GFunctionIndividualFactory()
	: GParameterSetFactory("empty")
{ /* nothing */ }

/******************************************************************************/
/**
 * Loads the data of another GFunctionIndividualFactory object
 */
void GFunctionIndividualFactory::load(std::shared_ptr <Gem::Common::GFactoryT<GParameterSet>> cp_raw_ptr) {
	// Load our parent class'es data
	GParameterSetFactory::load(cp_raw_ptr);

	// Convert the base pointer
	std::shared_ptr <GFunctionIndividualFactory> cp_ptr
		= Gem::Common::convertSmartPointer<Gem::Common::GFactoryT<GParameterSet>, GFunctionIndividualFactory>(cp_raw_ptr);

	// And then our own
	adProb_ = cp_ptr->adProb_;
	adaptAdProb_ = cp_ptr->adaptAdProb_;
	minAdProb_ = cp_ptr->minAdProb_;
	maxAdProb_ = cp_ptr->maxAdProb_;
	adaptionThreshold_ = cp_ptr->adaptionThreshold_;
	useBiGaussian_ = cp_ptr->useBiGaussian_;
	sigma1_ = cp_ptr->sigma1_;
	sigmaSigma1_ = cp_ptr->sigmaSigma1_;
	minSigma1_ = cp_ptr->minSigma1_;
	maxSigma1_ = cp_ptr->maxSigma1_;
	sigma2_ = cp_ptr->sigma2_;
	sigmaSigma2_ = cp_ptr->sigmaSigma2_;
	minSigma2_ = cp_ptr->minSigma2_;
	maxSigma2_ = cp_ptr->maxSigma2_;
	delta_ = cp_ptr->delta_;
	sigmaDelta_ = cp_ptr->sigmaDelta_;
	minDelta_ = cp_ptr->minDelta_;
	maxDelta_ = cp_ptr->maxDelta_;
	parDim_ = cp_ptr->parDim_;
	minVar_ = cp_ptr->minVar_;
	maxVar_ = cp_ptr->maxVar_;
	pT_ = cp_ptr->pT_;
	iM_ = cp_ptr->iM_;
}

/******************************************************************************/
/**
 * Creates a deep clone of this object
 */
std::shared_ptr <Gem::Common::GFactoryT<GParameterSet>> GFunctionIndividualFactory::clone() const {
	return std::shared_ptr<GFunctionIndividualFactory>(new GFunctionIndividualFactory(*this));
}

/******************************************************************************/
/**
 * (Re-)Set the dimension of the function
 */
void GFunctionIndividualFactory::setParDim(std::size_t parDim) {
	if (parDim == 0) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GFunctionIndividualFactory::setParDim(): Error!" << std::endl
				<< "Dimension of the function is set to 0" << std::endl
		);
	}

	parDim_ = parDim;
}

/******************************************************************************/
/**
 * Extract the minimum and maximum boundaries of the variables
 */
std::tuple<double, double> GFunctionIndividualFactory::getVarBoundaries() const {
	return std::tuple<double, double>{minVar_, maxVar_};
}

/******************************************************************************/
/**
 * Set the minimum and maximum boundaries of the variables
 */
void GFunctionIndividualFactory::setVarBoundaries(std::tuple<double, double> boundaries) {
	double min = std::get<0>(boundaries);
	double max = std::get<1>(boundaries);

	if (min >= max) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GFunctionIndividualFactory::setVarBoundaries(): Error!" << std::endl
				<< "Received invalid boundaries " << min << " / " << max << std::endl
		);
	}

	setMinVar(min);
	setMaxVar(max);
}

/******************************************************************************/
/**
 * Get the value of the adaptionThreshold_ variable
 */
std::uint32_t GFunctionIndividualFactory::getAdaptionThreshold() const {
	return adaptionThreshold_;
}

/******************************************************************************/
/**
 * Set the value of the adaptionThreshold_ variable
 */
void GFunctionIndividualFactory::setAdaptionThreshold(
	std::uint32_t adaptionThreshold
) {
	adaptionThreshold_ = adaptionThreshold;
}

/******************************************************************************/
/**
 * Allows to retrieve the adProb_ variable
 */
double GFunctionIndividualFactory::getAdProb() const {
	return adProb_;
}

/******************************************************************************/
/**
 * Set the value of the adProb_ variable
 */
void GFunctionIndividualFactory::setAdProb(double adProb) {
	adProb_ = adProb;
}

/******************************************************************************/
/**
 * Allows to retrieve the delta_ variable
 */
double GFunctionIndividualFactory::getDelta() const {
	return delta_;
}

/******************************************************************************/
/**
 * Set the value of the delta_ variable
 */
void GFunctionIndividualFactory::setDelta(double delta) {
	delta_ = delta;
}

/******************************************************************************/
/**
 * Allows to retrieve the iM_ variable
 */
initMode GFunctionIndividualFactory::getIM() const {
	return iM_;
}

/******************************************************************************/
/**
 * Set the value of the iM_ variable
 */
void GFunctionIndividualFactory::setIM(initMode m) {
	iM_ = m;
}

/******************************************************************************/
/**
 * Allows to retrieve the maxDelta_ variable
 */
double GFunctionIndividualFactory::getMaxDelta() const {
	return maxDelta_;
}

/******************************************************************************/
/**
 * Set the value of the maxDelta_ variable
 */
void GFunctionIndividualFactory::setMaxDelta(double maxDelta) {
	maxDelta_ = maxDelta;
}

/******************************************************************************/
/**
 * Allows to retrieve the maxSigma1_ variable
 */
double GFunctionIndividualFactory::getMaxSigma1() const {
	return maxSigma1_;
}

/******************************************************************************/
/**
 * Set the value of the maxSigma1_ variable
 */
void GFunctionIndividualFactory::setMaxSigma1(double maxSigma1) {
	maxSigma1_ = maxSigma1;
}

/******************************************************************************/
/**
 * Allows to retrieve the maxSigma2_ variable
 */
double GFunctionIndividualFactory::getMaxSigma2() const {
	return maxSigma2_;
}

/******************************************************************************/
/**
 * Set the value of the maxSigma2_ variable
 */
void GFunctionIndividualFactory::setMaxSigma2(double maxSigma2) {
	maxSigma2_ = maxSigma2;
}

/******************************************************************************/
/**
 * Allows to retrieve the maxVar_ variable
 */
double GFunctionIndividualFactory::getMaxVar() const {
	return maxVar_;
}

/******************************************************************************/
/**
 * Set the value of the maxVar_ variable
 */
void GFunctionIndividualFactory::setMaxVar(double maxVar) {
	maxVar_ = maxVar;
}

/******************************************************************************/
/**
 * Allows to retrieve the minDelta_ variable
 */
double GFunctionIndividualFactory::getMinDelta() const {
	return minDelta_;
}

/******************************************************************************/
/**
 * Set the value of the minDelta_ variable
 */
void GFunctionIndividualFactory::setMinDelta(double minDelta) {
	minDelta_ = minDelta;
}

/******************************************************************************/
/**
 * Allows to retrieve the allowed value range of delta
 */
std::tuple<double, double> GFunctionIndividualFactory::getDeltaRange() const {
	return std::tuple<double, double>{minDelta_, maxDelta_};
}

/******************************************************************************/
/**
 * Allows to set the allowed value range of delta
 */
void GFunctionIndividualFactory::setDeltaRange(std::tuple<double, double> range) {
	double min = std::get<0>(range);
	double max = std::get<1>(range);

	if (min < 0) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GFunctionIndividualFactory::setDeltaRange(): Error" << std::endl
				<< "min must be >= 0. Got : " << max << std::endl
		);
	}

	if (min >= max) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GFunctionIndividualFactory::setDeltaRange(): Error" << std::endl
				<< "Invalid range specified: " << min << " / " << max << std::endl
		);
	}

	minDelta_ = min;
	maxDelta_ = max;
}

/******************************************************************************/
/**
 * Allows to retrieve the minSigma1_ variable
 */
double GFunctionIndividualFactory::getMinSigma1() const {
	return minSigma1_;
}

/******************************************************************************/
/**
 * Set the value of the minSigma1_ variable
 */
void GFunctionIndividualFactory::setMinSigma1(double minSigma1) {
	minSigma1_ = minSigma1;
}

/******************************************************************************/
/**
 * Allows to retrieve the allowed value range of sigma1_
 */
std::tuple<double, double> GFunctionIndividualFactory::getSigma1Range() const {
	return std::tuple<double, double> {minSigma1_, maxSigma1_};
}

/******************************************************************************/
/**
 * Allows to set the allowed value range of sigma1_
 */
void GFunctionIndividualFactory::setSigma1Range(std::tuple<double, double> range) {
	double min = std::get<0>(range);
	double max = std::get<1>(range);

	if (min < 0) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GFunctionIndividualFactory::setSigma1Range(): Error" << std::endl
				<< "min must be >= 0. Got : " << max << std::endl
		);
	}

	if (min >= max) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GFunctionIndividualFactory::setSigma1Range(): Error" << std::endl
				<< "Invalid range specified: " << min << " / " << max << std::endl
		);
	}

	minSigma1_ = min;
	maxSigma1_ = max;
}

/******************************************************************************/
/**
 * Allows to retrieve the minSigma2_ variable
 */
double GFunctionIndividualFactory::getMinSigma2() const {
	return minSigma2_;
}

/******************************************************************************/
/**
 * Set the value of the minSigma2_ variable
 */
void GFunctionIndividualFactory::setMinSigma2(double minSigma2) {
	minSigma2_ = minSigma2;
}

/******************************************************************************/
/**
 * Allows to retrieve the allowed value range of sigma2_
 */
std::tuple<double, double> GFunctionIndividualFactory::getSigma2Range() const {
	return std::tuple<double, double>{minSigma2_, maxSigma2_};
}

/******************************************************************************/
/**
 * Allows to set the allowed value range of sigma2_
 */
void GFunctionIndividualFactory::setSigma2Range(std::tuple<double, double> range) {
	double min = std::get<0>(range);
	double max = std::get<1>(range);

	if (min < 0) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GFunctionIndividualFactory::setSigma2Range(): Error" << std::endl
				<< "min must be >= 0. Got : " << max << std::endl
		);
	}

	if (min >= max) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GFunctionIndividualFactory::setSigma2Range(): Error" << std::endl
				<< "Invalid range specified: " << min << " / " << max << std::endl
		);
	}

	minSigma2_ = min;
	maxSigma2_ = max;
}

/******************************************************************************/
/**
 * Allows to retrieve the minVar_ variable
 */
double GFunctionIndividualFactory::getMinVar() const {
	return minVar_;
}

/******************************************************************************/
/**
 * Set the value of the minVar_ variable
 */
void GFunctionIndividualFactory::setMinVar(double minVar) {
	minVar_ = minVar;
}

/******************************************************************************/
/**
 * Allows to retrieve the parDim_ variable
 */
std::size_t GFunctionIndividualFactory::getParDim() const {
	return parDim_;
}

/******************************************************************************/
/**
 * Allows to retrieve the pT_ variable
 */
parameterType GFunctionIndividualFactory::getPT() const {
	return pT_;
}

/******************************************************************************/
/**
 * Set the value of the pT_ variable
 */
void GFunctionIndividualFactory::setPT(parameterType pt) {
	pT_ = pt;
}

/******************************************************************************/
/**
 * Allows to retrieve the sigma1_ variable
 */
double GFunctionIndividualFactory::getSigma1() const {
	return sigma1_;
}

/******************************************************************************/
/**
 * Set the value of the sigma1_ variable
 */
void GFunctionIndividualFactory::setSigma1(double sigma1) {
	sigma1_ = sigma1;
}

/******************************************************************************/
/**
 * Allows to retrieve the sigma2_ variable
 */
double GFunctionIndividualFactory::getSigma2() const {
	return sigma2_;
}

/******************************************************************************/
/**
 * Set the value of the sigma2_ variable
 */
void GFunctionIndividualFactory::setSigma2(double sigma2) {
	sigma2_ = sigma2;
}

/******************************************************************************/
/**
 * Allows to retrieve the sigmaDelta_ variable
 */
double GFunctionIndividualFactory::getSigmaDelta() const {
	return sigmaDelta_;
}

/******************************************************************************/
/**
 * Set the value of the sigmaDelta_ variable
 */
void GFunctionIndividualFactory::setSigmaDelta(double sigmaDelta) {
	sigmaDelta_ = sigmaDelta;
}

/******************************************************************************/
/**
 * Allows to retrieve the sigmaSigma1_ variable
 */
double GFunctionIndividualFactory::getSigmaSigma1() const {
	return sigmaSigma1_;
}

/******************************************************************************/
/**
 * Set the value of the sigmaSigma1_ variable
 */
void GFunctionIndividualFactory::setSigmaSigma1(double sigmaSigma1) {
	sigmaSigma1_ = sigmaSigma1;
}

/******************************************************************************/
/**
 * Allows to retrieve the sigmaSigma2_ variable
 */
double GFunctionIndividualFactory::getSigmaSigma2() const {
	return sigmaSigma2_;
}

/******************************************************************************/
/**
 * Set the value of the sigmaSigma2_ variable
 */
void GFunctionIndividualFactory::setSigmaSigma2(double sigmaSigma2) {
	sigmaSigma2_ = sigmaSigma2;
}

/******************************************************************************/
/**
 * Allows to retrieve the useBiGaussian_ variable
 */
bool GFunctionIndividualFactory::getUseBiGaussian() const {
	return useBiGaussian_;
}

/******************************************************************************/
/**
 * Set the value of the useBiGaussian_ variable
 */
void GFunctionIndividualFactory::setUseBiGaussian(bool useBiGaussian) {
	useBiGaussian_ = useBiGaussian;
}

/******************************************************************************/
/**
 * Allows to retrieve the rate of evolutionary adaption of adProb_
 */
double GFunctionIndividualFactory::getAdaptAdProb() const {
	return adaptAdProb_;
}

/******************************************************************************/
/**
 * Allows to specify an adaption factor for adProb_ (or 0, if you do not want this feature)
 */
void GFunctionIndividualFactory::setAdaptAdProb(double adaptAdProb) {
#ifdef DEBUG
	if(adaptAdProb < 0.) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GFunctionIndividualFactory::setAdaptAdProb(): Error!" << std::endl
				<< "Invalid value for adaptAdProb given: " << adaptAdProb << std::endl
		);
	}
#endif /* DEBUG */

	adaptAdProb_ = adaptAdProb;
}

/******************************************************************************/
/**
 * Allows to retrieve the allowed range for adProb_ variation
 */
std::tuple<double, double> GFunctionIndividualFactory::getAdProbRange() const {
	return std::tuple<double, double>{minAdProb_.value(), maxAdProb_.value()};
}

/******************************************************************************/
/**
 * Allows to set the allowed range for adaption probability variation
 */
void GFunctionIndividualFactory::setAdProbRange(double minAdProb, double maxAdProb) {
#ifdef DEBUG
	if(minAdProb < 0.) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GFunctionIndividualFactory::setAdProbRange(): Error!" << std::endl
				<< "minAdProb < 0: " << minAdProb << std::endl
		);
	}

	if(minAdProb > maxAdProb) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GFunctionIndividualFactory::setAdProbRange(): Error!" << std::endl
				<< "Invalid minAdProb and/or maxAdProb: " << minAdProb << " / " << maxAdProb << std::endl
		);
	}

	if(maxAdProb > 1.) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GFunctionIndividualFactory::setAdProbRange(): Error!" << std::endl
				<< "maxAdProb > 1: " << maxAdProb << std::endl
		);
	}
#endif /* DEBUG */

	minAdProb_ = minAdProb;
	maxAdProb_ = maxAdProb;
}

/******************************************************************************/
/**
 * Creates items of this type
 *
 * @return Items of the desired type
 */
std::shared_ptr <GParameterSet> GFunctionIndividualFactory::getObject_(
	Gem::Common::GParserBuilder &gpb, const std::size_t &id
) {
	// Will hold the result
	std::shared_ptr <GFunctionIndividual> target(new GFunctionIndividual());

	// Make the object's local configuration options known
	target->addConfigurationOptions(gpb);

	return target;
}

/******************************************************************************/
/**
 * Allows to describe local configuration options for gradient descents
 */
void GFunctionIndividualFactory::describeLocalOptions_(Gem::Common::GParserBuilder &gpb) {
	// Describe our own options
	using namespace Gem::Courtier;

	std::string comment;

	comment = "";
	comment += "The probability for random adaption of values in evolutionary algorithms;";
	gpb.registerFileParameter<double>(
		"adProb", adProb_.reference(), GFI_DEF_ADPROB, Gem::Common::VAR_IS_ESSENTIAL, comment
	);

	comment = "";
	comment += "Determines the rate of adaption of adProb. Set to 0, if you do not need this feature;";
	gpb.registerFileParameter<double>(
		"adaptAdProb", adaptAdProb_.reference(), GFI_DEF_ADAPTADPROB, Gem::Common::VAR_IS_ESSENTIAL, comment
	);

	comment = "";
	comment += "The lower allowed boundary for adProb-variation;";
	gpb.registerFileParameter<double>(
		"minAdProb", minAdProb_.reference(), GFI_DEF_MINADPROB, Gem::Common::VAR_IS_ESSENTIAL, comment
	);

	comment = "";
	comment += "The upper allowed boundary for adProb-variation;";
	gpb.registerFileParameter<double>(
		"maxAdProb", maxAdProb_.reference(), GFI_DEF_MAXADPROB, Gem::Common::VAR_IS_ESSENTIAL, comment
	);

	comment = "";
	comment += "The number of successful calls to an adaptor after which adaption;";
	comment += "of mutation parameters takes place (e.g sigma-variation in gauss mutation);";
	gpb.registerFileParameter<std::uint32_t>(
		"adaptionThreshold", adaptionThreshold_.reference(), GFI_DEF_ADAPTIONTHRESHOLD, Gem::Common::VAR_IS_ESSENTIAL,
		comment
	);

	comment = "";
	comment += "Whether to use a double gaussion for the adaption of parmeters in ES;";
	gpb.registerFileParameter<bool>(
		"useBiGaussian", useBiGaussian_.reference(), GFI_DEF_USEBIGAUSSIAN, Gem::Common::VAR_IS_ESSENTIAL, comment
	);

	comment = "";
	comment += "The sigma for gauss-adaption in ES;(or the sigma of the left peak of a double gaussian);";
	gpb.registerFileParameter<double>(
		"sigma1", sigma1_.reference(), GFI_DEF_SIGMA1, Gem::Common::VAR_IS_ESSENTIAL, comment
	);

	comment = "";
	comment += "Influences the self-adaption of gauss-mutation in ES;";
	gpb.registerFileParameter<double>(
		"sigmaSigma1", sigmaSigma1_.reference(), GFI_DEF_SIGMASIGMA1, Gem::Common::VAR_IS_ESSENTIAL, comment
	);

	comment = "";
	comment += "The minimum value of sigma1;";
	gpb.registerFileParameter<double>(
		"minSigma1", minSigma1_.reference(), GFI_DEF_MINSIGMA1, Gem::Common::VAR_IS_ESSENTIAL, comment
	);

	comment = "";
	comment += "The maximum value of sigma1;";
	gpb.registerFileParameter<double>(
		"maxSigma1", maxSigma1_.reference(), GFI_DEF_MAXSIGMA1, Gem::Common::VAR_IS_ESSENTIAL, comment
	);

	comment = "";
	comment += "The sigma of the right peak of a double gaussian (if any);";
	gpb.registerFileParameter<double>(
		"sigma2", sigma2_.reference(), GFI_DEF_SIGMA2, Gem::Common::VAR_IS_ESSENTIAL, comment
	);

	comment = "";
	comment += "Influences the self-adaption of gauss-mutation in ES;";
	gpb.registerFileParameter<double>(
		"sigmaSigma2", sigmaSigma2_.reference(), GFI_DEF_SIGMASIGMA2, Gem::Common::VAR_IS_ESSENTIAL, comment
	);

	comment = "";
	comment += "The minimum value of sigma2;";
	gpb.registerFileParameter<double>(
		"minSigma2", minSigma2_.reference(), GFI_DEF_MINSIGMA2, Gem::Common::VAR_IS_ESSENTIAL, comment
	);

	comment = "";
	comment += "The maximum value of sigma2;";
	gpb.registerFileParameter<double>(
		"maxSigma2", maxSigma2_.reference(), GFI_DEF_MAXSIGMA2, Gem::Common::VAR_IS_ESSENTIAL, comment
	);

	comment = "";
	comment += "The start distance between both peaks used for bi-gaussian mutations in ES;";
	gpb.registerFileParameter<double>(
		"delta", delta_.reference(), GFI_DEF_DELTA, Gem::Common::VAR_IS_ESSENTIAL, comment
	);

	comment = "";
	comment += "The width of the gaussian used for mutations of the delta parameter;";
	gpb.registerFileParameter<double>(
		"sigmaDelta", sigmaDelta_.reference(), GFI_DEF_SIGMADELTA, Gem::Common::VAR_IS_ESSENTIAL, comment
	);

	comment = "";
	comment += "The minimum allowed value of delta;";
	gpb.registerFileParameter<double>(
		"minDelta", minDelta_.reference(), GFI_DEF_MINDELTA, Gem::Common::VAR_IS_ESSENTIAL, comment
	);

	comment = "";
	comment += "The maximum allowed value of delta;";
	gpb.registerFileParameter<double>(
		"maxDelta", maxDelta_.reference(), GFI_DEF_MAXDELTA, Gem::Common::VAR_IS_ESSENTIAL, comment
	);

	comment = "";
	comment += "The number of dimensions used for the demo function;";
	gpb.registerFileParameter<std::size_t>(
		"parDim", parDim_.reference(), GFI_DEF_PARDIM, Gem::Common::VAR_IS_ESSENTIAL, comment
	);

	comment = "";
	comment += "The lower boundary of the initialization range for parameters;";
	gpb.registerFileParameter<double>(
		"minVar", minVar_.reference(), GFI_DEF_MINVAR, Gem::Common::VAR_IS_ESSENTIAL, comment
	);

	comment = "";
	comment += "The upper boundary of the initialization range for parameters;";
	gpb.registerFileParameter<double>(
		"maxVar", maxVar_.reference(), GFI_DEF_MAXVAR, Gem::Common::VAR_IS_ESSENTIAL, comment
	);

	comment = "";
	comment += "Indicates what type of parameter object should be used;(0) GDoubleCollection;(1) GConstrainedDoubleCollection;(2) GDoubleObjectCollection; (3) GConstrainedDoubleObjectCollection; (4) GConstrainedDoubleObjects on the root level;";
	gpb.registerFileParameter<parameterType>(
		"parameterType", pT_.reference(), GFI_DEF_PARAMETERTYPE, Gem::Common::VAR_IS_ESSENTIAL, comment
	);

	comment = "";
	comment += "Indicates how the parameters are initialized;(0) randomly;(1) with a value on the perimeter of the allowed or recommended value range";
	gpb.registerFileParameter<initMode>(
		"initMode", iM_.reference(), GFI_DEF_INITMODE, Gem::Common::VAR_IS_ESSENTIAL, comment
	);

	// Allow our parent class to describe its options
	GParameterSetFactory::describeLocalOptions_(gpb);
}

/******************************************************************************/
/**
 * Allows to act on the configuration options received from the configuration file. Here
 * we can add the options described in describeLocalOptions to the object. In practice,
 * we add the parameter objects here
 *
 * @param p A smart-pointer to be acted on during post-processing
 */
void GFunctionIndividualFactory::postProcess_(std::shared_ptr < GParameterSet > &p) {
	// Set up a random number generator
	Gem::Hap::GRandom gr;

	// Set up an adaptor for the collections, so they know how to be adapted
	std::shared_ptr <GAdaptorT<double>> gat_ptr;
	if (useBiGaussian_.value()) {
		std::shared_ptr <GDoubleBiGaussAdaptor> gdbga_ptr(new GDoubleBiGaussAdaptor());
		gdbga_ptr->setAllSigma1(sigma1_.value(), sigmaSigma1_.value(), minSigma1_.value(), maxSigma1_.value());
		gdbga_ptr->setAllSigma1(sigma2_.value(), sigmaSigma2_.value(), minSigma2_.value(), maxSigma2_.value());
		gdbga_ptr->setAllSigma1(delta_.value(), sigmaDelta_.value(), minDelta_.value(), maxDelta_.value());
		gdbga_ptr->setAdaptionThreshold(adaptionThreshold_.value());
		gdbga_ptr->setAdaptionProbability(adProb_.value());
		gat_ptr = gdbga_ptr;
	} else {
		std::shared_ptr <GDoubleGaussAdaptor> gdga_ptr(
			new GDoubleGaussAdaptor(sigma1_.value(), sigmaSigma1_.value(), minSigma1_.value(), maxSigma1_.value()));
		gdga_ptr->setAdaptionThreshold(adaptionThreshold_.value());
		gdga_ptr->setAdaptionProbability(adProb_.value());
		gat_ptr = gdga_ptr;
	}

	// Store parameters pertaining to the adaption probability in the adaptor
	gat_ptr->setAdaptAdProb(adaptAdProb_.value());
	gat_ptr->setAdProbRange(minAdProb_.value(), maxAdProb_.value());

	// Find out about the amount of data items to be added
	// std::size_t nData = parDimLocal_?parDimLocal_:parDim_;
	std::size_t nData = parDim_.value();

	// Set up the data collections
	switch (pT_.value()) {
		case parameterType::USEGDOUBLECOLLECTION: {
			// Set up a collection, each initialized with a random number in the range [min,max[
			// Random initialization happens in the constructor.
			std::shared_ptr <GDoubleCollection> gdc_ptr;

			if (initMode::INITRANDOM == iM_.value()) {
				gdc_ptr = std::shared_ptr<GDoubleCollection>(new GDoubleCollection(nData, minVar_.value(), maxVar_.value()));
			} else { // initMode::INITPERIMETER
				gdc_ptr = std::shared_ptr<GDoubleCollection>(new GDoubleCollection(nData, minVar_.value(), minVar_.value(), maxVar_.value()));
			}

			gdc_ptr->addAdaptor(gat_ptr);
			gdc_ptr->setParameterName("var0");

			p->push_back(gdc_ptr);
		}
			break;

		case parameterType::USEGCONSTRAINEDOUBLECOLLECTION: {

			// Set up a collection
			std::shared_ptr <GConstrainedDoubleCollection> gcdc_ptr;

			if (initMode::INITRANDOM == iM_) {
				gcdc_ptr = std::shared_ptr<GConstrainedDoubleCollection>(new GConstrainedDoubleCollection(nData, minVar_.value(), maxVar_.value()));
			} else { // initMode::INITPERIMETER
				gcdc_ptr = std::shared_ptr<GConstrainedDoubleCollection>(new GConstrainedDoubleCollection(nData, minVar_.value(), minVar_.value(), maxVar_.value()));
			}

			gcdc_ptr->addAdaptor(gat_ptr);
			gcdc_ptr->setParameterName("var0");

			p->push_back(gcdc_ptr);
		}
			break;

		case parameterType::USEGDOUBLEOBJECTCOLLECTION: {
			// Set up a collection of GDoubleObject objects
			std::shared_ptr <GDoubleObjectCollection> gdoc_ptr(new GDoubleObjectCollection());

			// Fill the collection with GDoubleObject objects, each equipped with a copy of our adaptor
			// Note that addAdaptor() itself will take care of cloning the adaptor
			for (std::size_t i = 0; i < nData; i++) {
				std::shared_ptr<GDoubleObject> gdo_ptr(new GDoubleObject(minVar_.value(), maxVar_.value()));
				if (initMode::INITPERIMETER == iM_.value()) { *gdo_ptr = minVar_.value(); }

				gdo_ptr->addAdaptor(gat_ptr);
				gdo_ptr->setParameterName(std::string("var") + Gem::Common::to_string(i));

				gdoc_ptr->push_back(gdo_ptr);
			}

			p->push_back(gdoc_ptr);
		}
			break;

		case parameterType::USEGCONSTRAINEDDOUBLEOBJECTCOLLECTION: {
			// Set up a collection of GConstrainedDoubleObject objects
			std::shared_ptr <GConstrainedDoubleObjectCollection> gcdoc_ptr(new GConstrainedDoubleObjectCollection());

			// Fill the collection with GConstrainedDoubleObject objects, each equipped with a copy of our adaptor
			// Note that addAdaptor() itself will take care of cloning the adaptor
			for (std::size_t i = 0; i < nData; i++) {
				std::shared_ptr<GConstrainedDoubleObject> gcdo_ptr(new GConstrainedDoubleObject(minVar_.value(), maxVar_.value()));
				if (initMode::INITPERIMETER == iM_.value()) { *gcdo_ptr = minVar_.value(); }

				gcdo_ptr->addAdaptor(gat_ptr);
				gcdo_ptr->setParameterName(std::string("var") + Gem::Common::to_string(i));

				gcdoc_ptr->push_back(gcdo_ptr);
			}

			p->push_back(gcdoc_ptr);
		}
			break;

		case parameterType::USEGCONSTRAINEDDOUBLEOBJECT: {
			// Fill the individual with GConstrainedDoubleObject objects, each equipped with a copy of our adaptor
			// Note that addAdaptor() itself will take care of cloning the adaptor
			for (std::size_t i = 0; i < nData; i++) {
				std::shared_ptr<GConstrainedDoubleObject> gcdo_ptr(new GConstrainedDoubleObject(minVar_.value(), maxVar_.value()));
				if (initMode::INITPERIMETER == iM_.value()) { *gcdo_ptr = minVar_.value(); }

				gcdo_ptr->addAdaptor(gat_ptr);
				gcdo_ptr->setParameterName(std::string("var") + Gem::Common::to_string(i));

				p->push_back(gcdo_ptr);
			}
		}
			break;

		default: {
			throw gemfony_exception(
				g_error_streamer(DO_LOG,  time_and_place)
					<< "In GFunctionIndividualFactory::postProcess_(): Error!"
					<< "Found invalid pT_: " << pT_ << std::endl
			);
		}
			break;
	}
}

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */
