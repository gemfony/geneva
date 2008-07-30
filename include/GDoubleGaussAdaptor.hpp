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

#include <cmath>

#include <boost/version.hpp>

#if BOOST_VERSION < 103600
#error "Error: Boost should at least have version 1.36 !"
#endif /* BOOST_VERSION */

#include <boost/cstdint.hpp>

#ifndef GDOUBLEGAUSSADAPTOR_H_
#define GDOUBLEGAUSSADAPTOR_H_

using namespace std;

#include "GTemplateAdaptor.hpp"
#include "GRandom.hpp"

namespace GenEvA
{

  const double DEFAULTSIGMA=0.1;
  const double DEFAULTSIGMASIGMA=0.;
  const double DEFAULTMINSIGMA=0.0000001;

  /*************************************************************************/
  /**
   * The GDoubleGaussAdaptor reprensents an adaptor used for the mutation of double values through 
   * the addition of gaussian-distributed random numbers. See the documentation of GTemplateAdaptor<T>
   * for further information on adaptors in the GenEvA context. This class is at the
   * core of evolutionary strategies, as implemented by this library.
   * 
   * According to valgrind, for a program doing solely ES-based optimization, most time is spent in this 
   * class, due to the heavy use of the random number generator, so it is a likely target for optimization.  
   */
  class GDoubleGaussAdaptor
    :public GTemplateAdaptor<double>
  {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version){
      using boost::serialization::make_nvp;
      ar & make_nvp("GTemplateAdaptor", boost::serialization::base_object<GTemplateAdaptor<double> >(*this));
      ar & make_nvp("sigma", _sigma);
      ar & make_nvp("sigmaSigma", _sigmaSigma);
      ar & make_nvp("minSigma", _minSigma);
    }
    ///////////////////////////////////////////////////////////////////////

  public:
    /** \brief The standard constructor - every adaptor needs a name */
    explicit GDoubleGaussAdaptor(string nm) throw();
    /** \brief A standard constructor, including initialization of the sigma value */
    GDoubleGaussAdaptor(double sigma, string name) throw();
    /** \brief A standard constructor including initialization of the sigma, sigmaSigma and
     *  minSigma values */
    GDoubleGaussAdaptor(double sigma, double sigmaSigma, double minSigma, string name) throw();
    /** \brief The standard copy constructor */
    GDoubleGaussAdaptor(const GDoubleGaussAdaptor& cp) throw();
    /** \brief The standard destructor */
    virtual ~GDoubleGaussAdaptor();

    /** \brief A standard assignment operator */
    const GDoubleGaussAdaptor& operator=(const GDoubleGaussAdaptor&);
    /** \brief An assignment operator for GObject objects */
    virtual const GObject& operator=(const GObject&);

    /** \brief Resets the object to its initial state */
    virtual void reset(void);
    /** \brief Loads the values of another GDoubleGaussAdaptor */
    virtual void load(const GObject *gb);
    /** \brief Creates a deep copy of this object */
    virtual GObject *clone(void);

    /** \brief Initialises a new mutation run */
    virtual void initNewRun(void);
    /** \brief Specifies the mutations performed in this class */
    virtual void customMutate(double &value);

    /** \brief Sets the width of the gaussian */
    void  setSigma(double sigma) throw();
    /** \brief Retrieves the current width of the gaussian */
    double  getSigma(void) const throw();

    /** \brief Sets the width of the sigma adaption and the minimally 
     * allowed  value for sigma */
    void  setSigmaSigma(double sigmaSigma, double minSigma) throw();
    /** \brief Sets a minimal value for sigma */
    void  setMinSigma(double minSigma) throw();
    /** \brief Retrieves the current value of the sigma adaption */
    double  getSigmaSigma(void) const throw();
    /** \brief Retrieves the current minimal value allowed for sigma */
    double  getMinSigma(void) const throw();

    /** \brief Sets all values needed for the mutation in one go */
    void setAll(double sigma, double sigmaSigma, double minSigma);

    /** \brief Assembles a reports about the inner state of the object */
    virtual string assembleReport(uint16_t indention = 0) const;

  private:
    /** \brief The default constructor - not for public consumption */
    GDoubleGaussAdaptor(void) throw();

    double _sigma;
    double _sigmaSigma;
    double _minSigma;
  };

  /*************************************************************************/

} /* namespace GenEvA */

#endif /*GDOUBLEGAUSSADAPTOR_H_*/
