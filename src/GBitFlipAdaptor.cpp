/**
 * \file
 */

/**
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
BOOST_CLASS_EXPORT(GenEvA::GBitFlipAdaptor)

namespace GenEvA
{

  /*************************************************************************/
  /**
   * The default constructor. As we want to enforce a name for adaptors, this
   * constructor is empty and labelled private. 
   */
  GBitFlipAdaptor::GBitFlipAdaptor() throw()
    :GTemplateAdaptor<GenEvA::bit>("GBitFlipAdaptor") // make the compiler happy
  { /* nothing */ }


  /*************************************************************************/
  /**
   * Every adaptor is required to have a name. This is enforced by providing 
   * only constructors that take a name argument.
   *
   * \param name The name to be assigned to this adaptor
   */
  GBitFlipAdaptor::GBitFlipAdaptor(string name)
    :GTemplateAdaptor<GenEvA::bit>(name)
  {
    GDoubleGaussAdaptor *gaussAdaptor = 
      new GDoubleGaussAdaptor(SGM, SGMSGM, MSGM, DEFAULTGDGANAME);
    _mutProb.addAdaptor(gaussAdaptor);
    _mutProb.setBoundaries(0.,false, 1., false);
	
    setMutationProbability(DEFAULTMUTPROB);
    setAllowProbabilityMutation(false);
  }

  /*************************************************************************/
  /**
   * In addition to GBitFlipAdaptor::GBitFlipAdaptor(string name), this constructor
   * also takes an argument, that specifies the probability for the mutation of
   * a bit value.
   *
   * \param prob The probability for a bit-flip
   * \param name The name to be assigned to this adaptor
   */
  GBitFlipAdaptor::GBitFlipAdaptor(double prob, string name)
    :GTemplateAdaptor<GenEvA::bit>(name)
  {
    GDoubleGaussAdaptor *gaussAdaptor = 
      new GDoubleGaussAdaptor(SGM, SGMSGM, MSGM, DEFAULTGDGANAME);
    _mutProb.addAdaptor(gaussAdaptor);
    _mutProb.setBoundaries(0.,false, 1., false);
	
    setMutationProbability(prob);
    setAllowProbabilityMutation(false);
  }

  /*************************************************************************/
  /**
   * A standard copy constructor.
   *
   * \param cp A copy of another GBitFlipAdaptor object
   */
  GBitFlipAdaptor::GBitFlipAdaptor(const GBitFlipAdaptor& cp)
    :GTemplateAdaptor<GenEvA::bit>(cp)
  {
    _mutProb.load(&(cp._mutProb));
    _allowProbabilityMutation = cp._allowProbabilityMutation;
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
   * \param cp A copy of another GBitFlipAdaptor object
   * \return A constant reference to this object
   */
  const GBitFlipAdaptor& GBitFlipAdaptor::operator=(const GBitFlipAdaptor& cp){
    GBitFlipAdaptor::load(&cp);
    return *this;
  }
	
  /*******************************************************************************************/	
  /** 
   * A standard assignment operator for GBitFlipAdaptor objects,
   * 
   * \param cp A copy of another GBitFlipAdaptor object, camouflaged as a GObject
   * \return A constant reference to this object
   */	
  const GObject& GBitFlipAdaptor::operator=(const GObject& cp){
    GBitFlipAdaptor::load(&cp);
    return *this;	
  }

  /*************************************************************************/
  /**
   * Resets the object to its initial state.
   */
  void GBitFlipAdaptor::reset(void){	
    _mutProb.reset();
	
    GDoubleGaussAdaptor *gaussAdaptor = 
      new GDoubleGaussAdaptor(SGM, SGMSGM, MSGM, DEFAULTGDGANAME);
    _mutProb.addAdaptor(gaussAdaptor);
    _mutProb.setBoundaries(0.,false, 1., false);
	
    setMutationProbability(DEFAULTMUTPROB);
    setAllowProbabilityMutation(false);

    GTemplateAdaptor<GenEvA::bit>::reset();
  }

  /*************************************************************************/
  /**
   * Loads the content of another GBitFlipAdaptor, camouflaged as a GObject
   *
   * \param gb A pointer to another GBitFlipAdaptor object, camouflaged as a GObject
   */
  void GBitFlipAdaptor::load(const GObject *gb){
    const GBitFlipAdaptor *gbfa = dynamic_cast<const GBitFlipAdaptor *>(gb);

    // dynamic_cast will emit a NULL pointer, if the conversion failed
    if(!gb){
      gls << "In GBitFlipAdaptor::load() : Conversion error!" << endl
	  << logLevel(CRITICAL);

      exit(1);
    }

    // Check that this object is not accidently assigned to itself. 
    if(gbfa == this){
      gls << "In GBitFlipAdaptor::load(): Error!" << endl
	  << "Tried to assign an object to itself." << endl
	  << logLevel(CRITICAL);

      exit(1);
    }

    // Now we can load the actual data	
    GTemplateAdaptor<GenEvA::bit>::load(gb);
    _mutProb = gbfa->_mutProb;
    _allowProbabilityMutation = gbfa->_allowProbabilityMutation;
  }

  /*************************************************************************/
  /**
   * Creates a deep copy of this object.
   *
   * \return A deep copy of this object
   */
  GObject *GBitFlipAdaptor::clone(void){
    return new GBitFlipAdaptor(*this);
  }

  /*************************************************************************/
  /**
   * Retrieves the current value of the mutation probability
   *
   * \return The current value of the mutation probability
   */
  double GBitFlipAdaptor::getMutationProbability(void){
    return _mutProb.fitness();
  }

  /*************************************************************************/
  /**
   * Sets the mutation probability to a given value. Note that, if the variable
   * _allowProbabilityMutation is set to true, this value will change over time.
   * Use GBitFlipAdaptor::setAllowProbabilityMutation(bool) to disallow adaptions
   * of the mutation probability.
   *
   * \param val The new value of the probability for bit flips
   */
  void GBitFlipAdaptor::setMutationProbability(double val){
    // Adapt the probability, where needed. Needs to be logged.
    if(val < 0.){
      gls << "In GBitFlipAdaptor::setMutationProbability(" << val << ") : WARNING" << endl
	  << "Negative probability. Setting to 0." << endl
	  << logLevel(UNCRITICAL);
      _mutProb = 0.;
      return;
    }
	
    if(val > 1.){
      gls << "In GBitFlipAdaptor::setMutationProbability(" << val << ") : WARNING" << endl
	  << "Probability > 1. Setting to 1." << endl
	  << logLevel(UNCRITICAL);
      _mutProb = 1.;
      return;
    }

    _mutProb = val;
  }

  /*************************************************************************/
  /**
   * The mutation of a GDouble object has a number of parameters that can 
   * be set with this function. See the documentation of the GDouble class
   * for further information. Note that shared_ptr<> implements operator bool,
   * so we can test whether it contains a valid pointer in a simple way.
   *
   * \param sgm Sigma for gauss mutation
   * \param sgmSgm Parameter which determines the amount of evolutionary adaption of sigma
   * \param mSgm The minimal value sigma may assume
   */
  void GBitFlipAdaptor::setInternalMutationParameters(double sgm, double sgmSgm, double mSgm){
    // First get a pointer to the GDoubleGaussAdaptor.
    shared_ptr<GTemplateAdaptor<double> > basePtr = _mutProb.getAdaptor(DEFAULTGDGANAME);
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
   * Reports the internal state of this object.
   * 
   * \param indention The number of white spaces in front of each output line 
   * \return A string containing a report about the inner state of the object
   */
  string GBitFlipAdaptor::assembleReport(uint16_t indention ) const
  {
    ostringstream oss;
    // Do our own reporting
    oss << ws(indention) << "GBitFlipAdaptor: " << this << endl
	<< ws(indention) << "Mutation probability from GDouble::_mutProb:" << endl
        << _mutProb.assembleReport(indention+NINDENTION) << endl
	<< ws(indention) << "-----> Report from parent class GTemplateAdaptor<GenEvA::bit>" << endl 		
	<< GTemplateAdaptor<GenEvA::bit>::assembleReport(indention+NINDENTION) << endl;
    return oss.str();
  }

  /*************************************************************************/
  /**
   * Allow the mutation of the probability mutation with parameter==true (the
   * default), disallow with allowProbabilityMutation=false.
   *
   \param allowProbabilityMutation Determines whether bit flip probability may be mutated
   */
  void GBitFlipAdaptor::setAllowProbabilityMutation(bool allowProbabilityMutation=true){
    _allowProbabilityMutation = allowProbabilityMutation;
  }

  /*************************************************************************/
  /**
   * Retrieve the value of the _allowProbabilityMutation variable.
   *
   * \return The value of the _allowProbabilityMutation; variable
   */
  bool GBitFlipAdaptor::getAllowProbabilityMutation(void) const throw()
  {
    return _allowProbabilityMutation;
  }

  /*************************************************************************/
  /**
   * The mutation probability is implemented as a GDouble. It thus can take
   * care of its own mutation within its boundaries [0.,1.] . We only mutate
   * the mutation probability if _allowProbabilityMutation is set to true.
   */
  void GBitFlipAdaptor::initNewRun(void){
    if(_allowProbabilityMutation) _mutProb.mutate();
  }

  /*************************************************************************/
  /**
   * We want to flip the value only in a given percentage of cases. Thus
   * we calculate a probability between 0 and 1 and compare it with the desired
   * mutation probability. Please note that evenRandom returns a value in the
   * range of [0,1[, so we make a tiny error here.
   *
   * \param value The bit value to be mutated
   */
  void GBitFlipAdaptor::customMutate(GenEvA::bit &value){
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
   * \param value The bit value to be flipped
   */
  void GBitFlipAdaptor::flip(GenEvA::bit &value) const throw()
  {
    value==GenEvA::TRUE ? value = GenEvA::FALSE : value=GenEvA::TRUE;
  }

  /***********************************************************************************/

} /* namespace GenEvA */
