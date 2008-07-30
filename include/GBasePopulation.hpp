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
#include <boost/lexical_cast.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/cast.hpp>
#include <boost/date_time.hpp>
#include <boost/date_time/gregorian/greg_serialize.hpp>
#include <boost/date_time/posix_time/time_serialize.hpp>

/** 
 * Check that we have support for threads
 */
#ifndef BOOST_HAS_THREADS
#error "Error: Support for multi-threading does not seem to be available."
#endif

#ifndef GBASEPOPULATION_H_
#define GBASEPOPULATION_H_

using namespace std;

#include "GManagedMemberCollection.hpp"
#include "GRandom.hpp"

namespace GenEvA
{

  /**
   * The two const variables MUPLUSNU and MUCOMMANU determine, whether new parents
   * should be selected from the entire population or from the children only
   */
  const bool MUPLUSNU=true;
  const bool MUCOMMANU=false;

  /**
   * The two const variables MAXIMIZE and MINIMIZE determine, whether the library
   * should work in maximization or minimization mode.
   */
  const bool MAXIMIZE=true;
  const bool MINIMIZE=false;

  /**
   * Currently three types of recombination schemes are supported:
   * - DEFAULTRECOMBINE defaults to RANDOMRECOMBINE
   * - RANDOMRECOMBINE chooses the parents to be replicated randomly from all parents
   * - VALUERECOMBINE prefers parents with a higher fitness
   */
  const int8_t DEFAULTRECOMBINE=0;
  const int8_t RANDOMRECOMBINE=1;
  const int8_t VALUERECOMBINE=2;

  /**
   * The number of generations after which information should be
   * emitted about the inner state of the population.
   */
  const int32_t DEFAULTREPORTGEN=20;

  /**
   * The default maximum number of generations
   */
  const int32_t DEFAULTMAXGEN=100;

  /**
   * The default maximization mode
   */
  const bool DEFAULTMAXMODE=false;

  /**
   * A 0 time period . timedHalt will not trigger if this duration is set
   */
  const string EMPTYDURATION = "00:00:00.000"; // 0 - no duration

  /**
   * The default maximum duration of the calculation. 
   */
  const string DEFAULTDURATION = EMPTYDURATION;



  /*********************************************************************************/
  /**
   * The GBasePopulation class adds the notion of parents and children to
   * the GMemberCollection class. The evolutionary adaptation is realised
   * through the cycle of mutation, evaluation and sorting, as defined in this
   * class.
   *
   * A population is a collection of individuals, which themselves are classes
   * exhibiting the GMember class'es API, most notably the GMember::fitness() and
   * GMember::mutate() functions. Individuals thus can themselves be populations,
   * which can again contain populations, and so on.
   *
   * In order to add parents to this class, use the default constructor, then add a
   * at least one GMember to the class and call setNParents(). This function will "fill 
   * up" the class as required. Note that children you might have added already 
   * will be discarded.
   */
  class GBasePopulation
    :public GManagedMemberCollection
  {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version){
      using boost::serialization::make_nvp;
	
      ar & make_nvp("GManagedMemberCollection", boost::serialization::base_object<GManagedMemberCollection>(*this));
      ar & make_nvp("nParents_",nParents_);
      ar & make_nvp("popSize_",popSize_);
      ar & make_nvp("generation_",generation_);
      ar & make_nvp("maxGeneration_",maxGeneration_);
      ar & make_nvp("reportGeneration_",reportGeneration_);
      ar & make_nvp("recombinationMethod_",recombinationMethod_);
      ar & make_nvp("muplusnu_",muplusnu_);
      ar & make_nvp("maximize_",maximize_);
      ar & make_nvp("id_",id_);
      ar & make_nvp("defaultChildren_", defaultChildren_);
      ar & make_nvp("maxDuration_", maxDuration_);
      // Note that firstId_ is not serialized as we want the id to 
      // be recalculated for de-serialized objects
    }
    ///////////////////////////////////////////////////////////////////////

  public:
    /** \brief The default constructor */
    GBasePopulation(void);
    /** \brief A standard copy constructor */
    GBasePopulation(const GBasePopulation&);
    /** \brief The destructor */
    virtual ~GBasePopulation();

    /** \brief A standard assignment operator */
    const GBasePopulation& operator=(const GBasePopulation&);
    /** \brief An assignment operator for GObject objects */
    virtual const GObject& operator=(const GObject&);

    /** \brief Resets the object to the state it had upon initialization
     * with the default constructor */
    virtual void reset(void);
    /** \brief Loads the data of another population */
    virtual void load(const GObject *);
    /** \brief Creates a deep clone of this object */
    virtual GObject *clone(void);

    /** \brief The core function of the entire GenEvA library.
     * Triggers the optimisation of a population. */
    virtual void optimize(void);
	
    /** \brief Emits information specific to this population */
    virtual void doInfo(void);

    /** \brief Registers a function to be called when emitting information from doInfo */
    void registerInfoFunction(boost::function<void (GBasePopulation * const)>);
	
    /** \brief Sets population size and number of parents */
    void setPopulationSize(uint16_t, uint16_t);

    /** \brief Retrieve the number of parents in this population */
    uint16_t getNParents(void) const;
    /** \brief Retrieve the number of children in this population */
    uint16_t getNChildren(void) const;
    /** \brief Retrieve the current population size */
    uint16_t getPopulationSize(void) const;

    /** \brief Set the sorting scheme for this population (MUPLUSNU or MUCOMMANU) */
    void setSortingScheme(bool);
    /** \brief Retrieve the current sorting scheme for this population */
    bool getSortingScheme(void) const;

    /** \brief Set the number of generations after which sorting should be stopped */
    void setMaxGeneration(uint32_t);
    /** \brief Retrieve the number of generations after which sorting should be stopped */
    uint32_t getMaxGeneration(void) const;
    /** \brief Get information about the current generation */
    const uint32_t getGeneration(void) const;

    /** \brief Sets the maximum allowed processing time day/hour/minute/second */
    int32_t setMaxTime(int32_t, int32_t, int32_t, int32_t);
    /** \brief Retrieves the maximum allowed processing time */
    uint32_t getMaxTime(void);
	
    /** \brief Specify whether we want to work in maximization or minimization mode */
    void setMaximize(bool val);
    /** \brief Find out whether we work in maximization or minimization mode */
    bool getMaximize(void) const;

    /** \brief Specify, what recombination mode should be used */
    void setRecombinationMethod(int8_t);
    /** \brief Find out, what recombination mode is being used */
    int8_t getRecombinationMethod(void) const;

    /** \brief Sets the number of generations after which the population should
     * report about its inner state. */
    void setReportGeneration(uint32_t);
    /** \brief Returns the number of generations after which the population should
     * report about its inner state. */
    uint32_t getReportGeneration(void) const;
	
    /** \brief Retrieve the id of this class */
    string getId(void);

    /** \brief Emits information about the internal state of this object */
    virtual string assembleReport(uint16_t indention = 0) const;

  protected:
    /** \brief user-defined halt-criterium for the optimization */
    virtual bool customHalt(void);
    /** \brief user-defined recombination scheme */
    virtual void customRecombine(void);

    /** \brief Creates children from parents according to a predefined recombination scheme */
    virtual void recombine(void);
    /** \brief Mutates all children of this population */
    virtual void mutateChildren(void);
    /** \brief Selects the best children of the population */
    virtual void select(void);

    /** \brief The mutation scheme for this population */
    virtual void customMutate(void);
    /** \brief The evaluation scheme for this population */
    virtual double customFitness(void);
	
    /** \brief Marks parents as parents and children as children */
    void markParents(void);
	
    /** \brief Retrieves the defaultChildren_ parameter */
    uint16_t getDefaultChildren(void) const;
	
  private:
    /** \brief Sets the defaultChildren_ parameter */
    void setDefaultChildren(uint16_t);
    /** \brief Emits true once a given time has passed */
    bool timedHalt(void);
    /** \brief Reset the generation_ counter to 0 */
    void resetGeneration(void);
    /** \brief Adjusts the actual population size to the desired value */
    void adjustPopulationSize(void);
    /** \brief Increases the generation_ counter by 1 */
    void incrGeneration(void);
    /** \brief Sets the generation_ counter to a given value */
    void setGeneration(uint32_t);

    /** \brief sets the id of this class to a sensible value */
    void setId(void);
	
    /** \brief Determines when to stop the optimization */
    bool halt(void);

    /** \brief Implements the RANDOMRECOMBINE recombination scheme */
    void randomRecombine(uint16_t);
    /** \brief Implements the VALUERECOMBINE recombination scheme */
    void valueRecombine(uint16_t);

    uint16_t nParents_; ///< The number of parents
    uint16_t popSize_; ///< The size of the population. Only used in adjustPopulationSize()
    uint32_t generation_; ///< The current generation
    uint32_t maxGeneration_; ///< The maximum number of generations
    uint32_t reportGeneration_; ///< Number of generations after which a report should be issued
    int8_t recombinationMethod_; ///< The chosen recombination method
    bool muplusnu_; ///< The chosen sorting scheme
    bool maximize_; ///< The optimisation mode (minimization/false vs. maximization/true)
    string id_; ///< A unique id, used in networking contexts
    bool firstId_; ///< Is this the first call to getId() ?
    boost::posix_time::time_duration maxDuration_; ///< Maximum timeframe for the optimization
    boost::posix_time::ptime startTime_; ///< Used to store the start time of the optimization
    uint16_t defaultChildren_; ///< Expected number of children
    
    boost::function<bool (shared_ptr<GMember>, shared_ptr<GMember>)> f_min_; ///< Function object for minimization
    boost::function<bool (shared_ptr<GMember>, shared_ptr<GMember>)> f_max_; ///< Function object for maximization	
    boost::function<void (GBasePopulation * const)> infoFunction_; ///< Used to emit information with doInfo()
  };

  /*********************************************************************************/

}

#endif /*GBASEPOPULATION_H_*/
