/**
 * @file
 */

/* GRandom.hpp
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

#include <cstdlib>
#include <iomanip>
#include <ctime>
#include <cmath>
#include <iostream>
#include <sstream>
#include <cassert>

#include <boost/version.hpp>

#if BOOST_VERSION < 103600
#error "Error: Boost should at least have version 1.36 !"
#endif /* BOOST_VERSION */

#include <boost/cstdint.hpp>
#include <boost/random.hpp>
#include <boost/date_time.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>
#include <boost/utility.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/pool/detail/singleton.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/exception.hpp>
#include <boost/array.hpp>
#include <boost/cstdint.hpp>

// We require at least Boost version 1.36 because of the usage of system_time
// in the boost::thread code below
#if BOOST_VERSION < 103600
#error "Error: Boost should at least have version 1.36 !"
#endif /* BOOST_VERSION */

/**
 * Check that we have support for threads. This collection of classes is useless
 * without this.
 */
#ifndef BOOST_HAS_THREADS
#error "Error: No support for Boost.threads available."
#endif

#ifndef GRANDOM_H
#define GRANDOM_H

#include "GBoundedBuffer.hpp"
#include "GEnums.hpp"
#include "GLogger.hpp"
#include "GThreadGroup.hpp"

/****************************************************************************/

namespace Gem {
namespace Util {

  const std::size_t DEFAULTARRAYSIZE = 1000;
  const std::size_t DEFAULTFACTORYBUFFERSIZE = 1000;
  const boost::uint16_t DEFAULTFACTORYPUTWAIT = 10; ///< waiting time in milliseconds
  const boost::uint16_t DEFAULTFACTORYGETWAIT = 10; ///< waiting time in milliseconds

  /****************************************************************************/
  //////////////////////////////////////////////////////////////////////////////
  /****************************************************************************/
  /**
   * \brief Returns a seed based on the current time.
   */
  boost::uint32_t GSeed(void);

  /****************************************************************************/
  //////////////////////////////////////////////////////////////////////////////
  /****************************************************************************/

  /**
   * Holds the random number packages
   */
  typedef boost::array<double,DEFAULTARRAYSIZE> GRandomNumberContainer_dbl;

  /**
   * The number of threads that simultaneously produce [0,1[ random numbers
   */
  const boost::uint8_t DEFAULT01PRODUCERTHREADS = 2;

  /**
   * Past implementations of random numbers for the Geneva library showed a
   * particular bottle neck in the random number generation. Every GObject
   * had its own random number generator, and seeding was very expensive.
   * We thus now produce floating point numbers in the range [0,1[ in a separate
   * thread in this class and calculate other numbers from this in the GRandom class.
   * A second thread is responsible for the creation of gaussian random numbers.
   * This circumvents the necessity to seed the generator over and over again and
   * allows us to get rid of a dependency on the MersenneTwister library. We are now
   * using a generator from the boost library instead, so users need to download fewer
   * libraries to use the GenEvA library.
   *
   * This class produces packets of random numbers and stores them in bounded buffers.
   * Clients can retrieve packets of random numbers, while a separate thread keeps
   * filling the buffer up.
   *
   * The implementation currently uses the lagged fibonacci generator. According to
   * http://www.boost.org/doc/libs/1_35_0/libs/random/random-performance.html this is
   * the fastest generator amongst all of Boost's generators. It is the author's belief that
   * the "quality" of random numbers is of less concern in evolutionary algorithms, as the
   * geometry of the quality surface adds to the randomness.
   */
  class GRandomFactory
  {
  public:
    /*************************************************************************/
    /**
     * The default constructor. It seeds the random number generator and starts the
     * producer threads.
     */
    GRandomFactory(void) throw() :
      g01_(DEFAULTFACTORYBUFFERSIZE),
      seed_(GSeed()),
      n01Threads_(DEFAULT01PRODUCERTHREADS)
    {
      startProducerThreads();
    }

    /*************************************************************************/
    /**
     * A constructor that creates a user-specified number of [0,1[ threads and
     * gauss threads. It seeds the random number generator and starts the
     * producer01 thread. Note that we enforce a minimum number of threads.
     */
    GRandomFactory(boost::uint8_t n01Threads) throw() :
		  g01_(DEFAULTFACTORYBUFFERSIZE),
		  seed_(GSeed()),
          n01Threads_(n01Threads ? n01Threads : 1)
    {
      startProducerThreads();
    }

    /*************************************************************************/
    /**
     * The destructor. All threads are given the interrupt signal. Then
     * we wait for them to join us.
     */
    ~GRandomFactory() throw()
    {
      producer_threads_01_.interrupt_all(); // doesn't throw

      try
        {
          producer_threads_01_.join_all();

        }
      catch (boost::thread_interrupted&)
        {
          // This should not happen - we should have caught all thread_interrupted
          // signals in the threads themselves.
    	  std::ostringstream error;
          error << "In GRandomFactory::~GRandomFactory: Error!" << std::endl
				<< "Caught boost::thread_interrupted exception." << std::endl;

          LOGGER.log(error.str(), Gem::GLogFramework::CRITICAL);

          // Terminate the process - nothing else to do in a destructor
          std::terminate();
        }
    }

    /*************************************************************************/
    /**
     * Sets the number of producer threads for this factory. The number should
     * be relatively low. 3 01 producer threads and 2 gauss producer threads (the
     * default in this library) should be a good choice for most applications.
     *
     * @param n01Threads
     * @param nGaussThreads
     */
    void setNProducerThreads(boost::uint8_t n01Threads, boost::uint8_t nGaussThreads){
      if(n01Threads > n01Threads_){ // start new 01 threads
        for(boost::uint8_t i=n01Threads_; i<n01Threads; i++){
          producer_threads_01_.create_thread(boost::bind(
                       &GRandomFactory::producer01, this, seed_ + boost::uint32_t(i)));
        }
      }
      else if(n01Threads < n01Threads_){ // We need to remove threads
    	if(n01Threads == 0) return; // We need at least 1 thread

        producer_threads_01_.remove_last(n01Threads_-n01Threads);
      }

      n01Threads_ = n01Threads;
    }

    /*************************************************************************/
    /**
     * When objects need new [0,1[ random numbers, they call this function. Note
     * that calling threads are responsible for catching the boost::thread_interrupted
     * exception.
     *
     * @return A packet of new [0,1[ random numbers
     */
    boost::shared_ptr<GRandomNumberContainer_dbl>
    new01Container(void)
    {
      boost::shared_ptr<GRandomNumberContainer_dbl> result; // empty

      try
        {
          g01_.pop_back(&result, 0, DEFAULTFACTORYGETWAIT);
        }
      catch (Gem::Util::gem_util_condition_time_out&)
        {
          // nothing - our way of signaling a time out
          // is to return an empty boost::shared_ptr
        }

      return result;
    }

  private:
    /*************************************************************************/
    /**
     * This function starts the threads needed for the production of random numbers
     */
    void
    startProducerThreads(void) throw()
    {
      for (boost::uint8_t i = 0; i < n01Threads_; i++)
        {
          // thread() doesn't throw, and no exceptions are listed in the documentation
          // for the create_thread() function, so we assume it doesn't throw.
          producer_threads_01_.create_thread(boost::bind(
              &GRandomFactory::producer01, this, seed_ + boost::uint32_t(i)));
        }
    }

    /*************************************************************************/
    /**
     * The production of [0,1[ random numbers takes place here. As this function
     * is called in a thread, it may not throw under any circumstance. Exceptions
     * could otherwise go unnoticed. Hence this function has a possibly confusing
     * setup.
     *
     * @param seed The seed for our local random number generator
     */
    void
    producer01(const boost::uint32_t& seed) throw()
    {
      try
        {
          boost::lagged_fibonacci607 lf(seed);

          while (true)
            {
              // Interruption requested ?
              if (boost::this_thread::interruption_requested())
                break;

              if (g01_.remainingSpace())
                { // any space left ?
                  boost::shared_ptr<GRandomNumberContainer_dbl> p(
                      new GRandomNumberContainer_dbl);

                  for (std::size_t i = 0; i < DEFAULTARRAYSIZE; i++)
                    {
#ifdef DEBUG
                      double value = lf();
                      assert(value>=0. && value<1.);
                      p->at(i)=value;
#else
                      (*p)[i] = lf();
#endif /* DEBUG */
                    }

                  try
                    {
                      g01_.push_front(p, 0, DEFAULTFACTORYPUTWAIT);
                    }
                  catch (Gem::Util::gem_util_condition_time_out&)
                    {
                      p.reset();
                    }
                }
              else
                {
                  // we put ourselves to sleep for a while.
                  // Note that this is also an interruption point,
                  // whose exception is caught outside of the loop.
                  boost::this_thread::sleep(
                  boost::posix_time::milliseconds(DEFAULTFACTORYPUTWAIT));
                }
            }
        }
      catch (boost::thread_interrupted&)
        { // Not an error
          return; // We're done
        }
      catch (std::bad_alloc& e)
        {
    	  std::ostringstream error;
    	  error << "In GRandomFactory::producer01(): Error!" << std::endl
                << "Caught std::bad_alloc exception with message" << std::endl
                << e.what() << std::endl;

    	  LOGGER.log(error.str(), Gem::GLogFramework::CRITICAL);

          std::terminate();
        }
      catch (std::invalid_argument& e)
        {
    	  std::ostringstream error;
    	  error << "In GRandomFactory::producer01(): Error!" << std::endl
                << "Caught std::invalid_argument exception with message" << std::endl
                << e.what() << std::endl;

    	  LOGGER.log(error.str(), Gem::GLogFramework::CRITICAL);

          std::terminate();
        }
      catch (boost::thread_resource_error&)
        {
    	  std::ostringstream error;
          error << "In GRandomFactory::producer01(): Error!" << std::endl
                << "Caught boost::thread_resource_error exception which" << std::endl
                << "likely indicates that a mutex could not be locked." << std::endl;

          LOGGER.log(error.str(), Gem::GLogFramework::CRITICAL);

          // Terminate the process
          std::terminate();
        }
      catch (...)
        {
    	  std::ostringstream error;
          error << "In GRandomFactory::producer01(): Error!" << std::endl
                << "Caught unkown exception." << std::endl;

          LOGGER.log(error.str(), Gem::GLogFramework::CRITICAL);

          // Terminate the process
          std::terminate();
        }
    }

    /*************************************************************************/

    /** @brief A bounded buffer holding the [0,1[ random number packages */
    Gem::Util::GBoundedBuffer<boost::shared_ptr<GRandomNumberContainer_dbl> > g01_;

    boost::uint32_t seed_; ///< The seed for the random number generators

    boost::uint8_t n01Threads_; ///< The number of threads used to produce [0,1[ random numbers (255)

    GThreadGroup producer_threads_01_; ///< A thread group that holds [0,1[ producer threads
  };

/**
 * A single, global random number factory is created.
 */
typedef boost::details::pool::singleton_default<Gem::Util::GRandomFactory> grfactory;
#define GRANDOMFACTORY grfactory::instance()

/****************************************************************************/
//////////////////////////////////////////////////////////////////////////////
/****************************************************************************/
/**
 * This class gives objects access to random numbers. It internally handles
 * retrieval of random numbers from the GRandomFactory class as needed. Random
 * distributions are calculated on the fly from these numbers. Usage is thus
 * transparent to the user.
 */
class GRandom : boost::noncopyable
{
public:
  /*************************************************************************/
  /**
   * The standard constructor. It gets the initial random containers from the
   * random factory.
   */
  GRandom(void) throw() :
    current01_(0)
  { /* nothing */
  }

  /*************************************************************************/
  /**
   * This function emits evenly distributed random numbers in the range [0,1[ .
   * Random numbers are usually not produced locally, but are taken from an array
   * provided by the the GRandomFactory class. Random numbers are only produced
   * locally if no valid array could be retrieved.
   *
   * @return Random numbers evenly distributed in the range [0,1[ .
   */
  double
  evenRandom(void)
  {
    // If the object has been newly created,
    // p01_ will be empty
    if(!p01_ || current01_ == DEFAULTARRAYSIZE){
       getNewP01();
       current01_ = 0;
    }

#ifdef DEBUG
    return p01_->at(current01_++); // throws
#else
    return (*p01_)[current01_++];
#endif
  }

  /*************************************************************************/
  /**
   * This function emits evenly distributed random numbers in the range [0,max[ .
   *
   * @param max The maximum (excluded) value of the range
   * @return Random numbers evenly distributed in the range [0,max[
   */
  double
  evenRandom(double max)
  {
#ifdef DEBUG
    // Check that min and max have appropriate values
    assert(max>0.);
#endif
    return GRandom::evenRandom() * max;
  }

  /*************************************************************************/
  /**
   * This function produces evenly distributed random numbers in the range [min,max[ .
   *
   * @param min The minimum value of the range
   * @param max The maximum (excluded) value of the range
   * @return Random numbers evenly distributed in the range [min,max[
   */
  double
  evenRandom(double min, double max)
  {
#ifdef DEBUG
    // Check that min and max have appropriate values
    assert(min<=max);
#endif
    return GRandom::evenRandom() * (max - min) + min;
  }

  /*************************************************************************/
  /**
   * Gaussian-distributed random numbers form the core of Evolutionary Strategies.
   * This function provides an easy means of producing such random numbers with
   * mean "mean" and sigma "sigma".
   *
   * @param mean The mean value of the Gaussian
   * @param sigma The sigma of the Gaussian
   * @return double random numbers with a gaussian distribution
   */
  double
  gaussRandom(double mean, double sigma)
  {
	return sima*sqrt(fabs(-2.*log(1.-evenRandom()))) * sin(2.*M_PI*evenRandom()) + mean;

    if(!pGauss_ || currentGauss_==DEFAULTARRAYSIZE){
      getNewPGauss();
      currentGauss_=0;
    }

#ifdef DEBUG
    return sigma * pGauss_->at(currentGauss_++) + mean; // throws
#else
    return sigma * (*pGauss_)[currentGauss_++] + mean;
#endif
  }

  /*************************************************************************/
  /**
   * This function adds two gaussians with sigma "sigma" and a distance
   * "distance" from each other of distance, centered around mean.
   *
   * @param mean The mean value of the entire distribution
   * @param sigma The sigma of both gaussians
   * @param distance The distance between both peaks
   * @return Random numbers with a double-gaussian shape
   */
  double
  doubleGaussRandom(double mean, double sigma, double distance)
  {
	  if(GRandom::bitRandom() == Gem::GenEvA::TRUE)
		  return GRandom::gaussRandom(mean - fabs(distance / 2.), sigma);
	  else
		  return GRandom::gaussRandom(mean + fabs(distance / 2.), sigma);
  }

  /*************************************************************************/
  /**
   * This function produces integer random numbers in the range of [0, max[ .
   *
   * @param max The maximum (excluded) value of the range
   * @return Discrete random numbers evenly distributed in the range [0,max[
   */
  boost::uint16_t
  discreteRandom(boost::uint16_t max)
  {
    boost::uint16_t result = static_cast<boost::uint16_t> (GRandom::evenRandom(
        static_cast<double> (max)));
#ifdef DEBUG
    assert(result<max);
#endif
    return result;
  }

  /*************************************************************************/
  /**
   * This function produces integer random numbers in the range of [min, max[ .
   * Note that max may also be < 0. .
   *
   * @param min The minimum value of the range
   * @param max The maximum (excluded) value of the range
   * @return Discrete random numbers evenly distributed in the range [min,max[
   */
  boost::int16_t
  discreteRandom(boost::int16_t min, boost::int16_t max)
  {
#ifdef DEBUG
	assert(min < max);
#endif
	boost::int16_t result = discreteRandom(static_cast<boost::int16_t>(max-min)) + min;

#ifdef DEBUG
    assert(result>=min && result<max);
#endif
    return result;
  }

  /*************************************************************************/
  /**
   * This function produces boolean values with a 50% likelihood each for
   * true and false.
   *
   * @return Boolean values with a 50% likelihood for true/false respectively
   */
  GenEvA::bit
  bitRandom(void)
  {
    return bitRandom(0.5);
  }

  /*************************************************************************/
  /**
   * This function returns true with a probability "probability", otherwise false.
   *
   * @param p The probability for the value "true" to be returned
   * @return A boolean value, which will be true with a user-defined likelihood
   */
  GenEvA::bit
  bitRandom(double probability)
  {
#ifdef DEBUG
    assert(probability>=0 && probability<=1);
#endif
    return (GRandom::evenRandom() < probability ? GenEvA::TRUE : GenEvA::FALSE);
  }

  /*************************************************************************/
  /**
   * This function produces random ASCII characters. Please note that that
   * includes also non-printable characters, if "printable" is set to false
   * (default is true).
   *
   * @param printable A boolean variable indicating whether only printable characters should be produced
   * @return Random ASCII characters
   */
  char
  charRandom(bool printable = true)
  {
    if (!printable)
      {
        return (char) discreteRandom(0, 128);
      }
    else
      {
        return (char) discreteRandom(33, 127);
      }
  }

private:
  GRandom(const GRandom&); ///< Intentionally left undefined
  GRandom& operator=(const GRandom&); ///< Intentionally left undefined

  /*************************************************************************/
  /**
   * In cases where GRandomFactory was not able to supply us with a suitable
   * array of [0,1[ random numbers we need to produce our own.
   */
  void
  fillContainer01(void)
  {
    boost::lagged_fibonacci607 lf(GSeed());
    boost::shared_ptr<GRandomNumberContainer_dbl> p(new GRandomNumberContainer_dbl);

    for (std::size_t i = 0; i < DEFAULTARRAYSIZE; i++)
      {
#ifdef DEBUG
        double value = lf();
        assert(value>=0. && value<1.);
        p->at(i)=value;
#else
        (*p)[i] = lf();
#endif /* DEBUG */
      }

    p01_ = p;
  }

  /*************************************************************************/
  /**
   * (Re-)Initialization of p01_
   */
  inline void
  getNewP01(void)
  {
    p01_ = GRANDOMFACTORY.new01Container();

    if (!p01_)
      {
        // Something went wrong with the retrieval of the
        // random number container. We need to create
        // our own instead.
        GRandom::fillContainer01();
      }
  }

  /*************************************************************************/
  boost::shared_ptr<GRandomNumberContainer_dbl> p01_; ///< Holds the container of [0,1[ random numbers

  std::size_t current01_;
};

/****************************************************************************/
//////////////////////////////////////////////////////////////////////////////
/****************************************************************************/

} /* namespace Util */
} /* namespace Gem */

#endif /* GRANDOM_H */
