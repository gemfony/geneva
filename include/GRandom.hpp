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

#include <cstdlib>
#include <iomanip>
#include <ctime>
#include <cmath>
#include <iostream>
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

using namespace std;
using namespace boost;

#include "GBoundedBuffer.hpp"
#include "GEnums.hpp"
#include "GLogStreamer.hpp"
#include "GThreadGroup.hpp"

/****************************************************************************/

namespace GenEvA
{
  // should be uint32_t or std::size_t ??
  const uint16_t DEFAULTARRAYSIZE = 1000;
  const uint16_t DEFAULTFACTORYBUFFERSIZE = 1000;
  const uint16_t DEFAULTFACTORYPUTWAIT = 10; ///< waiting time in milliseconds
  const uint16_t DEFAULTFACTORYGETWAIT = 10; ///< waiting time in milliseconds
  const uint16_t DEFAULTFACTORYGAUSSWAIT = 200; ///< waiting time in milliseconds


  /****************************************************************************/
  //////////////////////////////////////////////////////////////////////////////
  /****************************************************************************/
  /**
   * \brief Returns a seed based on the current time.
   */
  uint32_t
  GSeed(void);

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
  const uint8_t DEFAULT01PRODUCERTHREADS = 3;

  /**
   * The number of threads that simultaneously produce gauss random numbers
   */
  const uint8_t DEFAULTGAUSSPRODUCERTHREADS = 3;

  /**
   * Past implementations of random numbers for the Geneva library showed a
   * particular bottle neck in the random number generation. Every GMember
   * object had its own random number generator, and seeding was very expensive.
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
      g01_(DEFAULTFACTORYBUFFERSIZE), gGauss_(DEFAULTFACTORYBUFFERSIZE), seed_(
          GSeed()), n01Threads_(DEFAULT01PRODUCERTHREADS), nGaussThreads_(
          DEFAULTGAUSSPRODUCERTHREADS)
    {
      startProducerThreads();
    }

    /*************************************************************************/
    /**
     * A constructor that creates a user-specified number of [0,1[ threads and
     * gauss threads. It seeds the random number generator and starts the
     * producer01 thread. Note that we enforce a minimum number of threads.
     */
    GRandomFactory(uint8_t n01Threads, uint8_t nGaussThreads) throw() :
      g01_(DEFAULTFACTORYBUFFERSIZE), gGauss_(DEFAULTFACTORYBUFFERSIZE), seed_(
          GSeed()), n01Threads_(n01Threads ? n01Threads : 1), nGaussThreads_(
          nGaussThreads ? nGaussThreads : 1)
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
      producer_threads_gauss_.interrupt_all(); // doesn't throw

      try
        {
          producer_threads_01_.join_all();
          producer_threads_gauss_.join_all();

        }
      catch (boost::thread_interrupted&)
        {
          // This should not happen - we should have caught all thread_interrupted
          // signals in the threads themselves.
          GLogStreamer gls;
          gls << "In GRandomFactory::~GRandomFactory: Error!" << endl
              << "Caught boost::thread_interrupted exception." << endl
              << logLevel(CRITICAL);

          // Terminate the process - nothing else to do in a destructor
          abort();
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
    void setNProducerThreads(uint8_t n01Threads, uint8_t nGaussThreads){
      if(n01Threads > n01Threads_){ // start new 01 threads
        for(uint8_t i=n01Threads_; i<n01Threads; i++){
          producer_threads_01_.create_thread(boost::bind(
                       &GRandomFactory::producer01, this, seed_ + uint32_t(i)));
        }
      }
      else if(n01Threads < n01Threads_){ // We need to remove threads
        producer_threads_01_.remove_last(n01Threads_-n01Threads);
      }

      if(nGaussThreads > nGaussThreads_){ // start new gauss threads
        for(uint8_t i=nGaussThreads_; i<nGaussThreads; i++){
          producer_threads_gauss_.create_thread(boost::bind(
                       &GRandomFactory::producer01, this, seed_ + uint32_t(i)));
        }
      }
      else if(nGaussThreads < nGaussThreads_){ // We need to remove threads
        producer_threads_gauss_.remove_last(nGaussThreads_-nGaussThreads);
      }

      n01Threads_ = n01Threads;
      nGaussThreads_=nGaussThreads;
    }

    /*************************************************************************/
    /**
     * When objects need new [0,1[ random numbers, they call this function. Note
     * that calling threads are responsible for catching the boost::thread_interrupted
     * exception.
     *
     * @return A packet of new [0,1[ random numbers
     */
    shared_ptr<GRandomNumberContainer_dbl>
    new01Container(void)
    {
      shared_ptr<GRandomNumberContainer_dbl> result; // empty

      try
        {
          g01_.pop_back(&result, 0, DEFAULTFACTORYGETWAIT);
        }
      catch (time_out&)
        {
          // nothing - our way of signaling a time out
          // is to return an empty shared_ptr
        }

      return result;
    }

    /*************************************************************************/
    /**
     * When objects need new gaussian random numbers, they call this function.
     *
     * @return A packet of new gaussian random numbers
     */
    shared_ptr<GRandomNumberContainer_dbl>
    newGaussContainer(void)
    {
      shared_ptr<GRandomNumberContainer_dbl> dummy; // empty

      try
        {
          gGauss_.pop_back(&dummy, 0, DEFAULTFACTORYGETWAIT);
        }
      catch (time_out&)
        {
          // nothing - our way of signalling a time out
          // is to return an empty shared_ptr
        }

      return dummy;
    }

  private:
    /*************************************************************************/
    /**
     * This function starts the threads needed for the production of random numbers
     */
    void
    startProducerThreads(void) throw()
    {
      for (uint8_t i = 0; i < n01Threads_; i++)
        {
          // thread() doesn't throw, and no exceptions are listed in the documentation
          // for the create_thread() function, so we assume it doesn't throw.
          producer_threads_01_.create_thread(boost::bind(
              &GRandomFactory::producer01, this, seed_ + uint32_t(i)));
        }

      for (uint8_t i = 0; i < nGaussThreads_; i++)
        {
          producer_threads_gauss_.create_thread(boost::bind(
              &GRandomFactory::producerGauss, this));
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
    producer01(const uint32_t& seed) throw()
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
                  shared_ptr<GRandomNumberContainer_dbl> p(
                      new GRandomNumberContainer_dbl);

                  for (uint16_t i = 0; i < DEFAULTARRAYSIZE; i++)
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
                  catch (time_out& t)
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
          GLogStreamer gls;
          gls << "In GRandomFactory::producer01(): Error!" << endl
              << "Caught std::bad_alloc exception with message" << endl
              << e.what() << endl << logLevel(CRITICAL);

          abort();
        }
      catch (std::invalid_argument& e)
        {
          GLogStreamer gls;
          gls << "In GRandomFactory::producer01(): Error!" << endl
              << "Caught std::invalid_argument exception with message" << endl
              << e.what() << endl << logLevel(CRITICAL);

          abort();
        }
      catch (boost::thread_resource_error&)
        {
          GLogStreamer gls;
          gls << "In GRandomFactory::producer01(): Error!" << endl
              << "Caught boost::thread_resource_error exception which" << endl
              << "likely indicates that a mutex could not be locked." << endl
              << logLevel(CRITICAL);

          // Terminate the process
          abort();
        }
      catch (...)
        {
          GLogStreamer gls;
          gls << "In GRandomFactory::producer01(): Error!" << endl
              << "Caught unkown exception." << endl << logLevel(CRITICAL);

          abort();
        }
    }

    /*************************************************************************/
    /**
     * The production of gaussian random numbers takes place here.
     */
    void
    producerGauss(void) throw()
    {
      try
        {
          // Give the [0,1[ thread(s) time to produce some packages before we start
          boost::this_thread::sleep(
          boost::posix_time::milliseconds(DEFAULTFACTORYGAUSSWAIT));

          while (true)
            {
              // Interruption requested ?
              if (boost::this_thread::interruption_requested())
                break;

              if (gGauss_.remainingSpace()) // any space left in the buffer ?
                {
                  shared_ptr<GRandomNumberContainer_dbl> p(
                      new GRandomNumberContainer_dbl);

                  shared_ptr<GRandomNumberContainer_dbl> x1_ = new01Container();
                  shared_ptr<GRandomNumberContainer_dbl> x2_ = new01Container();

#ifdef DEBUG
                  cout << "Retrieved factory [0,1[ container" << endl;
                  cout << "Retrieved factory [0,1[ container" << endl;
#endif

                  if (x1_ && x2_)
                    {
                      for (uint16_t i = 0; i < DEFAULTARRAYSIZE; i++)
                        {
                          double d1 = 0., d2 = 0.;

#ifdef DEBUG
                          d1 = x1_->at(i);
                          d2 = x2_->at(i);
                          p->at(i) = sqrt(fabs(-2. * log(1. - d1))) * sin(2. * M_PI * d2);
#else
                          d1 = (*x1_)[i];
                          d2 = (*x2_)[i];
                          (*p)[i] = sqrt(fabs(-2. * log(1. - d1))) * sin(2.
                              * M_PI * d2);
#endif /* DEBUG */
                        }

                      try
                        {
                          gGauss_.push_front(p, 0, DEFAULTFACTORYPUTWAIT);
                        }
                      catch (time_out& t)
                        {
                          p.reset();
                        }
                    }
                  else
                    {
                      // couldn't get valid arrays. Go to sleep. This is a possible interruption point.
                      boost::this_thread::sleep(
                      boost::posix_time::milliseconds(DEFAULTFACTORYGAUSSWAIT));
                    }
                }
              else
                { // no space available. Sleep for a while
                  boost::this_thread::sleep(
                  boost::posix_time::milliseconds(DEFAULTFACTORYPUTWAIT));
                }
            }
        }
      catch (boost::thread_interrupted&)
        {
          return; // Leave the function. We're done.
        }
      catch (std::bad_alloc& e)
        {
          GLogStreamer gls;
          gls << "In GRandomFactory::producerGauss(): Error!" << endl
              << "Caught std::bad_alloc exception with message" << endl
              << e.what() << endl << logLevel(CRITICAL);

          abort();
        }
      catch (boost::thread_resource_error&)
        {
          GLogStreamer gls;
          gls << "In GRandomFactory::producerGauss(): Error!" << endl
              << "Caught boost::thread_resource_error exception which" << endl
              << "likely indicates that a mutex could not be locked." << endl
              << logLevel(CRITICAL);

          // Terminate the process
          abort();
        }
      catch (...)
        {
          GLogStreamer gls;
          gls << "In GRandomFactory::producerGauss(): Error!" << endl
              << "Caught unknown exception" << endl << logLevel(CRITICAL);

          // Terminate the process
          abort();
        }
    }

    /*************************************************************************/

    GBoundedBuffer<boost::shared_ptr<GRandomNumberContainer_dbl> > g01_; ///< A bounded buffer holding the [0,1[ random number packages
    GBoundedBuffer<boost::shared_ptr<GRandomNumberContainer_dbl> > gGauss_; ///< A bounded buffer holding gaussian random number packages

    uint32_t seed_; ///< The seed for the random number generators

    uint8_t n01Threads_; ///< The number of threads used to produce [0,1[ random numbers (255)
    uint8_t nGaussThreads_; ///< The number of threads used to produce gauss random numbers (255)

    GThreadGroup producer_threads_01_; ///< A thread group that holds [0,1[ producer threads
    GThreadGroup producer_threads_gauss_; ///< A thread group that holds [0,1[ producer threads
  };

/**
 * A single, global random number factory is created.
 */
typedef boost::details::pool::singleton_default<GRandomFactory> grfactory;
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
    current01_(0),
    currentGauss_(0)
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
    double simpleGauss = 0;

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
    return GRandom::gaussRandom(mean - fabs(distance / 2.), sigma)
        + GRandom::gaussRandom(mean + fabs(distance / 2.), sigma);
  }

  /*************************************************************************/
  /**
   * This function produces integer random numbers in the range of [0, max[ .
   *
   * @param max The maximum (excluded) value of the range
   * @return Discrete random numbers evenly distributed in the range [0,max[
   */
  int16_t
  discreteRandom(int16_t max)
  {
    int16_t result = static_cast<int16_t> (GRandom::evenRandom(
        static_cast<double> (max)));
#ifdef DEBUG
    assert(result>=0 && result<max);
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
  int16_t
  discreteRandom(int16_t min, int16_t max)
  {
    int16_t result = static_cast<int16_t> (GRandom::evenRandom(
        static_cast<double> (min), static_cast<double> (max)));
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
   * This function returns true with a probability p, otherwise false.
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
  GRandom&
  operator=(const GRandom&); ///< Intentionally left undefined

  /*************************************************************************/
  /**
   * In cases where GRandomFactory was not able to supply us with a suitable
   * array of [0,1[ random numbers we need to produce our own.
   */
  void
  fillContainer01(void)
  {
    boost::lagged_fibonacci607 lf(GSeed());
    shared_ptr<GRandomNumberContainer_dbl> p(new GRandomNumberContainer_dbl);

    for (uint16_t i = 0; i < DEFAULTARRAYSIZE; i++)
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
   * In cases where GRandomFactory was not able to supply us with a suitable
   * array of gaussian random numbers we need to produce our own.
   */
  void
  fillContainerGauss(void)
  {
    shared_ptr<GRandomNumberContainer_dbl> p(new GRandomNumberContainer_dbl);

    for (uint16_t i = 0; i < DEFAULTARRAYSIZE; i++)
      {
#ifdef DEBUG
        double value = sqrt(fabs(-2. * log(1. - evenRandom()))) * sin(2. * M_PI * evenRandom());
        p->at(i)=value;
#else
        (*p)[i] = sqrt(fabs(-2. * log(1. - evenRandom()))) * sin(2. * M_PI * evenRandom());
#endif /* DEBUG */
      }

    pGauss_ = p;
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

#ifdef DEBUG
        cout << "Manually filled [0,1[ container" << endl;
#endif
      }
#ifdef DEBUG
    else {
      cout << "Retrieved factory [0,1[ container" << endl;
    }
#endif

  }

  /*************************************************************************/
  /**
   * (Re-)Initialization of pGauss_
   */
  inline void
  getNewPGauss(void)
  {
    pGauss_ = GRANDOMFACTORY.newGaussContainer();

    if (!pGauss_)
      {
        // Something went wrong with the retrieval of the
        // random number container. We need to create
        // our own instead.
        GRandom::fillContainerGauss();
#ifdef DEBUG
        cout << "Manually filled gauss container" << endl;
#endif
      }
#ifdef DEBUG
    else {
      cout << "Retrieved factory gauss container" << endl;
    }
#endif
  }

  /*************************************************************************/
  shared_ptr<GRandomNumberContainer_dbl> p01_; ///< Holds the container of [0,1[ random numbers
  shared_ptr<GRandomNumberContainer_dbl> pGauss_; ///< Holds the container of gaussian random numbers

  uint16_t current01_;
  uint16_t currentGauss_;
};

/****************************************************************************/
//////////////////////////////////////////////////////////////////////////////
/****************************************************************************/

} /* namespace GenEvA */

#endif /* GRANDOM_H */
