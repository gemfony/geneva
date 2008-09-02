/**
 * @file GBitFlipAdaptor.cpp
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

#include "GBitFlipAdaptor.hpp"

/**
 * Included here so no conflicts occur. See explanation at
 * http://www.boost.org/libs/serialization/doc/special.html#derivedpointers
 */
#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(Gem::GenEvA::GBitFlipAdaptor)

namespace Gem
{
	namespace GenEvA
	{

		/*************************************************************************/
		/**
		 * The default constructor. As we want to enforce a name for adaptors, this
		 * constructor is empty and labeled private. It is needed for the serialization
		 * of this object, though.
		 */
		GBitFlipAdaptor::GBitFlipAdaptor() throw()
			:GAdaptorT<GenEvA::bit>("GBitFlipAdaptor"),
			 mutProb_(DEFAULTMUTPROB,0.,1.),
			 allowProbabilityMutation_(false)
		{ /* nothing */ }

		/*************************************************************************/
		/**
		 * Every adaptor is required to have a name. This is enforced by providing
		 * only constructors that take a name argument.
		 *
		 * @param name The name to be assigned to this adaptor
		 */
		GBitFlipAdaptor::GBitFlipAdaptor(std::string name)
			:GAdaptorT<GenEvA::bit>(name),
			 mutProb_(DEFAULTMUTPROB,0.,1.), // probability is in the range [0:1]
			 allowProbabilityMutation_(false)
		{
			boost::shared_ptr<GAdaptorT<double> >
				gaussAdaptor(new GDoubleGaussAdaptor(SGM, SGMSGM, MSGM, DEFAULTGDGANAME));
			mutProb_.addAdaptor(gaussAdaptor);
		}

		/*************************************************************************/
		/**
		 * In addition to GBitFlipAdaptor::GBitFlipAdaptor(std::string name), this constructor
		 * also takes an argument, that specifies the probability for the mutation of
		 * a bit value. Note that, in order to use probability mutation, you will still
		 * need to explicitly allow this feature.
		 *
		 * @param prob The probability for a bit-flip
		 * @param name The name to be assigned to this adaptor
		 */
		GBitFlipAdaptor::GBitFlipAdaptor(double prob, std::string name)
			:GAdaptorT<GenEvA::bit>(name),
			 mutProb_(prob,0.,1.), // probability is in the range [0:1]
			 allowProbabilityMutation_(true)
		{
			boost::shared_ptr<GAdaptorT<double> >
				gaussAdaptor(new GDoubleGaussAdaptor(SGM, SGMSGM, MSGM, DEFAULTGDGANAME));
			mutProb_.addAdaptor(gaussAdaptor);
		}

		/*************************************************************************/
		/**
		 * A standard copy constructor.
		 *
		 * @param cp A copy of another GBitFlipAdaptor object
		 */
		GBitFlipAdaptor::GBitFlipAdaptor(const GBitFlipAdaptor& cp)
			:GAdaptorT<GenEvA::bit>(cp),
			 mutProb_(cp.mutProb_),
			 allowProbabilityMutation_(cp.allowProbabilityMutation_)
		{ /* nothing */ }

		/*************************************************************************/
		/**
		 * A standard destructor. Internally in GDouble, the GDoubleGaussAdaptor is
		 * wrapped into a shared_ptr, which takes care of its deletion. So we
		 * have no data to take care of here.
		 */
		GBitFlipAdaptor::~GBitFlipAdaptor()
		{ /* nothing */}

		/*******************************************************************************************/
		/**
		 * A standard assignment operator for GBitFlipAdaptor objects.
		 *
		 * @param cp A copy of another GBitFlipAdaptor object
		 * @return A constant reference to this object
		 */
		const GBitFlipAdaptor& GBitFlipAdaptor::operator=(const GBitFlipAdaptor& cp) {
			GBitFlipAdaptor::load(&cp);
			return *this;
		}

		/*************************************************************************/
		/**
		 * Loads the content of another GBitFlipAdaptor, camouflaged as a GObject
		 *
		 * @param cp A pointer to another GBitFlipAdaptor object, camouflaged as a GObject
		 */
		void GBitFlipAdaptor::load(const GObject *cp) {
			const GBitFlipAdaptor *gbfa = this->checkedConversion(cp, this);

			// Now we can load our parent class'es data ...
			GAdaptorT<GenEvA::bit>::load(cp);

			// ... and then our own
			mutProb_ = gbfa->mutProb_;
			allowProbabilityMutation_ = gbfa->allowProbabilityMutation_;
		}

		/*************************************************************************/
		/**
		 * Creates a deep copy of this object.
		 *
		 * @return A deep copy of this object
		 */
		GObject *GBitFlipAdaptor::clone() {
			return new GBitFlipAdaptor(*this);
		}

		/*************************************************************************/
		/**
		 * Retrieves the current value of the mutation probability
		 *
		 * @return The current value of the mutation probability
		 */
		double GBitFlipAdaptor::getMutationProbability() {
			return mutProb_.value();
		}

		/*************************************************************************/
		/**
		 * Sets the mutation probability to a given value. Note that, if the variable
		 * allowProbabilityMutation_ is set to true, this value will change over time.
		 * Use GBitFlipAdaptor::setAllowProbabilityMutation(false) to disallow adaptions
		 * of the mutation probability (the default). This function will throw if the
		 * probability is not in the allowed range.
		 *
		 * @param val The new value of the probability for bit flips
		 */
		void GBitFlipAdaptor::setMutationProbability(double probability) {
			// Check the supplied probability value
			if(probability < 0. || probability > 1.) {
				std::ostringstream error;
				error << "In GBitFlipAdaptor::setMutationProbability(double) : Error!" << std::endl
					  << "Bad probability value given: " << probability << std::endl;

				LOGGER.log(error.str(), Gem::GLogFramework::CRITICAL);

				// throw an exception. Add some information so that if the exception
				// is caught through a base object, no information is lost.
				throw geneva_bad_mutation_probability() << error_string(error.str());
			}

			mutProb_ = probability;
		}

		/*************************************************************************/
		/**
		 * The mutation of a GDouble object has a number of parameters that can
		 * be set with this function. See the documentation of the GDouble class
		 * for further information. Note that shared_ptr<> implements operator bool,
		 * so we can test whether it contains a valid pointer in a simple way.
		 *
		 * @param sgm Sigma for gauss mutation
		 * @param sgmSgm Parameter which determines the amount of evolutionary adaption of sigma
		 * @param mSgm The minimal value sigma may assume
		 */
		void GBitFlipAdaptor::setMutationParameters(double sgm, double sgmSgm, double mSgm) {
			boost::shared_ptr<GDoubleGaussAdaptor> gaussAdaptor =
				mutProb_.adaptor_cast<GDoubleGaussAdaptor>(DEFAULTGDGANAME);

			// Then set the values as requested.
			gaussAdaptor->setAll(sgm,sgmSgm,mSgm);
		}

		/*************************************************************************/
		/**
		 * Allow the mutation of the probability mutation with parameter==true (the
		 * default), disallow with allowProbabilityMutation=false.
		 *
		 @param allowProbabilityMutation Determines whether bit flip probability may be mutated
		 */
		void GBitFlipAdaptor::setAllowProbabilityMutation(bool allowProbabilityMutation=true) {
			allowProbabilityMutation_ = allowProbabilityMutation;
		}

		/*************************************************************************/
		/**
		 * Retrieve the value of the allowProbabilityMutation_ variable.
		 *
		 * @return The value of the allowProbabilityMutation_; variable
		 */
		bool GBitFlipAdaptor::getAllowProbabilityMutation() const throw()
		{
			return allowProbabilityMutation_;
		}

		/*************************************************************************/
		/**
		 * The mutation probability is implemented as a GDouble. It thus can take
		 * care of its own mutation within its boundaries [0.,1.] . We only mutate
		 * the mutation probability if allowProbabilityMutation_ is set to true.
		 */
		void GBitFlipAdaptor::initNewRun() {
			if(allowProbabilityMutation_) mutProb_.mutate();
		}

		/*************************************************************************/
		/**
		 * We want to flip the value only in a given percentage of cases. Thus
		 * we calculate a probability between 0 and 1 and compare it with the desired
		 * mutation probability. Please note that evenRandom returns a value in the
		 * range of [0,1[, so we make a tiny error here.
		 *
		 * @param value The bit value to be mutated
		 */
		void GBitFlipAdaptor::customMutations(Gem::GenEvA::bit &value){
			double probe = gr.evenRandom(0.,1.);
			if(probe < mutProb_.value()) this->flip(value);
		}

		/*************************************************************************/
		/**
		 * This private function simply flips a boolean to the opposite value.
		 * Note that this uses GenEvA's own implementation of a boolean (an enum
		 * defined in GEnums.h - see there for an explanation of why our own bit
		 * value is necessary).
		 *
		 * @param value The bit value to be flipped
		 */
		void GBitFlipAdaptor::flip(GenEvA::bit &value) const throw()
		{
			value==Gem::GenEvA::G_TRUE ? value = Gem::GenEvA::G_FALSE : value=Gem::GenEvA::G_TRUE;
		}

		/***********************************************************************************/

	} /* namespace GenEvA */
} /* namespace Gem */
