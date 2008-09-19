/**
 * @file GIntFlipAdaptorT.hpp
 */

/* Copyright (C) 2004-2008 Dr. Ruediger Berlich
 * Copyright (C) 2007-2008 Forschungszentrum Karlsruhe GmbH
 *
 * This file is part of Geneva, Gemfony scientific's optimization library.
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
 */

// Standard headers go here
#include <cmath>
#include <string>
#include <sstream>
#include <utility> // For std::pair

// Boost headers go here

#include <boost/version.hpp>

#if BOOST_VERSION < 103600
#error "Error: Boost should at least have version 1.36 !"
#endif /* BOOST_VERSION */

#include <boost/cstdint.hpp>
#include <boost/limits.hpp>
#include <boost/cast.hpp>

#ifndef GINTFLIPADAPTORT_HPP_
#define GINTFLIPADAPTORT_HPP_

// GenEvA headers go here

#include "GAdaptorT.hpp"
#include "GBoundedDouble.hpp"
#include "GObject.hpp"
#include "GRandom.hpp"
#include "GLogger.hpp"
#include "GenevaExceptions.hpp"
#include "GEnums.hpp"

namespace Gem {
namespace GenEvA {

const std::string GINTFLIPADAPTORSTANDARDNAME = "GIntFlipAdaptorT"; ///< The designated name of this adaptor

/************************************************************************************************/
/**
 * GIntFlipAdaptorT represents an adaptor used for the mutation of integer
 * types, by flipping an integer number to the next larger or smaller number
 * with a given probability. The integer type used needs to be specified as
 * a template parameter. Note that a specialization of this class, as defined
 * in GIntFlipAdaptorT.cpp, allows to deal with booleans instead of "standard"
 * integer types.
 */
template<typename int_type>
class GIntFlipAdaptorT:
	public GAdaptorT<int_type>
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int version) {
		using boost::serialization::make_nvp;
		ar & make_nvp("GAdaptorT", boost::serialization::base_object<GAdaptorT<int_type> >(*this));
		ar & make_nvp("mutProb_", mutProb_);
	}
	///////////////////////////////////////////////////////////////////////

public:
	/********************************************************************************************/
	/**
	 * The standard constructor. It passes the adaptor's standard name to the
	 * parent class and initializes the internal variables.
	 */
	GIntFlipAdaptorT()
		:GAdaptorT<int_type> (GINTFLIPADAPTORSTANDARDNAME),
		 mutProb_(DEFAULTMUTPROB, 0., 1.) // probability is in the range [0:1[
	{
		boost::shared_ptr<GAdaptorT<double> > gaussAdaptor(new GDoubleGaussAdaptor(DEFAULTSIGMA, DEFAULTSIGMASIGMA,
				                                                                   DEFAULTMINSIGMA, DEFAULTMAXSIGMA));
		mutProb_.addAdaptor(gaussAdaptor);
	}

	/********************************************************************************************/
	/**
	 * This constructor takes an argument, that specifies the (initial) probability
	 * for the mutation of a bit value.
	 *
	 * @param prob The probability for a bit-flip
	 */
	explicit GIntFlipAdaptorT(const double& prob)
		:GAdaptorT<int_type>(GINTFLIPADAPTORSTANDARDNAME),
		 mutProb_(prob, 0., 1.) // probability is in the range [0:1]
	{
		boost::shared_ptr<GAdaptorT<double> > gaussAdaptor(new GDoubleGaussAdaptor(DEFAULTSIGMA, DEFAULTSIGMASIGMA,
																				   DEFAULTMINSIGMA, DEFAULTMAXSIGMA));
		mutProb_.addAdaptor(gaussAdaptor);
	}

	/********************************************************************************************/
	/**
	 * A standard copy constructor.
	 *
	 * @param cp Another GIntFlipAdaptorT object
	 */
	GIntFlipAdaptorT(const GIntFlipAdaptorT<int_type>& cp)
		:GAdaptorT<int_type>(cp),
		 mutProb_(cp.mutProb_)
	{ /* nothing */	}

	/********************************************************************************************/
	/**
	 * The standard destructor. Empty, as we have no local, dynamically
	 * allocated data.
	 */
	~GIntFlipAdaptorT()
	{ /* nothing */ }

	/********************************************************************************************/
	/**
	 * A standard assignment operator for GIntFlipAdaptorT objects,
	 *
	 * @param cp A copy of another GIntFlipAdaptorT object
	 */
	const GIntFlipAdaptorT<int_type>& operator=(const GIntFlipAdaptorT<int_type>& cp)
	{
		GIntFlipAdaptorT<int_type>::load(&cp);
		return *this;
	}

	/********************************************************************************************/
	/**
	 * This function loads the data of another GIntFlipAdaptorT, camouflaged as a GObject.
	 *
	 * @param A copy of another GIntFlipAdaptorT, camouflaged as a GObject
	 */
	void load(const GObject *cp)
	{
		// Convert GObject pointer to local format
		const GIntFlipAdaptorT<int_type> *gifa = this->conversion_cast(cp, this);

		// Load the data of our parent class ...
		GAdaptorT<int_type>::load(cp);

		// ... and then our own
		mutProb_ = gifa->mutProb_;
	}


	/********************************************************************************************/
	/**
	 * This function creates a deep copy of this object
	 *
	 * @return A deep copy of this object
	 */
	GObject *clone()
	{
		return new GIntFlipAdaptorT<int_type>(*this);
	}


	/********************************************************************************************/
	/**
	 * Retrieves the current value of the mutation probability
	 *
	 * @return The current value of the mutation probability
	 */
	double getMutationProbability() {
		return mutProb_.value();
	}

	/*************************************************************************/
	/**
	 * Sets the mutation probability to a given value. This function will throw
	 * if the probability is not in the allowed range.
	 *
	 * @param val The new value of the probability for integer flips
	 */
	void setMutationProbability(const double& probability) {
		// Check the supplied probability value
		if(probability < 0. || probability > 1.) {
			std::ostringstream error;
			error << "In GIntFlipAdaptor::setMutationProbability(double) : Error!" << std::endl
				  << "Bad probability value given: " << probability << std::endl;

			// throw an exception. Add some information so that if the exception
			// is caught through a base object, no information is lost.
			throw geneva_error_condition(error.str());
		}

		mutProb_ = probability;
	}

	/********************************************************************************************/
	/**
	 * The mutation of a GDouble object has a number of parameters that can
	 * be set with this function. See the documentation of the GDouble class
	 * for further information.
	 *
	 * @param sgm Sigma for gauss mutation
	 * @param sgmSgm Parameter which determines the amount of evolutionary adaption of sigma
	 * @param minSgm The minimal value sigma may assume
	 * @param maxSgm The maximim value sigma may assume
	 */
	void setMutationParameters(const double& sgm, const double& sgmSgm,
							   const double& minSgm, const double& maxSgm) {
		boost::shared_ptr<GDoubleGaussAdaptor> gaussAdaptor
			= mutProb_.adaptor_cast<GDoubleGaussAdaptor>(GDoubleGaussAdaptor::adaptorName());
		// Then set the values as requested.
		gaussAdaptor->setAll(sgm,sgmSgm,minSgm,maxSgm);
	}

	/********************************************************************************************/
	/**
	 * Returns the standard name of a GIntFlipAdaptorT
	 *
	 * @return The name assigned to adaptors of this type
	 */
	static std::string adaptorName() throw() {
		return GINTFLIPADAPTORSTANDARDNAME;
	}

protected:
	/********************************************************************************************/
	/**
	 * The mutation probability is implemented as a GDouble. It thus can take
	 * care of its own mutation within its boundaries [0.,1.] .
	 */
	void adaptMutation() {
		mutProb_.mutate();
	}

	/********************************************************************************************/
	/**
	 * We want to flip the value only in a given percentage of cases. Thus
	 * we calculate a probability between 0 and 1 and compare it with the desired
	 * mutation probability. Please note that evenRandom returns a value in the
	 * range of [0,1[, so we make a tiny error here. This function assumes
	 * an integer type. It hence flips the value up or down. A specialization
	 * for booleans is provided in GIntFlipAdaptorT.cpp .
	 *
	 * @param value The bit value to be mutated
	 */
	virtual void customMutations(int_type& value) {
		double probe = this->gr.evenRandom(0.,1.);
		if(probe < mutProb_.value()) {
			bool up = this->gr.boolRandom();
			if(up){
#if defined (CHECKOVERFLOWS) || defined (DEBUG)
				if(std::numeric_limits<int_type>::max() == value) value -= 1;
				else value += 1;
#else
				value += 1;
#endif
			}
			else {
#if defined (CHECKOVERFLOWS) || defined (DEBUG)
				if(std::numeric_limits<int_type>::min() == value) value += 1;
				else value -= 1;
#else
				value -= 1;
#endif
			}
		}
	}

private:
	/********************************************************************************************/

	GBoundedDouble mutProb_; ///< internal representation of the mutation probability
};

// Declaration of some specializations
template<> void GIntFlipAdaptorT<bool>::customMutations(bool&);
template<> void GIntFlipAdaptorT<short>::customMutations(short&);
template<> void GIntFlipAdaptorT<unsigned short>::customMutations(unsigned short&);
template<> void GIntFlipAdaptorT<boost::uint8_t>::customMutations(boost::uint8_t&);
template<> void GIntFlipAdaptorT<boost::int8_t>::customMutations(boost::int8_t&);

} /* namespace GenEvA */
} /* namespace Gem */

#endif /* GINTFLIPADAPTORT_HPP_ */
