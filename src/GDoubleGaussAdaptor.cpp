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

#include "GDoubleGaussAdaptor.hpp"

// Included here so no conflicts occur. See explanation at
// http://www.boost.org/libs/serialization/doc/special.html#derivedpointers
#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(GenEvA::GDoubleGaussAdaptor)


namespace GenEvA
{
  /*************************************************************************/
  /**
   * The default constructor. We do not want it to be publicly accessible.
   * Hence it is defined private.
   */
  GDoubleGaussAdaptor::GDoubleGaussAdaptor(void) throw()
    :GTemplateAdaptor<double>("GDoubleGaussAdaptor")
  { 
    setSigma(DEFAULTSIGMA);
    setSigmaSigma(DEFAULTSIGMASIGMA, DEFAULTMINSIGMA);
  }


  /*************************************************************************/
  /**
   * The standard constructor. It passes the adaptor's name to the parent class
   * and initialises the internal variables.
   * 
   * \param name The name assigned to this adaptor
   */
  GDoubleGaussAdaptor::GDoubleGaussAdaptor(string name) throw()
    :GTemplateAdaptor<double>(name)
  {
    setSigma(DEFAULTSIGMA);
    setSigmaSigma(DEFAULTSIGMASIGMA, DEFAULTMINSIGMA);
  }

  /*************************************************************************/
  /**
   * In addition to passing the name of the adaptor to the parent class,
   * it is also possible to specify a value for the _sigma parameter in
   * this constructor.
   * 
   * \param sigma The initial value for the _sigma parameter
   * \param name The name assigned to this adaptor
   */
  GDoubleGaussAdaptor::GDoubleGaussAdaptor(double sigma, string name) throw()
    :GTemplateAdaptor<double>(name)
  {
    // These functions do error checks on sigma, so we do not assign
    // the value directly to the private variable.
    setSigma(sigma); 
    setSigmaSigma(DEFAULTSIGMASIGMA, DEFAULTMINSIGMA);
  }

  /*************************************************************************/
  /**
   * This constructor lets a user set all parameters in one go.
   * 
   * \param sigma The initial value for the _sigma parameter
   * \param sigmaSigma The initial value for the _sigmaSigma parameter
   * \param minSigma The minimal value allowed for _sigma
   * \param name The name assigned to this adaptor
   */
  GDoubleGaussAdaptor::GDoubleGaussAdaptor(double sigma, double sigmaSigma, double minSigma, string name) throw()
    :GTemplateAdaptor<double>(name)
  {
    setSigma(sigma);
    setSigmaSigma(sigmaSigma,minSigma);
  }

  /*************************************************************************/
  /**
   * A standard copy constructor. 
   * 
   * \param cp Another GDoubleGaussAdaptor object
   */
  GDoubleGaussAdaptor::GDoubleGaussAdaptor(const GDoubleGaussAdaptor& cp) throw()
    :GTemplateAdaptor<double>(cp)
  {
    setSigma(cp._sigma);
    setSigmaSigma(cp._sigmaSigma,cp._minSigma);
  }

  /*************************************************************************/
  /**
   * The standard destructor. Empty, as we have no local, dynamically 
   * allocated data.  
   */
  GDoubleGaussAdaptor::~GDoubleGaussAdaptor()
  { /* nothing */ }

  /*******************************************************************************************/
  /** 
   * A standard assignment operator for GDoubleGaussAdaptor objects,
   * 
   * \param cp A copy of another GDoubleGaussAdaptor object
   */
  const GDoubleGaussAdaptor& GDoubleGaussAdaptor::operator=(const GDoubleGaussAdaptor& cp){
    GDoubleGaussAdaptor::load(&cp);
    return *this;
  }
	
  /*******************************************************************************************/	
  /** 
   * A standard assignment operator for GDoubleGaussAdaptor objects,
   * 
   * \param cp A copy of another GDoubleGaussAdaptor object, camouflaged as a GObject
   */	
  const GObject& GDoubleGaussAdaptor::operator=(const GObject& cp){
    GDoubleGaussAdaptor::load(&cp);
    return *this;	
  }

  /*************************************************************************/
  /**
   * Resets the object to its initial state.
   */
  void GDoubleGaussAdaptor::reset()
  {
    // First we reset our own values
    _sigma=DEFAULTSIGMA;
    _sigmaSigma=DEFAULTSIGMASIGMA;
    _minSigma=DEFAULTMINSIGMA;
	
    // and then our parent class
    GTemplateAdaptor<double>::reset();
  }

  /*************************************************************************/
  /**
   * This is where the actual mutation of the supplied value takes place.
   * The _sigma retrieved with GDoubleGaussAdaptor::getSigma() might get 
   * mutated itself, if the _sigmaSigma parameter is not 0. The function is
   * declared inline as it will be called very frequently.
   * 
   * \param value The value that is going to be mutated 
   */
  inline void GDoubleGaussAdaptor::customMutate(double &value)
  {
    // adapt the value in situ. Note: this changes
    // the argument of this function
    value += gr.gaussRandom(0.,_sigma);
  }

  /*************************************************************************/
  /**
   * This adaptor allows the evolutionary adaption of _sigma. This allows the
   * algorithm to adapt to changing geometries of the quality surface. The
   * function is declared inline, as it will likely be called often.
   */
  inline void GDoubleGaussAdaptor::initNewRun(void){
    // do we want to adapt _sigma at all ?
    if(_sigmaSigma){ // != 0 ?
      _sigma *= exp(gr.gaussRandom(0.,_sigmaSigma));
      // make sure _sigma doesn't get too small
      if(fabs(_sigma) < _minSigma) _sigma = _minSigma;
    }
  }

  /*************************************************************************/
  /**
   * This function sets the value of the _sigma parameter. If the value
   * does not make sense, it will be adapted to a useful value. A log
   * message is emitted in this case.
   * 
   * \param sigma The initial value for the _sigma parameter
   */
  void GDoubleGaussAdaptor::setSigma(double sigma) throw()
  {
    // A value of sigma smaller or equal 0 is not useful. Adapt and log.
    if(sigma <= 0.)
      {
	gls << "In GDoubleGaussAdaptor::setSigma(double): WARNING" << endl
	    << "Bad value for sigma given: " << sigma << endl
	    << "The value will be adapted to the default value " << DEFAULTSIGMA << endl
	    << logLevel(UNCRITICAL);

	_sigma = DEFAULTSIGMA;
	return;
      }

    _sigma = sigma;
  }

  /*************************************************************************/
  /**
   * This function sets the values of the _sigmaSigma parameter and the
   * minimal value allowed for _sigma. Note that there will only be adaptation 
   * of _sigma, if the user specifies a value for _sigmaSigma other than 0.
   * 
   * If either sigmaSigma or minSigma do not have useful values, they will be adapted
   * and a log message will be emiited.
   * 
   * \param sigmaSigma The initial value for the _sigmaSigma parameter
   * \param minSigma The minimal value allowed for _sigma
   */
  void GDoubleGaussAdaptor::setSigmaSigma(double sigmaSigma, double minSigma) throw()
  {
    double tmpSigmaSigma = sigmaSigma, tmpMinSigma = minSigma;
	
    // A value of sigmaSigma smaller than 0. is not useful.
    // Note that a sigmaSigma of 0 indicates that no adaption of the
    // stepwidth is intended.
    if(sigmaSigma < 0.)
      {
	gls << "In GDoubleGaussAdaptor::setSigmaSigma(double, double): WARNING" << endl
	    << "Bad value for sigmaSigma given: " << sigmaSigma << endl
	    << "The value will be adapted to the default value " << DEFAULTSIGMASIGMA << endl
	    << logLevel(UNCRITICAL);
		    
	tmpSigmaSigma = DEFAULTSIGMASIGMA;
      }

    // A minimum allowed value for _sigma <= 0 is not useful. Note that
    // this way also 0 is forbidden as value, as no progress would be 
    // possible anymore in the optimisation.
    if(minSigma <= 0.)
      {
	gls << "In GDoubleGaussAdaptor::setSigmaSigma(double, double): WARNING" << endl
	    << "Bad value for minSigma given: " << minSigma << endl
	    << "The value will be adapted to the default value " << DEFAULTMINSIGMA << endl
	    << logLevel(UNCRITICAL);
		    
	tmpMinSigma = DEFAULTMINSIGMA;
      }

    _sigmaSigma = tmpSigmaSigma;
    setMinSigma(tmpMinSigma);
  }

  /*************************************************************************/
  /**
   * Retrieves the current value of _sigma.
   * 
   * \return The current value of _sigma
   */
  double GDoubleGaussAdaptor::getSigma(void) const throw()
  {
    return _sigma;
  }

  /*************************************************************************/
  /**
   * Retrieves the current value of _sigmaSigma .
   * 
   * \return The value of _sigmaSigma
   */
  double GDoubleGaussAdaptor::getSigmaSigma(void) const throw()
  {
    return _sigmaSigma;
  }

  /*************************************************************************/
  /**
   * Allows to set a value for the minimally allowed _sigma. If minSigma does not
   * have a useful value, it will be reset to the default value and a log
   * message will be emitted.
   * 
   * \param The minimally allowed value for _sigma
   */
  void GDoubleGaussAdaptor::setMinSigma(double minSigma) throw()
  {
    // A value of minSigma <= 0. is not useful
    if(minSigma <= 0.)
      {
	gls << "In GDoubleGaussAdaptor::setMinSigma(double): WARNING" << endl
	    << "Bad value for minSigma given: " << minSigma << endl
	    << "The value will be adapted to the default value " << DEFAULTMINSIGMA << endl
	    << logLevel(UNCRITICAL);
		
	_minSigma = DEFAULTMINSIGMA;
	return;
      }

    _minSigma=minSigma;
  }

  /*************************************************************************/
  /**
   * Retrieves the minimally allowed value of _sigma
   * 
   * \return The minimally allowed value for _sigma
   */
  double GDoubleGaussAdaptor::getMinSigma(void) const throw()
  {
    return _minSigma;
  }

  /*************************************************************************/
  /**
   * Convenience function that lets users set all relevant parameters of this class
   * at once.
   * \param sigma The initial value for the _sigma parameter
   * \param sigmaSigma The initial value for the _sigmaSigma parameter
   * \param minSigma The minimal value allowed for _sigma 
   */
  void GDoubleGaussAdaptor::setAll(double sigma, double sigmaSigma, double minSigma){
    GDoubleGaussAdaptor::setSigma(sigma);
    GDoubleGaussAdaptor::setSigmaSigma(sigmaSigma,minSigma);
  }

  /*************************************************************************/
  /**
   * This function creates a deep copy of this object
   * 
   * \return A deep copy of this object
   */
  GObject *GDoubleGaussAdaptor::clone(void){
    return new GDoubleGaussAdaptor(*this);
  }

  /*************************************************************************/
  /**
   * This function loads the data of another GDoubleGaussAdaptor, camouflaged
   * as a GObject.
   * 
   * \param A copy of another GDoubleGaussAdaptor, camouflaged as a GObject
   */
  void GDoubleGaussAdaptor::load(const GObject *gb){
    const GDoubleGaussAdaptor *gdga = dynamic_cast<const GDoubleGaussAdaptor *>(gb);

    // dynamic_cast will emit a NULL pointer, if the conversion failed
    if(!gdga){
      gls << "In GDoubleGaussAdaptor::load() : Conversion error!" << endl
	  << logLevel(CRITICAL);

      exit(1);
    }

    // Check that this object is not accidently assigned to itself. 
    if(gdga == this){
      gls << "In GDoubleGaussAdaptor::load(): Error!" << endl
	  << "Tried to assign an object to itself." << endl
	  << logLevel(CRITICAL);

      exit(1);
    }

    // Now we can load the actual data	
    GTemplateAdaptor<double>::load(gb);
    _sigma = gdga->_sigma;
    _sigmaSigma = gdga->_sigmaSigma;
    _minSigma = gdga->_minSigma;
  }

  /*************************************************************************/
  /**
   * Assembles a report about the inner state of this object.
   * 
   * \param indention The number of white spaces in front of each output line 
   * \return A string containing a report about the inner state of the object
   */
  string GDoubleGaussAdaptor::assembleReport(uint16_t indention) const
  {
    ostringstream oss;
    oss << ws(indention) << "GDoubleGaussAdaptor : " << this << endl
	<< ws(indention) << "_sigma =      " << this->getSigma() << endl
	<< ws(indention) << "_sigmaSigma = " << this->getSigmaSigma() << endl
	<< ws(indention) << "_minSigma =   " << this->getMinSigma() << endl
	<< ws(indention) << "-----> Report from parent class GTemplateAdaptor<double> : " << endl
	<< GTemplateAdaptor<double>::assembleReport(indention+NINDENTION) << endl;

    return oss.str();
  }

  /*************************************************************************/

} /* namespace GenEvA */
