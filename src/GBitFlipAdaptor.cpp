/**
 * @file
 */

/* GBitFlipAdaptor.cpp
 *
 * Copyright (C) 2004-2008 Dr. Ruediger Berlich
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
   * constructor is empty and labelled private. It is needed for the serialization
   * of this object, though.
   */
  GBitFlipAdaptor::GBitFlipAdaptor() throw()
    :GAdaptorT<GenEvA::bit>("GBitFlipAdaptor"),
     mutProb_(DEFAULTMUTPROB),
     allowProbabilityMutation_(false)// make the compiler happy
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
    mutProb_(DEFAULTMUTPROB),
    allowProbabilityMutation_(false)
  {
	mutProb_.setBoundaries(0.,BNDISCLOSED, 1., BNDISCLOSED);
	GDoubleGaussAdaptor *gaussAdaptor =
    new GDoubleGaussAdaptor(SGM, SGMSGM, MSGM, DEFAULTGDGANAME);
    mutProb_.addAdaptor(gaussAdaptor);
  }

  /*************************************************************************/
  /**
   * In addition to GBitFlipAdaptor::GBitFlipAdaptor(std::string name), this constructor
   * also takes an argument, that specifies the probability for the mutation of
   * a bit value. As this probability is explicitly set, it is assumed that
   * probability mutations are welcome and are allowed. allowProbabilityMutation_
   * is set accordingly.
   *
   * @param prob The probability for a bit-flip
   * @param name The name to be assigned to this adaptor
   */
  GBitFlipAdaptor::GBitFlipAdaptor(double prob, std::string name)
    :GAdaptorT<GenEvA::bit>(name),
     mutProb_(prob),
     allowProbabilityMutation_(true)
  {
	mutProb_.setBoundaries(0.,BNDISCLOSED, 1., BNDISCLOSED);
    GDoubleGaussAdaptor *gaussAdaptor =
      new GDoubleGaussAdaptor(SGM, SGMSGM, MSGM, DEFAULTGDGANAME);
    mutProb_.addAdaptor(gaussAdaptor);
  }

  /*************************************************************************/
  /**
   * A standard copy constructor.
   *
   * @param cp A copy of another GBitFlipAdaptor object
   */
  GBitFlipAdaptor::GBitFlipAdaptor(const GBitFlipAdaptor& cp)
    :GAdaptorT<GenEvA::bit>(cp)
  {
    mutProb_.load(&(cp.mutProb_));
    allowProbabilityMutation_ = cp.allowProbabilityMutation_;
  }

  /*************************************************************************/
  /**
   * A standard destructor. Internally in GDouble, the GDoubleGaussAdaptor is
   * wrapped into a shared_ptr, which takes care of its deletion. So we
   * have no data to take care of.
   */
  GBitFlipAdaptor::~GBitFlipAdaptor()
  { /* nothing */ }

  /*******************************************************************************************/
  /**
   * A standard assignment operator for GBitFlipAdaptor objects,
   *
   * @param cp A copy of another GBitFlipAdaptor object
   * @return A constant reference to this object
   */
  const GBitFlipAdaptor& GBitFlipAdaptor::operator=(const GBitFlipAdaptor& cp){
    GBitFlipAdaptor::load(&cp);
    return *this;
  }

  /*************************************************************************/
  /**
   * Resets the object to its initial state.
   */
  void GBitFlipAdaptor::reset(){
    mutProb_.reset();

    GDoubleGaussAdaptor *gaussAdaptor =
      new GDoubleGaussAdaptor(SGM, SGMSGM, MSGM, DEFAULTGDGANAME);
    mutProb_.addAdaptor(gaussAdaptor);
    mutProb_.setBoundaries(0.,false, 1., false);

    setMutationProbability(DEFAULTMUTPROB);
    setAllowProbabilityMutation(false);

    GAdaptorT<GenEvA::bit>::reset();
  }

  /*************************************************************************/
  /**
   * Loads the content of another GBitFlipAdaptor, camouflaged as a GObject
   *
   * @param cp A pointer to another GBitFlipAdaptor object, camouflaged as a GObject
   */
  void GBitFlipAdaptor::load(const GObject *cp){
    const GBitFlipAdaptor *gbfa = this->checkedConversion<GBitFlipAdaptor>(cp, this);

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
  GObject *GBitFlipAdaptor::clone(){
    return new GBitFlipAdaptor(*this);
  }

  /*************************************************************************/
  /**
   * Retrieves the current value of the mutation probability
   *
   * @return The current value of the mutation probability
   */
  double GBitFlipAdaptor::getMutationProbability(){
    return mutProb_.fitness();
  }

  /*************************************************************************/
  /**
   * Sets the mutation probability to a given value. Note that, if the variable
   * allowProbabilityMutation_ is set to true, this value will change over time.
   * Use GBitFlipAdaptor::setAllowProbabilityMutation(bool) to disallow adaptions
   * of the mutation probability.
   *
   * @param val The new value of the probability for bit flips
   */
  void GBitFlipAdaptor::setMutationProbability(double val){
    // Adapt the probability, where needed. Needs to be logged.
    if(val < 0.){
      gls << "In GBitFlipAdaptor::setMutationProbability(" << val << ") : WARNING" << endl
	  << "Negative probability. Setting to 0." << endl
	  << logLevel(UNCRITICAL);
      mutProb_ = 0.;
      return;
    }

    if(val > 1.){
      gls << "In GBitFlipAdaptor::setMutationProbability(" << val << ") : WARNING" << endl
	  << "Probability > 1. Setting to 1." << endl
	  << logLevel(UNCRITICAL);
      mutProb_ = 1.;
      return;
    }

    mutProb_ = val;
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
  void GBitFlipAdaptor::setInternalMutationParameters(double sgm, double sgmSgm, double mSgm){
    // First get a pointer to the GDoubleGaussAdaptor.
    shared_ptr<GAdaptorT<double> > basePtr = mutProb_.getAdaptor(DEFAULTGDGANAME);
    if(!basePtr){ // This should not be!
      gls << "In GBitFlipAdaptor::setInternalMutationParameters(): Error!" << endl
	  << "Adaptor with name " << DEFAULTGDGANAME << " was not found" << endl
	  << logLevel(CRITICAL);

      exit(1);
    }

    GDoubleGaussAdaptor *gaussAdaptor = dynamic_cast<GDoubleGaussAdaptor *>(basePtr.get());
    if(!gaussAdaptor){
      gls << "In GBitFlipAdaptor::setInternalMutationParameters(): Conversion error!" << endl
	  << logLevel(CRITICAL);

      exit(1);
    }

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
  void GBitFlipAdaptor::setAllowProbabilityMutation(bool allowProbabilityMutation=true){
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
  void GBitFlipAdaptor::initNewRun(){
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
  void GBitFlipAdaptor::customeMutations(GenEvA::bit &value){
    double probe = gr.evenRandom(0.,1.);
    if(probe < getMutationProbability()) GBitFlipAdaptor::flip(value);
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
    value==GenEvA::TRUE ? value = GenEvA::FALSE : value=GenEvA::TRUE;
  }

  /***********************************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */
