/**
 * @file GFunctionIndividual.cpp
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

#include "geneva-individuals/GFunctionIndividual.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GFunctionIndividual)
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GFunctionIndividualFactory)
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GDoubleSumConstraint)
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GDoubleSumGapConstraint)
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GSphereConstraint)

namespace Gem {
namespace Geneva {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * The default constructor
 */
GDoubleSumConstraint::GDoubleSumConstraint()
	: C_(1.) { /* nothing */ }

/******************************************************************************/
/**
 * Initialization with the constant
 */
GDoubleSumConstraint::GDoubleSumConstraint(const double &C)
	: C_(C) { /* nothing */ }

/******************************************************************************/
/**
 * The copy constructor
 */
GDoubleSumConstraint::GDoubleSumConstraint(const GDoubleSumConstraint &cp)
	: GParameterSetConstraint(cp), C_(cp.C_) { /* nothing */ }

/******************************************************************************/
/**
 * The destructor
 */
GDoubleSumConstraint::~GDoubleSumConstraint() { /* nothing */ }

/******************************************************************************/
/**
 * A standard assignment operator
 */
const GDoubleSumConstraint &GDoubleSumConstraint::operator=(const GDoubleSumConstraint &cp) {
	GParameterSetConstraint::load_(&cp);
	return *this;
}

/******************************************************************************/
/**
 * Checks for equality with another GIndividualConstraint object
 */
bool GDoubleSumConstraint::operator==(const GDoubleSumConstraint &cp) const {
	using namespace Gem::Common;
	try {
		this->compare(cp, CE_EQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
		return true;
	} catch (g_expectation_violation &) {
		return false;
	}
}

/******************************************************************************/
/**
 * Checks for inequality with another GIndividualConstraint object
 */
bool GDoubleSumConstraint::operator!=(const GDoubleSumConstraint &cp) const {
	using namespace Gem::Common;
	try {
		this->compare(cp, CE_INEQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
		return true;
	} catch (g_expectation_violation &) {
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
void GDoubleSumConstraint::compare(
	const GObject &cp, const Gem::Common::expectation &e, const double &limit
) const {
	using namespace Gem::Common;

	// Check that we are dealing with a GDoubleSumConstraint reference independent of this object and convert the pointer
	const GDoubleSumConstraint *p_load = Gem::Common::g_convert_and_compare<GObject, GDoubleSumConstraint>(cp, this);

	Gem::Common::GToken token("GDoubleSumConstraint", e);

	// Compare our parent data ...
	Gem::Common::compare_base<GParameterSetConstraint>(IDENTITY(*this, *p_load), token);

	// ... and then the local data
	Gem::Common::compare_t(IDENTITY(C_, p_load->C_), token);

	// React on deviations from the expectation
	token.evaluate();
}

/******************************************************************************/
/**
 * Adds local configuration options to a GParserBuilder object
 */
void GDoubleSumConstraint::addConfigurationOptions(
	Gem::Common::GParserBuilder &gpb
) {
	// Call our parent class'es function
	GParameterSetConstraint::addConfigurationOptions(gpb);
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
 * The default constructor
 */
GDoubleSumGapConstraint::GDoubleSumGapConstraint()
	: C_(1.), gap_(0.5) { /* nothing */ }

/******************************************************************************/
/**
 * Initialization with the constant
 */
GDoubleSumGapConstraint::GDoubleSumGapConstraint(const double &C, const double &gap)
	: C_(C), gap_(gap) { /* nothing */ }

/******************************************************************************/
/**
 * The copy constructor
 */
GDoubleSumGapConstraint::GDoubleSumGapConstraint(const GDoubleSumGapConstraint &cp)
	: GParameterSetConstraint(cp), C_(cp.C_), gap_(cp.gap_) { /* nothing */ }

/******************************************************************************/
/**
 * The destructor
 */
GDoubleSumGapConstraint::~GDoubleSumGapConstraint() { /* nothing */ }

/******************************************************************************/
/**
 * A standard assignment operator
 */
const GDoubleSumGapConstraint &GDoubleSumGapConstraint::operator=(const GDoubleSumGapConstraint &cp) {
	GParameterSetConstraint::load_(&cp);
	return *this;
}

/******************************************************************************/
/**
 * Checks for equality with another GIndividualConstraint object
 */
bool GDoubleSumGapConstraint::operator==(const GDoubleSumGapConstraint &cp) const {
	using namespace Gem::Common;
	try {
		this->compare(cp, CE_EQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
		return true;
	} catch (g_expectation_violation &) {
		return false;
	}
}

/******************************************************************************/
/**
 * Checks for inequality with another GIndividualConstraint object
 */
bool GDoubleSumGapConstraint::operator!=(const GDoubleSumGapConstraint &cp) const {
	using namespace Gem::Common;
	using namespace Gem::Common;
	try {
		this->compare(cp, CE_INEQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
		return true;
	} catch (g_expectation_violation &) {
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
void GDoubleSumGapConstraint::compare(
	const GObject &cp, const Gem::Common::expectation &e, const double &limit
) const {
	using namespace Gem::Common;

	// Check that we are dealing with a GDoubleSumGapConstraint reference independent of this object and convert the pointer
	const GDoubleSumGapConstraint *p_load = Gem::Common::g_convert_and_compare<GObject, GDoubleSumGapConstraint>(cp, this);

	Gem::Common::GToken token("GDoubleSumGapConstraint", e);

	// Compare our parent data ...
	Gem::Common::compare_base<GParameterSetConstraint>(IDENTITY(*this, *p_load), token);

	// ... and then the local data
	Gem::Common::compare_t(IDENTITY(C_, p_load->C_), token);

	// React on deviations from the expectation
	token.evaluate();
}

/******************************************************************************/
/**
 * Adds local configuration options to a GParserBuilder object
 */
void GDoubleSumGapConstraint::addConfigurationOptions(
	Gem::Common::GParserBuilder &gpb
) {
	// Call our parent class'es function
	GParameterSetConstraint::addConfigurationOptions(gpb);
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
 * The default constructor
 */
GSphereConstraint::GSphereConstraint()
	: diameter_(1.) { /* nothing */ }

/******************************************************************************/
/**
 * Initialization with the diameter
 */
GSphereConstraint::GSphereConstraint(const double &diameter)
	: diameter_(diameter) { /* nothing */ }

/******************************************************************************/
/**
 * The copy constructor
 */
GSphereConstraint::GSphereConstraint(const GSphereConstraint &cp)
	: GParameterSetConstraint(cp), diameter_(cp.diameter_) { /* nothing */ }

/******************************************************************************/
/**
 * The destructor
 */
GSphereConstraint::~GSphereConstraint() { /* nothing */ }

/******************************************************************************/
/**
 * A standard assignment operator
 */
const GSphereConstraint &GSphereConstraint::operator=(const GSphereConstraint &cp) {
	GParameterSetConstraint::load_(&cp);
	return *this;
}

/******************************************************************************/
/**
 * Checks for equality with another GIndividualConstraint object
 */
bool GSphereConstraint::operator==(const GSphereConstraint &cp) const {
	using namespace Gem::Common;
	try {
		this->compare(cp, CE_EQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
		return true;
	} catch (g_expectation_violation &) {
		return false;
	}
}

/******************************************************************************/
/**
 * Checks for inequality with another GIndividualConstraint object
 */
bool GSphereConstraint::operator!=(const GSphereConstraint &cp) const {
	using namespace Gem::Common;
	try {
		this->compare(cp, CE_INEQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
		return true;
	} catch (g_expectation_violation &) {
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
void GSphereConstraint::compare(
	const GObject &cp, const Gem::Common::expectation &e, const double &limit
) const {
	// Check that we are dealing with a GSphereConstraint reference independent of this object and convert the pointer
	const GSphereConstraint *p_load = Gem::Common::g_convert_and_compare<GObject, GSphereConstraint>(cp, this);

	Gem::Common::GToken token("GSphereConstraint", e);

	// Compare our parent data ...
	Gem::Common::compare_base<GParameterSetConstraint>(IDENTITY(*this, *p_load), token);

	// ... and then the local data
	Gem::Common::compare_t(IDENTITY(diameter_, p_load->diameter_), token);

	// React on deviations from the expectation
	token.evaluate();
}

/******************************************************************************/
/**
 * Adds local configuration options to a GParserBuilder object
 */
void GSphereConstraint::addConfigurationOptions(
	Gem::Common::GParserBuilder &gpb
) {
	// Call our parent class'es function
	GParameterSetConstraint::addConfigurationOptions(gpb);
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
	boost::uint16_t tmp = static_cast<boost::uint16_t>(ur);
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
	boost::uint16_t tmp;
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
	boost::uint16_t tmp = static_cast<boost::uint16_t>(ur);
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
	boost::uint16_t tmp;
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
	boost::uint16_t tmp = static_cast<boost::uint16_t>(ur);
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
	boost::uint16_t tmp;
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
 * The default constructor
 */
GFunctionIndividual::GFunctionIndividual()
	: demoFunction_(PARABOLA) { /* nothing */ }

/******************************************************************************/
/**
 * Initialization with the desired demo function
 *
 * @param dF The id of the demo function
 */
GFunctionIndividual::GFunctionIndividual(const solverFunction &dF)
	: demoFunction_(dF) { /* nothing */ }

/******************************************************************************/
/**
 * A standard copy constructor
 *
 * @param cp A copy of another GFunctionIndidivual
 */
GFunctionIndividual::GFunctionIndividual(const GFunctionIndividual &cp)
	: GParameterSet(cp), demoFunction_(cp.demoFunction_) { /* nothing */ }

/******************************************************************************/
/**
 * The standard destructor
 */
GFunctionIndividual::~GFunctionIndividual() { /* nothing */   }

/******************************************************************************/
/**
 * A standard assignment operator
 *
 * @param cp A copy of another GFunctionIndividual
 */
const GFunctionIndividual &GFunctionIndividual::operator=(const GFunctionIndividual &cp) {
	GFunctionIndividual::load_(&cp);
	return *this;
}

/******************************************************************************/
/**
 * Checks for equality with another GFunctionIndividual object
 *
 * @param  cp A constant reference to another GFunctionIndividual object
 * @return A boolean indicating whether both objects are equal
 */
bool GFunctionIndividual::operator==(const GFunctionIndividual &cp) const {
	using namespace Gem::Common;
	try {
		this->compare(cp, CE_EQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
		return true;
	} catch (g_expectation_violation &) {
		return false;
	}
}

/******************************************************************************/
/**
 * Checks for inequality with another GFunctionIndividual object
 *
 * @param  cp A constant reference to another GFunctionIndividual object
 * @return A boolean indicating whether both objects are in-equal
 */
bool GFunctionIndividual::operator!=(const GFunctionIndividual &cp) const {
	using namespace Gem::Common;
	try {
		this->compare(cp, CE_INEQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
		return true;
	} catch (g_expectation_violation &) {
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
void GFunctionIndividual::compare(
	const GObject &cp, const Gem::Common::expectation &e, const double &limit
) const {
	// Check that we are dealing with a GFunctionIndividual reference independent of this object and convert the pointer
	const GFunctionIndividual *p_load = Gem::Common::g_convert_and_compare<GObject, GFunctionIndividual>(cp, this);

	Gem::Common::GToken token("GFunctionIndividual", e);

	// Compare our parent data ...
	Gem::Common::compare_base<GParameterSet>(IDENTITY(*this, *p_load), token);

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
void GFunctionIndividual::addConfigurationOptions(
	Gem::Common::GParserBuilder &gpb
) {
	// Call our parent class'es function
	GParameterSet::addConfigurationOptions(gpb);

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
		case PARABOLA: {
			for (std::size_t i = 0; i < parameterSize; i++) {
				result += GSQUARED(parVec[i]);
			}
		}
			break;

			//-----------------------------------------------------------
			// A "noisy" parabola, i.e. a parabola with a very large number of overlaid local optima
		case NOISYPARABOLA: {
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
		case ROSENBROCK: {
#ifdef DEBUG
		// Check the size of the parameter vector -- must be at least 2
		if(parameterSize < 2) {
		   glogger
		   << "In GFunctionIndividual::fitnessCalculation() / ROSENBROCK: Error!" << std::endl
         << "Need to use at least two input dimensions, but got " << parameterSize << std::endl
         << GEXCEPTION;
		}
#endif /* DEBUG */

			for (std::size_t i = 0; i < (parameterSize - 1); i++) {
				result += 100. * GSQUARED(GSQUARED(parVec[i]) - parVec[i + 1]) + GSQUARED(1. - parVec[i]);
			}
		}
			break;

			//-----------------------------------------------------------
			// The Ackeley function (see e.g. http://www.it.lut.fi/ip/evo/functions/node14.html)
		case ACKLEY: {
#ifdef DEBUG
		// Check the size of the parameter vector -- must be at least 2
		if(parameterSize < 2) {
		   glogger
		   << "In GFunctionIndividual::fitnessCalculation() / ACKLEY: Error!" << std::endl
         << "Need to use at least two input dimensions, but got " << parameterSize << std::endl
         << GEXCEPTION;
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
		case RASTRIGIN: {
			result = 10 * double(parameterSize);

			for (std::size_t i = 0; i < parameterSize; i++) {
				result += (GSQUARED(parVec[i]) - 10. * cos(2 * boost::math::constants::pi<double>() * parVec[i]));
			}
		}
			break;

			//-----------------------------------------------------------
			// The Schwefel function (see e.g. http://www.it.lut.fi/ip/evo/functions/node10.html)
		case SCHWEFEL: {
			for (std::size_t i = 0; i < parameterSize; i++) {
				result += -parVec[i] * sin(sqrt(fabs(parVec[i])));
			}

			result /= parameterSize;
		}
			break;

			//-----------------------------------------------------------
			// The Salomon function (see e.g. http://www.it.lut.fi/ip/evo/functions/node12.html)
		case SALOMON: {
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
		case NEGPARABOLA: {
			for (std::size_t i = 0; i < parameterSize; i++) {
				result += GSQUARED(parVec[i]);
			}
			result *= -1.;
		}
			break;

			//-----------------------------------------------------------

		default: {
			glogger
			<< "In GFunctionIndividual::fitnessCalculation(): Error!" << std::endl
			<< "Got invalid function type" << std::endl
			<< GEXCEPTION;
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

	std::vector<double>::iterator it;
	std::cout << "Fitness: " << f.fitness() << std::endl;
	for (it = parVec.begin(); it != parVec.end(); ++it) {
		std::cout << (it - parVec.begin()) << ": " << *it << std::endl;
	}

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
GFunctionIndividualFactory::GFunctionIndividualFactory(const std::string &configFile)
	: Gem::Common::GFactoryT<GParameterSet>(configFile), adProb_(GFI_DEF_ADPROB), adaptAdProb_(GFI_DEF_ADAPTADPROB),
	  minAdProb_(GFI_DEF_MINADPROB), maxAdProb_(GFI_DEF_MAXADPROB), adaptionThreshold_(GFI_DEF_ADAPTIONTHRESHOLD),
	  useBiGaussian_(GFI_DEF_USEBIGAUSSIAN), sigma1_(GFI_DEF_SIGMA1), sigmaSigma1_(GFI_DEF_SIGMASIGMA1),
	  minSigma1_(GFI_DEF_MINSIGMA1), maxSigma1_(GFI_DEF_MAXSIGMA1), sigma2_(GFI_DEF_SIGMA2),
	  sigmaSigma2_(GFI_DEF_SIGMASIGMA2), minSigma2_(GFI_DEF_MINSIGMA2), maxSigma2_(GFI_DEF_MAXSIGMA2),
	  delta_(GFI_DEF_DELTA), sigmaDelta_(GFI_DEF_SIGMADELTA), minDelta_(GFI_DEF_MINDELTA), maxDelta_(GFI_DEF_MAXDELTA),
	  parDim_(GFI_DEF_PARDIM), minVar_(GFI_DEF_MINVAR), maxVar_(GFI_DEF_MAXVAR), pT_(GFI_DEF_PARAMETERTYPE),
	  iM_(GFI_DEF_INITMODE) { /* nothing */ }

/******************************************************************************/
/**
 * The copy constructor
 */
GFunctionIndividualFactory::GFunctionIndividualFactory(const GFunctionIndividualFactory &cp)
	: Gem::Common::GFactoryT<GParameterSet>(cp), adProb_(cp.adProb_), adaptAdProb_(cp.adaptAdProb_),
	  minAdProb_(cp.minAdProb_), maxAdProb_(cp.maxAdProb_), adaptionThreshold_(cp.adaptionThreshold_),
	  useBiGaussian_(cp.useBiGaussian_), sigma1_(cp.sigma1_), sigmaSigma1_(cp.sigmaSigma1_), minSigma1_(cp.minSigma1_),
	  maxSigma1_(cp.maxSigma1_), sigma2_(cp.sigma2_), sigmaSigma2_(cp.sigmaSigma2_), minSigma2_(cp.minSigma2_),
	  maxSigma2_(cp.maxSigma2_), delta_(cp.delta_), sigmaDelta_(cp.sigmaDelta_), minDelta_(cp.minDelta_),
	  maxDelta_(cp.maxDelta_), parDim_(cp.parDim_), minVar_(cp.minVar_), maxVar_(cp.maxVar_), pT_(cp.pT_),
	  iM_(cp.iM_) { /* nothing */ }

/******************************************************************************/
/**
 * The default constructor. Only needed for (de-)serialization purposes, hence empty.
 */
GFunctionIndividualFactory::GFunctionIndividualFactory()
	: Gem::Common::GFactoryT<GParameterSet>("empty"), adProb_(GFI_DEF_ADPROB), adaptAdProb_(GFI_DEF_ADAPTADPROB),
	  minAdProb_(GFI_DEF_MINADPROB), maxAdProb_(GFI_DEF_MAXADPROB), adaptionThreshold_(GFI_DEF_ADAPTIONTHRESHOLD),
	  useBiGaussian_(GFI_DEF_USEBIGAUSSIAN), sigma1_(GFI_DEF_SIGMA1), sigmaSigma1_(GFI_DEF_SIGMASIGMA1),
	  minSigma1_(GFI_DEF_MINSIGMA1), maxSigma1_(GFI_DEF_MAXSIGMA1), sigma2_(GFI_DEF_SIGMA2),
	  sigmaSigma2_(GFI_DEF_SIGMASIGMA2), minSigma2_(GFI_DEF_MINSIGMA2), maxSigma2_(GFI_DEF_MAXSIGMA2),
	  delta_(GFI_DEF_DELTA), sigmaDelta_(GFI_DEF_SIGMADELTA), minDelta_(GFI_DEF_MINDELTA), maxDelta_(GFI_DEF_MAXDELTA),
	  parDim_(GFI_DEF_PARDIM), minVar_(GFI_DEF_MINVAR), maxVar_(GFI_DEF_MAXVAR), pT_(GFI_DEF_PARAMETERTYPE),
	  iM_(GFI_DEF_INITMODE) { /* nothing */ }

/******************************************************************************/
/**
 * The destructor
 */
GFunctionIndividualFactory::~GFunctionIndividualFactory() { /* nothing */ }

/******************************************************************************/
/**
 * Loads the data of another GFunctionIndividualFactory object
 */
void GFunctionIndividualFactory::load(std::shared_ptr < Gem::Common::GFactoryT<GParameterSet>> cp_raw_ptr) {
	// Load our parent class'es data
	Gem::Common::GFactoryT<GParameterSet>::load(cp_raw_ptr);

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
		glogger
		<< "In GFunctionIndividualFactory::setParDim(): Error!" << std::endl
		<< "Dimension of the function is set to 0" << std::endl
		<< GEXCEPTION;
	}

	parDim_ = parDim;
}

/******************************************************************************/
/**
 * Extract the minimum and maximum boundaries of the variables
 */
std::tuple<double, double> GFunctionIndividualFactory::getVarBoundaries() const {
	return std::tuple<double, double>(minVar_, maxVar_);
}

/******************************************************************************/
/**
 * Set the minimum and maximum boundaries of the variables
 */
void GFunctionIndividualFactory::setVarBoundaries(std::tuple<double, double> boundaries) {
	double min = std::get<0>(boundaries);
	double max = std::get<1>(boundaries);

	if (min >= max) {
		glogger
		<< "In GFunctionIndividualFactory::setVarBoundaries(): Error!" << std::endl
		<< "Received invalid boundaries " << min << " / " << max << std::endl
		<< GEXCEPTION;
	}

	setMinVar(min);
	setMaxVar(max);
}

/******************************************************************************/
/**
 * Get the value of the adaptionThreshold_ variable
 */
boost::uint32_t GFunctionIndividualFactory::getAdaptionThreshold() const {
	return adaptionThreshold_;
}

/******************************************************************************/
/**
 * Set the value of the adaptionThreshold_ variable
 */
void GFunctionIndividualFactory::setAdaptionThreshold(
	boost::uint32_t adaptionThreshold
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
	return std::tuple<double, double>(minDelta_, maxDelta_);
}

/******************************************************************************/
/**
 * Allows to set the allowed value range of delta
 */
void GFunctionIndividualFactory::setDeltaRange(std::tuple<double, double> range) {
	double min = std::get<0>(range);
	double max = std::get<1>(range);

	if (min < 0) {
		glogger
		<< "In GFunctionIndividualFactory::setDeltaRange(): Error" << std::endl
		<< "min must be >= 0. Got : " << max << std::endl
		<< GEXCEPTION;
	}

	if (min >= max) {
		glogger
		<< "In GFunctionIndividualFactory::setDeltaRange(): Error" << std::endl
		<< "Invalid range specified: " << min << " / " << max << std::endl
		<< GEXCEPTION;
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
	return std::tuple<double, double>(minSigma1_, maxSigma1_);
}

/******************************************************************************/
/**
 * Allows to set the allowed value range of sigma1_
 */
void GFunctionIndividualFactory::setSigma1Range(std::tuple<double, double> range) {
	double min = std::get<0>(range);
	double max = std::get<1>(range);

	if (min < 0) {
		glogger
		<< "In GFunctionIndividualFactory::setSigma1Range(): Error" << std::endl
		<< "min must be >= 0. Got : " << max << std::endl
		<< GEXCEPTION;
	}

	if (min >= max) {
		glogger
		<< "In GFunctionIndividualFactory::setSigma1Range(): Error" << std::endl
		<< "Invalid range specified: " << min << " / " << max << std::endl
		<< GEXCEPTION;
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
	return std::tuple<double, double>(minSigma2_, maxSigma2_);
}

/******************************************************************************/
/**
 * Allows to set the allowed value range of sigma2_
 */
void GFunctionIndividualFactory::setSigma2Range(std::tuple<double, double> range) {
	double min = std::get<0>(range);
	double max = std::get<1>(range);

	if (min < 0) {
		glogger
		<< "In GFunctionIndividualFactory::setSigma2Range(): Error" << std::endl
		<< "min must be >= 0. Got : " << max << std::endl
		<< GEXCEPTION;
	}

	if (min >= max) {
		glogger
		<< "In GFunctionIndividualFactory::setSigma2Range(): Error" << std::endl
		<< "Invalid range specified: " << min << " / " << max << std::endl
		<< GEXCEPTION;
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
      glogger
      << "In GFunctionIndividualFactory::setAdaptAdProb(): Error!" << std::endl
      << "Invalid value for adaptAdProb given: " << adaptAdProb << std::endl
      << GEXCEPTION;
   }
#endif /* DEBUG */

	adaptAdProb_ = adaptAdProb;
}

/******************************************************************************/
/**
 * Allows to retrieve the allowed range for adProb_ variation
 */
std::tuple<double, double> GFunctionIndividualFactory::getAdProbRange() const {
	return std::tuple<double, double>(minAdProb_.value(), maxAdProb_.value());
}

/******************************************************************************/
/**
 * Allows to set the allowed range for adaption probability variation
 */
void GFunctionIndividualFactory::setAdProbRange(double minAdProb, double maxAdProb) {
#ifdef DEBUG
   if(minAdProb < 0.) {
      glogger
      << "In GFunctionIndividualFactory::setAdProbRange(): Error!" << std::endl
      << "minAdProb < 0: " << minAdProb << std::endl
      << GEXCEPTION;
   }

   if(minAdProb > maxAdProb) {
      glogger
      << "In GFunctionIndividualFactory::setAdProbRange(): Error!" << std::endl
      << "Invalid minAdProb and/or maxAdProb: " << minAdProb << " / " << maxAdProb << std::endl
      << GEXCEPTION;
   }

   if(maxAdProb > 1.) {
      glogger
      << "In GFunctionIndividualFactory::setAdProbRange(): Error!" << std::endl
      << "maxAdProb > 1: " << maxAdProb << std::endl
      << GEXCEPTION;
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
	gpb.registerFileParameter<boost::uint32_t>(
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
	Gem::Common::GFactoryT<GParameterSet>::describeLocalOptions_(gpb);
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
	if (useBiGaussian_) {
		std::shared_ptr <GDoubleBiGaussAdaptor> gdbga_ptr(new GDoubleBiGaussAdaptor());
		gdbga_ptr->setAllSigma1(sigma1_, sigmaSigma1_, minSigma1_, maxSigma1_);
		gdbga_ptr->setAllSigma1(sigma2_, sigmaSigma2_, minSigma2_, maxSigma2_);
		gdbga_ptr->setAllSigma1(delta_, sigmaDelta_, minDelta_, maxDelta_);
		gdbga_ptr->setAdaptionThreshold(adaptionThreshold_);
		gdbga_ptr->setAdaptionProbability(adProb_);
		gat_ptr = gdbga_ptr;
	} else {
		std::shared_ptr <GDoubleGaussAdaptor> gdga_ptr(
			new GDoubleGaussAdaptor(sigma1_, sigmaSigma1_, minSigma1_, maxSigma1_));
		gdga_ptr->setAdaptionThreshold(adaptionThreshold_);
		gdga_ptr->setAdaptionProbability(adProb_);
		gat_ptr = gdga_ptr;
	}

	// Store parameters pertaining to the adaption probability in the adaptor
	gat_ptr->setAdaptAdProb(adaptAdProb_);
	gat_ptr->setAdProbRange(minAdProb_, maxAdProb_);

	// Find out about the amount of data items to be added
	// std::size_t nData = parDimLocal_?parDimLocal_:parDim_;
	std::size_t nData = parDim_;

	// Find out about the position that should be set to minVar_. Unless randomInit is set,
	// all other positions will be set to the mean value of minVar_ and maxVar_.
	std::size_t perimeterPos = gr.uniform_int<std::size_t>(nData - 1);

	// Set up the data collections
	switch (pT_.value()) {
		case USEGDOUBLECOLLECTION: {
			// Set up a collection, each initialized with a random number in the range [min,max[
			// Random initialization happens in the constructor.
			std::shared_ptr <GDoubleCollection> gdc_ptr;

			if (INITRANDOM == iM_) {
				gdc_ptr = std::shared_ptr<GDoubleCollection>(new GDoubleCollection(nData, minVar_, maxVar_));
			} else { // INITPERIMETER
				gdc_ptr = std::shared_ptr<GDoubleCollection>(
					new GDoubleCollection(nData, (maxVar_ + minVar_) / 2., minVar_, maxVar_));
				gdc_ptr->at(perimeterPos) = minVar_;
			}

			gdc_ptr->addAdaptor(gat_ptr);
			gdc_ptr->setParameterName("var0");

			p->push_back(gdc_ptr);
		}
			break;

		case USEGCONSTRAINEDOUBLECOLLECTION: {

			// Set up a collection
			std::shared_ptr <GConstrainedDoubleCollection> gcdc_ptr;

			if (INITRANDOM == iM_) {
				gcdc_ptr = std::shared_ptr<GConstrainedDoubleCollection>(
					new GConstrainedDoubleCollection(nData, minVar_, maxVar_));
			} else { // INITPERIMETER
				gcdc_ptr = std::shared_ptr<GConstrainedDoubleCollection>(
					new GConstrainedDoubleCollection(nData, (maxVar_ + minVar_) / 2., minVar_, maxVar_));
				gcdc_ptr->at(perimeterPos) = minVar_;
			}

			gcdc_ptr->addAdaptor(gat_ptr);
			gcdc_ptr->setParameterName("var0");

			p->push_back(gcdc_ptr);
		}
			break;

		case USEGDOUBLEOBJECTCOLLECTION: {
			// Set up a collection of GDoubleObject objects
			std::shared_ptr <GDoubleObjectCollection> gdoc_ptr(new GDoubleObjectCollection());

			// Fill the collection with GDoubleObject objects, each equipped with a copy of our adaptor
			// Note that addAdaptor() itself will take care of cloning the adaptor
			for (std::size_t i = 0; i < nData; i++) {
				std::shared_ptr <GDoubleObject> gdo_ptr(new GDoubleObject(minVar_, maxVar_));
				if (INITPERIMETER == iM_) {
					if (i == perimeterPos) {
						*gdo_ptr = minVar_;
					} else {
						*gdo_ptr = (maxVar_ + minVar_) / 2.;
					}
				}

				gdo_ptr->addAdaptor(gat_ptr);
				gdo_ptr->setParameterName(std::string("var") + boost::lexical_cast<std::string>(i));

				gdoc_ptr->push_back(gdo_ptr);
			}

			p->push_back(gdoc_ptr);
		}
			break;

		case USEGCONSTRAINEDDOUBLEOBJECTCOLLECTION: {
			// Set up a collection of GConstrainedDoubleObject objects
			std::shared_ptr <GConstrainedDoubleObjectCollection> gcdoc_ptr(new GConstrainedDoubleObjectCollection());

			// Fill the collection with GConstrainedDoubleObject objects, each equipped with a copy of our adaptor
			// Note that addAdaptor() itself will take care of cloning the adaptor
			for (std::size_t i = 0; i < nData; i++) {
				std::shared_ptr <GConstrainedDoubleObject> gcdo_ptr(new GConstrainedDoubleObject(minVar_, maxVar_));
				if (INITPERIMETER == iM_) {
					if (i == perimeterPos) {
						*gcdo_ptr = minVar_;
					} else {
						*gcdo_ptr = (maxVar_ + minVar_) / 2.;
					}
				}

				gcdo_ptr->addAdaptor(gat_ptr);
				gcdo_ptr->setParameterName(std::string("var") + boost::lexical_cast<std::string>(i));

				gcdoc_ptr->push_back(gcdo_ptr);
			}

			p->push_back(gcdoc_ptr);
		}
			break;

		case USEGCONSTRAINEDDOUBLEOBJECT: {
			// Fill the individual with GConstrainedDoubleObject objects, each equipped with a copy of our adaptor
			// Note that addAdaptor() itself will take care of cloning the adaptor
			for (std::size_t i = 0; i < nData; i++) {
				std::shared_ptr <GConstrainedDoubleObject> gcdo_ptr(new GConstrainedDoubleObject(minVar_, maxVar_));
				if (INITPERIMETER == iM_) {
					if (i == perimeterPos) {
						*gcdo_ptr = minVar_;
					} else {
						*gcdo_ptr = (maxVar_ + minVar_) / 2.;
					}
				}

				gcdo_ptr->addAdaptor(gat_ptr);
				gcdo_ptr->setParameterName(std::string("var") + boost::lexical_cast<std::string>(i));

				p->push_back(gcdo_ptr);
			}
		}
			break;

		default: {
			glogger
			<< "In GFunctionIndividualFactory::postProcess_(): Error!"
			<< "Found invalid pT_: " << pT_ << std::endl
			<< GEXCEPTION;
		}
			break;
	}
}

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */
