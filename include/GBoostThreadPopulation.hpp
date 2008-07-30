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

#include <boost/bind.hpp>
#include <boost/threadpool.hpp>

#ifndef GBOOSTTHREADPOPULATION_H_
#define GBOOSTTHREADPOPULATION_H_

using namespace std;
using namespace boost;
using namespace boost::threadpool;

#include "GBasePopulation.hpp"
#include "GBoostThreadGeneral.hpp"
#include "GMemberCarrier.hpp"

namespace GenEvA
{
  /** \brief The default number of threads for parallelisation with boost */
  const uint16_t DEFAULTBOOSTTHREADS=2;

  /********************************************************************/
  /**
   * A multithreaded population based on GBasePopulation. This version
   * uses the Boost.Threads library. See also the GPthreadPopulation 
   * class for another thread implementation.
   */
  class GBoostThreadPopulation
    :public GenEvA::GBasePopulation
  {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version){
      using boost::serialization::make_nvp;
      ar & make_nvp("GBTGBasePopulation", boost::serialization::base_object<GBasePopulation>(*this));
      ar & make_nvp("maxThreads_", maxThreads_);
    }
    ///////////////////////////////////////////////////////////////////////

  public:
    /** \brief The default constructor */
    GBoostThreadPopulation();
    /** \brief A standard copy constructor */
    GBoostThreadPopulation(const GBoostThreadPopulation&);
    /** \brief The standard destructor */
    virtual ~GBoostThreadPopulation();

    /** \brief Assignment operator */
    const GBoostThreadPopulation& operator=(const GBoostThreadPopulation&);
    /** \brief An assignment operator for GObject objects */
    virtual const GObject& operator=(const GObject&);
	
    /** \brief Resets the object to its initial state */
    virtual void reset(void);
    /** \brief Loads data from another object */
    virtual void load(const GObject *);
    /** \brief Creates a deep clone of this object */
    virtual GObject *clone(void);
  	
    /** \brief Overloaded from GBasePopulation::optimize() */
    virtual void optimize(void);
	  	
    /** \brief Sets the maximum number of threads */ 
    void setMaxThreads(uint8_t);
    /** \brief Retrieves the maximum number of threads */
    uint8_t getMaxThreads(void);

    /** \brief Creates a report about the inner state of this object */
    virtual string assembleReport(uint16_t indention = 0) const;

  protected:
    /** \brief Overloaded version from GBasePopulation, 
     * core of the Boost-thread implementation */
    virtual void mutateChildren(void);
  	
  private:
    uint8_t maxThreads_; ///< The maximum number of threads
    pool tp_; ///< A thread pool
  };

  /********************************************************************/

} /* namespace GenEvA */

#endif /*GBOOSTTHREADPOPULATION_H_*/
