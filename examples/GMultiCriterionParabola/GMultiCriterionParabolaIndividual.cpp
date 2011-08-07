/**
 * @file GMultiCriterionParabolaIndividual.hpp
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

#include "GMultiCriterionParabolaIndividual.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GMultiCriterionParabolaIndividual)

namespace Gem
{
namespace Geneva
{

/********************************************************************************************/
/**
 * The default constructor. It is declared private in the header file, as it will only be
 * needed for (de-)serialization purposes. As all variables will be set by the Boost.Serialization
 * library, no variables are initialized here.
 */
GMultiCriterionParabolaIndividual::GMultiCriterionParabolaIndividual()
{ /* nothing */ }

/********************************************************************************************/
/**
 * The destructor
 */
GMultiCriterionParabolaIndividual::~GMultiCriterionParabolaIndividual()
{ /* nothing */ }

/********************************************************************************************/
/**
 * The standard constructor. This function will add nPar constrained double parameters to
 * this individual, each of which has a constrained value range [par_min_, par_max_[.
 *
 * @param nPar The number of parameters to be added to this individual
 * @param par_min The lower boundary of the initialization range
 * @param par_max The upper boundary of the initialization range
 */
GMultiCriterionParabolaIndividual::GMultiCriterionParabolaIndividual(
	const std::size_t& nPar
	, const double& par_min
	, const double& par_max
	, const std::vector<double>& minima
)
	: nPar_(nPar)
	, par_min_(par_min)
 	, par_max_(par_max)
	, minima_(minima)
{
	for(std::size_t npar=0; npar<nPar_; npar++) {
		// GConstrainedDoubleObject cannot assume value below or above par_min_/max_
		boost::shared_ptr<GConstrainedDoubleObject> gcdo_ptr(new GConstrainedDoubleObject(par_min_, par_max_));
		// Assign a random value in the expected range
		gcdo_ptr->setValue(gr.uniform_real<double>(par_min_, par_max_));
		// Add the parameters to this individual
		this->push_back(gcdo_ptr);
	}
}

/********************************************************************************************/
/**
 * A standard copy constructor. All real work is done by the parent class. All we need to
 * do here is to copy local data.
 *
 * @param cp A copy of another GMultiCriterionParabolaIndividual
 */
GMultiCriterionParabolaIndividual::GMultiCriterionParabolaIndividual(const GMultiCriterionParabolaIndividual& cp)
	: GParameterSet(cp)
	, nPar_(cp.nPar_)
	, par_min_(cp.par_min_)
	, par_max_(cp.par_max_)
	, minima_(cp.minima_)
{ /* nothing */ }

/********************************************************************************************/
/**
 * A standard assignment operator
 *
 * @param cp A copy of another GMultiCriterionParabolaIndividual object
 * @return A constant reference to this object
 */
const GMultiCriterionParabolaIndividual& GMultiCriterionParabolaIndividual::operator=(const GMultiCriterionParabolaIndividual& cp){
	GMultiCriterionParabolaIndividual::load_(&cp);
	return *this;
}

/********************************************************************************************/
/**
 * Loads the data of another GMultiCriterionParabolaIndividual, camouflaged as a GObject.
 *
 * @param cp A copy of another GMultiCriterionParabolaIndividual, camouflaged as a GObject
 */
void GMultiCriterionParabolaIndividual::load_(const GObject* cp)
{
	const GMultiCriterionParabolaIndividual *p_load = GObject::conversion_cast<GMultiCriterionParabolaIndividual>(cp);

	// Load our parent's data ...
	GParameterSet::load_(cp);

	// We do not copy local data, as it doesn't change during the
	// course of the optimization
	// nPar_ = p_load->nPar_;
	// par_min_ = p_load->par_min_;
	// par_max_ = p_load->par_max_;
	// minima_ = p_load->minima_;
}

/********************************************************************************************/
/**
 * Creates a deep clone of this object
 *
 * @return A deep clone of this object, camouflaged as a GObject
 */
GObject* GMultiCriterionParabolaIndividual::clone_() const {
	return new GMultiCriterionParabolaIndividual(*this);
}

/********************************************************************************************/
/**
 * The actual fitness calculation takes place here.
 * @param id The id of the target function (ignored for this function)
 * @return The value of this object
 */
double GMultiCriterionParabolaIndividual::fitnessCalculation(){
	double main_result = 0.; // Will hold the main result
	std::vector<double> parVec; // Will hold the individual parameters

	this->streamline(parVec); // Retrieve the parameters

	// Do the actual calculations. Note that the first calculation
	// counts as the main result and that we can register other,
	// secondary evaluation criteria.
	main_result = GSQUARED(parVec[0] - minima_[0]);
	for(std::size_t i=0; i<parVec.size(); i++) {
		registerSecondaryResult(GSQUARED(parVec[i] - minima_[i]));
	}

	return main_result;
}

/********************************************************************************************/
//////////////////////////////////////////////////////////////////////////////////////////////
/********************************************************************************************/
/**
 * The standard constructor for this class
 *
 * @param cF The name of the configuration file
 */
GMultiCriterionParabolaIndividualFactory::GMultiCriterionParabolaIndividualFactory(const std::string& cF)
	: GIndividualFactoryT<GMultiCriterionParabolaIndividual>(cF)
	, nPar_(2)
	, par_min_(-10.)
	, par_max_(10.)
	, minima_(0)
	, minima_string_("")
{ /* nothing */ }

/********************************************************************************************/
/**
 * The destructor
 */
GMultiCriterionParabolaIndividualFactory::~GMultiCriterionParabolaIndividualFactory()
{ /* nothing */ }

/********************************************************************************************/
/**
 * Necessary initialization work. Here we divide minima_string_ into individual minima
 * and initialize the nPar_ variables.
 */
void GMultiCriterionParabolaIndividualFactory::init_() {
	// Parse the sleep string and break it into timing values
	std::vector<std::string> minimaTokens;
	typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
	boost::char_separator<char> space_sep(" ");
	tokenizer minimaTokenizer(minima_string_, space_sep);
	tokenizer::iterator s;
	std::size_t parabola_counter = 0;
	for(s=minimaTokenizer.begin(); s!=minimaTokenizer.end(); ++s){
		std::cout << "Parabola " << parabola_counter++ << " minimum = " << *s << std::endl;
		minimaTokens.push_back(*s);
	}
	if(minimaTokens.empty()) { // No sleep tokens were provided
		raiseException(
				"In GMultiCriterionParabolaIndividualFactory::init_(): Error!" << std::endl
				<< "You did not provide any minimum settings" << std::endl
		);
	}

	minima_.clear();

	std::vector<std::string>::iterator t;
	for(t=minimaTokens.begin(); t!=minimaTokens.end(); ++t) {
		// Assign the converted minimum
		minima_.push_back(boost::lexical_cast<double>(*t));
	}

	// Initialize the number of parabolas
	nPar_ = minima_.size();
}
/********************************************************************************************/
/**
 * Allows to describe configuration options in derived classes
 */
void GMultiCriterionParabolaIndividualFactory::describeConfigurationOptions_() {
	gpb.registerFileParameter("par_min", par_min_, par_min_);
	gpb.registerFileParameter("par_max", par_max_, par_max_);
	gpb.registerFileParameter("minima", minima_string_, std::string(""));
}

/********************************************************************************************/
/**
 * Creates individuals of the desired type. The argument "id" gives the function a means
 * of detecting how often it has been called before. The id will be incremented for each call.
 * This can e.g. be used to act differently for the first call to this function.
 *
 * @param id The id of the individual to be created
 * @return An individual of the desired type
 */
boost::shared_ptr<GMultiCriterionParabolaIndividual> GMultiCriterionParabolaIndividualFactory::getIndividual_(const std::size_t& id) {
	return boost::shared_ptr<GMultiCriterionParabolaIndividual>(new GMultiCriterionParabolaIndividual(nPar_, par_min_, par_max_, minima_));
}

/********************************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */
