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

#include <boost/version.hpp>

#if BOOST_VERSION < 103600
#error "Error: Boost should at least have version 1.36 !"
#endif /* BOOST_VERSION */

#include <boost/cstdint.hpp>

#ifndef GBITFLIPADAPTOR_H_
#define GBITFLIPADAPTOR_H_

using namespace std;

#include "GTemplateAdaptor.hpp"
#include "GDouble.hpp"
#include "GEnums.hpp"


namespace GenEvA
{

  const double SGM=0.001;
  const double SGMSGM=0.00001;
  const double MSGM=0.00001;

  const double DEFAULTMUTPROB=0.;
  const string DEFAULTGDGANAME="probabilityMutation";

  /***********************************************************************************/
  /**
   * This class is designed to allow mutations of bit values. Bits can be flipped with
   * a probability that is mutated along with the bit value. Hence the adaptor can 
   * adapt itself to varying conditions, if desired. Note that this makes the allegedly
   * simple application of flipping a bit a rather complicated procedure. Hence it is
   * recommended to limit usage of this adaptor to bit collections rather than single 
   * bits.
   */
  class GBitFlipAdaptor
    :public GTemplateAdaptor<GenEvA::bit>
  {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template<class Archive>
      void serialize(Archive & ar, const unsigned int version){
      using boost::serialization::make_nvp;
      ar & make_nvp("GTemplateAdaptor", boost::serialization::base_object<GTemplateAdaptor<GenEvA::bit> >(*this));
      ar & make_nvp("mutProb",_mutProb);
      ar & make_nvp("allowProbabilityMutation", _allowProbabilityMutation);
    }
    ///////////////////////////////////////////////////////////////////////

  public:
    /** \brief Standard constructor. Every adaptor needs a name */
    explicit GBitFlipAdaptor(string name);
    /** \brief Constructor sets the mutation probability to a given value */
    GBitFlipAdaptor(double prob, string name);
    /** \brief Standard copy constructor */
    GBitFlipAdaptor(const GBitFlipAdaptor& cp);
    /** \brief Standard destructor */
    virtual ~GBitFlipAdaptor();

    /** \brief A standard assignment operator */
    const GBitFlipAdaptor& operator=(const GBitFlipAdaptor&);
    /** \brief An assignment operator for GObject objects */
    virtual const GObject& operator=(const GObject&);

    /** \brief Resets the object to its initial state */
    virtual void reset(void);
    /** \brief Loads the content of another GBitFlipAdaptor */
    virtual void load(const GObject *gb);
    /** \brief Creates a deep copy of this object */
    virtual GObject *clone(void);

    /** \brief Retrieves the current mutation probability */
    double getMutationProbability(void);
    /** \brief Sets the mutation probability to a given value */
    void setMutationProbability(double);

    /** \brief Sets the mutation parameters of the internal GDouble */
    void setInternalMutationParameters(double,double,double);
	
    /** \brief Allow or disallow mutation of mutation probability */
    void setAllowProbabilityMutation(bool allowProbabilityMutation);
    /** \brief Retrieves value of allowProbabilityMutation */
    bool getAllowProbabilityMutation(void) const throw();

    /** \brief Initialises a new mutation run */
    virtual void initNewRun(void);

    /** \brief Assembles a reports about state of the object */
    virtual string assembleReport(uint16_t indention = 0) const;
	
  protected:
    /** \brief The actual muation of the bit value */
    virtual void customMutate(bit &value);

  private:
    /** \brief Standard constructor - not needed */
    GBitFlipAdaptor() throw();
	
    /** \brief Simple flip of a bit value */
    void flip(bit&) const throw();

    GDouble _mutProb; ///< internal representation of the mutation probability
    bool _allowProbabilityMutation; ///< do we allow the probability to be adapted ?
  };

  /***********************************************************************************/

} /* namespace GenEvA */

#endif /*GBITFLIPADAPTOR_H_*/
