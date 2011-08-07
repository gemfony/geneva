/**
 * @file GParaboloidIndividual.hpp
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

#include "GParaboloidIndividual.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GParaboloidIndividual)

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
GParaboloidIndividual::GParaboloidIndividual()
{ /* nothing */ }

/********************************************************************************************/
/**
 * The destructor
 */
GParaboloidIndividual::~GParaboloidIndividual()
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
GParaboloidIndividual::GParaboloidIndividual(
	const std::size_t& nPar
	, const double& par_min
	, const double& par_max
)
	: nPar_(nPar)
	, par_min_(par_min)
 	, par_max_(par_max)
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
 * @param cp A copy of another GParaboloidIndividual
 */
GParaboloidIndividual::GParaboloidIndividual(const GParaboloidIndividual& cp)
	: GParameterSet(cp)
	, nPar_(cp.nPar_)
	, par_min_(cp.par_min_)
	, par_max_(cp.par_max_)
{ /* nothing */ }

/********************************************************************************************/
/**
 * A standard assignment operator
 *
 * @param cp A copy of another GParaboloidIndividual object
 * @return A constant reference to this object
 */
const GParaboloidIndividual& GParaboloidIndividual::operator=(const GParaboloidIndividual& cp){
	GParaboloidIndividual::load_(&cp);
	return *this;
}

/********************************************************************************************/
/**
 * Loads the data of another GParaboloidIndividual, camouflaged as a GObject.
 *
 * @param cp A copy of another GParaboloidIndividual, camouflaged as a GObject
 */
void GParaboloidIndividual::load_(const GObject* cp)
{
	const GParaboloidIndividual *p_load = GObject::conversion_cast<GParaboloidIndividual>(cp);

	// Load our parent's data ...
	GParameterSet::load_(cp);

	// ... and then our local data
	nPar_ = p_load->nPar_;
	par_min_ = p_load->par_min_;
	par_max_ = p_load->par_max_;
}

/********************************************************************************************/
/**
 * Creates a deep clone of this object
 *
 * @return A deep clone of this object, camouflaged as a GObject
 */
GObject* GParaboloidIndividual::clone_() const {
	return new GParaboloidIndividual(*this);
}

/********************************************************************************************/
/**
 * The actual fitness calculation takes place here.
 * @param id The id of the target function (ignored for this function)
 * @return The value of this object
 */
double GParaboloidIndividual::fitnessCalculation(){
	double result = 0.; // Will hold the result
	std::vector<double> parVec; // Will hold the parameters

	this->streamline(parVec); // Retrieve the parameters

	// Do the actual calculation
	for(std::size_t i=0; i<parVec.size(); i++) {
		result += parVec[i]*parVec[i];
	}

	return result;
}

/********************************************************************************************/
//////////////////////////////////////////////////////////////////////////////////////////////
/********************************************************************************************/
/**
 * The standard constructor for this class
 *
 * @param cF The name of the configuration file
 */
GParaboloidIndividualFactory::GParaboloidIndividualFactory(const std::string& cF)
	: GIndividualFactoryT<GParaboloidIndividual>(cF)
	, nPar_(2)
	, par_min_(-10.)
	, par_max_(10.)
{ /* nothing */ }

/********************************************************************************************/
/**
 * The destructor
 */
GParaboloidIndividualFactory::~GParaboloidIndividualFactory()
{ /* nothing */ }

/********************************************************************************************/
/**
 * Allows to describe configuration options in derived classes
 */
void GParaboloidIndividualFactory::describeConfigurationOptions_() {
	gpb.registerFileParameter("nPar", nPar_, nPar_);
	gpb.registerFileParameter("par_min", par_min_, par_min_);
	gpb.registerFileParameter("par_max", par_max_, par_max_);
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
boost::shared_ptr<GParaboloidIndividual> GParaboloidIndividualFactory::getIndividual_(const std::size_t& id) {
	return boost::shared_ptr<GParaboloidIndividual>(new GParaboloidIndividual(nPar_, par_min_, par_max_));
}

/********************************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */
