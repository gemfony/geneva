/**
 * @file Go2.hpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) eu
 *
 * This file is part of the Geneva library collection.
 *
 * Geneva was developed with kind support from Karlsruhe Institute of
 * Technology (KIT) and Steinbuch Centre for Computing (SCC). Further
 * information about KIT and SCC can be found at http://www.kit.edu/english
 * and http://scc.kit.edu .
 *
 * Geneva is free software: you can redistribute and/or modify it under
 * the terms of version 3 of the GNU Affero General Public License
 * as published by the Free Software Foundation.
 *
 * Geneva is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with the Geneva library. If not, see <http://www.gnu.org/licenses/>.
 *
 * For further information on Gemfony scientific and Geneva, visit
 * http://www.gemfony.eu .
 */

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard header files go here

// Boost header files go here
#include <boost/algorithm/string.hpp>
#include <boost/thread/once.hpp>
#include <boost/function.hpp>

#ifndef GO2_HPP_
#define GO2_HPP_

// Geneva headers go here
#include "common/GFactoryT.hpp"
#include "common/GExceptions.hpp"
#include "common/GParserBuilder.hpp"
#include "hap/GRandomFactory.hpp"
#include "hap/GRandomT.hpp"
#include "courtier/GAsioHelperFunctions.hpp"
#include "courtier/GAsioTCPConsumerT.hpp"
#include "courtier/GBoostThreadConsumerT.hpp"
#include "courtier/GBrokerT.hpp"
#include "geneva/GenevaHelperFunctionsT.hpp"
#include "geneva/GOptimizableEntity.hpp"
#include "geneva/GMutableSetT.hpp"
#include "geneva/GOptimizableI.hpp"
#include "geneva/GOptimizationAlgorithmT.hpp"
#include "geneva/GOptimizationEnums.hpp"
#include "geneva/GParameterObjectCollection.hpp"
#include "geneva/GParameterSet.hpp"
#include "geneva/GOAFactoryStore.hpp"
#include "geneva/GOAMonitorStore.hpp"
#include "geneva/GIndividualStandardConsumers.hpp"
#include "geneva/GIndividualStandardConsumerInitializerT.hpp"
#include "geneva/GConsumerStore.hpp"

#include "geneva/GBaseEA.hpp"
#include "geneva/GBasePS.hpp"
#include "geneva/GBaseSwarm.hpp"
#include "geneva/GBrokerEA.hpp"
#include "geneva/GBrokerGD.hpp"
#include "geneva/GBrokerPS.hpp"
#include "geneva/GBrokerSA.hpp"
#include "geneva/GBrokerSwarm.hpp"
#include "geneva/GSerialEA.hpp"
#include "geneva/GSerialGD.hpp"
#include "geneva/GSerialPS.hpp"
#include "geneva/GSerialSA.hpp"
#include "geneva/GSerialSwarm.hpp"
#include "geneva/GMultiThreadedEA.hpp"
#include "geneva/GMultiThreadedGD.hpp"
#include "geneva/GMultiThreadedPS.hpp"
#include "geneva/GMultiThreadedSA.hpp"
#include "geneva/GMultiThreadedSwarm.hpp"
#include "geneva/GEvolutionaryAlgorithmFactory.hpp"
#include "geneva/GGradientDescentFactory.hpp"
#include "geneva/GParameterScanFactory.hpp"
#include "geneva/GSimulatedAnnealingFactory.hpp"
#include "geneva/GSwarmAlgorithmFactory.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
// Default values for the variables used by the optimizer
const std::string GO2_DEF_DEFAULTCONFIGFILE="config/Go2.json";
const bool GO2_DEF_CLIENTMODE=false;
const execMode GO2_DEF_DEFAULPARALLELIZATIONMODE=EXECMODE_MULTITHREADED;
const bool GO2_DEF_COPYBESTONLY=true;
const boost::uint16_t GO2_DEF_NPRODUCERTHREADS=0;
const boost::uint32_t GO2_DEF_OFFSET=0;
const std::string GO2_DEF_OPTALGS="";
const std::string GO2_DEF_NOCONSUMER="none";

/******************************************************************************/
/** @brief Set a number of parameters of the random number factory */
void setRNFParameters(const boost::uint16_t&);

/******************************************************************************/
/** Syntactic sugar -- make the code easier to read */
typedef Gem::Geneva::GOptimizationAlgorithmT<Gem::Geneva::GParameterSet> GOABase;

/******************************************************************************/
/**
 * This class allows to "chain" a number of optimization algorithms so that a given
 * set of individuals can be optimized using more than one algorithm in sequence. The
 * class also hides the details of client/server mode, consumer initialization, etc.
 * While it is derived from GOptimizableI, it is not currently meant to be used as an
 * individual. Hence the ability to serialize the class has been removed.
 */
class G_API Go2
	: public GMutableSetT<GParameterSet>
	, public GOptimizableI
{
public:
	/** @brief The default constructor */
	Go2();
	/** @brief A constructor that first parses the command line for relevant parameters */
	Go2(
      int
      , char **
      , const std::vector<boost::shared_ptr<boost::program_options::option_description> >&
           = std::vector<boost::shared_ptr<boost::program_options::option_description> >()
	);
   /** @brief A constructor that allows to specify a default config file name */
   Go2(const std::string&);
	/** @brief A constructor that first parses the command line for relevant parameters and allows to specify a default config file name */
	Go2(
      int
      , char **
      , const std::string&
      , const std::vector<boost::shared_ptr<boost::program_options::option_description> >&
           = std::vector<boost::shared_ptr<boost::program_options::option_description> >()
	);
	/** @brief A copy constructor */
	Go2(const Go2&);

	/** @brief The destructor */
	virtual ~Go2();

	/** @brief Standard assignment operator */
	const Go2& operator=(const Go2&);

	/** @brief Checks for equality with another Go2 object */
	bool operator==(const Go2&) const;
	/** @brief Checks for inequality with another Go2 object */
	bool operator!=(const Go2&) const;

	/** @brief Checks whether this object fulfills a given expectation in relation to another object */
	virtual boost::optional<std::string> checkRelationshipWith(
      const GObject&
      , const Gem::Common::expectation&
      , const double&
      , const std::string&
      , const std::string&
      , const bool&
	) const OVERRIDE;

	/** @brief Triggers execution of the client loop */
	int clientRun();

	/** @brief Checks whether this object is running in client mode */
	bool clientMode() const;

	/** @brief Sets the desired parallelization mode */
	void setParallelizationMode(const execMode&);
	/** @brief Retrieves the current parallelization mode */
	execMode getParallelizationMode() const;

	/* @brief Specifies whether only the best individuals of a population should be copied */
	void setCopyBestIndividualsOnly(const bool&);
	/** @brief Checks whether only the best individuals are copied */
	bool onlyBestIndividualsAreCopied() const;

	/** @brief Allows to randomly initialize parameter members. Unused in this wrapper object */
	virtual void randomInit(const activityMode&) OVERRIDE;
	/** @brief Triggers fitness calculation (i.e. optimization) for this object */
	virtual double fitnessCalculation() OVERRIDE;

	/** @brief Allows to add an optimization algorithm to the chain */
	void addAlgorithm(boost::shared_ptr<GOABase>);
	/** @brief Makes it easier to add algorithms */
	Go2& operator&(boost::shared_ptr<GOABase>);
	/** @brief Allows to add an optimization algorithm through its mnemonic */
   void addAlgorithm(const std::string&);
   /** @brief Makes it easier to add algorithms */
   Go2& operator&(const std::string&);

   /** @brief Retrieves the currently registered number of algorithms */
   std::size_t getNAlgorithms() const;
   /** @brief Retrieves the currently registered number of command line algorithms */
   std::size_t getNCLAlgorithms() const;

   /** @brief Allows to register a content creator */
   void registerContentCreator(
         boost::shared_ptr<Gem::Common::GFactoryT<GParameterSet> >
   );
	/** @brief Perform the actual optimization cycle */
	virtual void optimize(const boost::uint32_t& = 0) OVERRIDE;

	/***************************************************************************/
	// The following is a trivial list of getters and setters
	void setClientMode(bool);
	bool getClientMode() const;

	boost::uint16_t getNProducerThreads() const;

	void setOffset(const boost::uint32_t&);
	boost::uint32_t getIterationOffset() const;

	/** @brief Retrieval of the current iteration */
	virtual uint32_t getIteration() const OVERRIDE;

	/** @brief Returns the name of this optimization algorithm */
	virtual std::string getAlgorithmName() const OVERRIDE;

	/** @brief Loads some configuration data from arguments passed on the command line (or another char ** that is presented to it) */
	void parseCommandLine(
      int
      , char **
      , const std::vector<boost::shared_ptr<boost::program_options::option_description> >&
        = std::vector<boost::shared_ptr<boost::program_options::option_description> >()
   );
	/** @brief Loads some configuration data from a configuration file */
	void parseConfigFile(const std::string&);

	/** @brief Adds local configuration options to a GParserBuilder object */
	virtual void addConfigurationOptions(Gem::Common::GParserBuilder&) OVERRIDE;

	/** @brief Allows to assign a name to the role of this individual(-derivative) */
	virtual std::string getIndividualCharacteristic() const OVERRIDE;

	/***************************************************************************/
	/**
	 * Starts the optimization cycle and returns the best individual found, converted to
	 * the desired target type. This is a convenience overload of the corresponding
	 * GOptimizableI function.
	 *
	 * @return The best individual found during the optimization process, converted to the desired type
	 */
	template <typename individual_type>
	boost::shared_ptr<individual_type> optimize() {
		return GOptimizableI::optimize<individual_type>();
	}

	/***************************************************************************/
	/**
	 * Starts the optimization cycle and returns the best individual found, converted to
	 * the desired target type. This function uses a configurable offset for the iteration
	 * counter. This is a convenience overload of the corresponding
	 * GOptimizableI function.
	 *
	 * @param offset An offset for the iteration counter
	 * @return The best individual found during the optimization process, converted to the desired type
	 */
	template <typename individual_type>
	boost::shared_ptr<individual_type> optimize(
			const boost::uint32_t& offset
	) {
		return GOptimizableI::optimize<individual_type>(offset);
	}

   /***************************************************************************/
   /** @brief Emits a name for this class / object */
   virtual std::string name() const;

   /** @brief Allows to register a default algorithm. */
   void registerDefaultAlgorithm(boost::shared_ptr<GOABase>);
   /** @brief Allows to register a default algorithm. */
   void registerDefaultAlgorithm(const std::string& default_algorithm);

   /** @brief Retrieves a parameter of a given type at the specified position */
   virtual boost::any getVarVal(
      const std::string&
      , const boost::tuple<std::size_t, std::string, std::size_t>& target
   ) OVERRIDE;

   /** @brief Allows to register a pluggable optimization monitor */
   void registerPluggableOM (
         boost::function<void(const infoMode&, GOptimizationAlgorithmT<GParameterSet> * const)> pluggableInfoFunction
   );
   /** @brief Allows to reset the local pluggable optimization monitor */
   void resetPluggableOM();

protected:
	/***************************************************************************/
	/** @brief Loads the data of another Go2 object */
	virtual void load_(const GObject *) OVERRIDE;
	/** @brief Creates a deep clone of this object */
	virtual GObject *clone_() const OVERRIDE;

	/** @brief Retrieves the best individual found */
	virtual boost::shared_ptr<GParameterSet> customGetBestIndividual() OVERRIDE;
	/** @brief Retrieves a list of the best individuals found */
	virtual std::vector<boost::shared_ptr<GParameterSet> > customGetBestIndividuals() OVERRIDE;

	/** @brief Satisfies a requirement of GOptimizableI */
	virtual void runFitnessCalculation() OVERRIDE;

private:
   /***************************************************************************/
   /**
    * A termination handler
    */
   static void GTerminateImproperBoostTermination() {
      std::cout
      << "***********************************************************" << std::endl
      << "* Note that there seems to be a bug in some Boost         *" << std::endl
      << "* versions that prevents proper termination of Geneva.    *" << std::endl
      << "* If you see this message it means that you are using     *" << std::endl
      << "* one of the affected releases, so we have to force       *" << std::endl
      << "* termination. Since this happens when all work has       *" << std::endl
      << "* already been done, this will very likely have no effect *" << std::endl
      << "* on your results. So you can safely ignore this message. *" << std::endl
      << "***********************************************************" << std::endl;
   }

   /** @brief Sets the number of random number production threads */
   void setNProducerThreads(const boost::uint16_t&);

	/***************************************************************************/
	// These parameters can enter the object through the constructor
	bool clientMode_; ///< Specifies whether this object represents a network client
	std::string configFilename_; ///< Indicates where the configuration file is stored
	execMode parMode_; ///< The desired parallelization mode for free-form algorithms
	std::string consumerName_; ///< The name of a consumer requested by the user on the command line

	//---------------------------------------------------------------------------
	// Parameters for the random number generator
	boost::uint16_t nProducerThreads_; ///< The number of threads that will simultaneously produce random numbers

	//---------------------------------------------------------------------------
	// Internal parameters
	boost::uint32_t offset_; ///< The offset to be used when starting a new optimization run
	bool sorted_; ///< Indicates whether local individuals have been sorted
	boost::uint32_t iterationsConsumed_; ///< The number of successive iterations performed by this object so far

	//---------------------------------------------------------------------------
	// The list of "chained" optimization algorithms
	std::vector<boost::shared_ptr<GOABase> > algorithms_;
	// Algorithms that were specified on the command line
	std::vector<boost::shared_ptr<GOABase> > cl_algorithms_;
	// The default algorithm (if any)
	boost::shared_ptr<GOABase> default_algorithm_;
	// A string representation of the default algorithm
	const std::string default_algorithm_str_; ///< This is the last fall-back
	// Holds an object capable of producing objects of the desired type
	boost::shared_ptr<Gem::Common::GFactoryT<GParameterSet> > contentCreatorPtr_;
	// A user-defined call-back for information retrieval
	boost::function<void(const infoMode&, GOptimizationAlgorithmT<GParameterSet> * const)> pluggableInfoFunction_;
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class performs some necessary initialization and finalization work, so
 * the user does not need to do this manually in main()
 */
class GenevaInitializer {
public:
	/** @brief The default constructor */
   inline GenevaInitializer();
   /** @brief The destructor */
   inline ~GenevaInitializer();

private:
   // Local copies of the factory and broker pointers. Needed so we are
   // sure the corresponding objects still exist when the destructor is called.
   boost::shared_ptr<Gem::Hap::GRandomFactory> grf_;
   boost::shared_ptr<Gem::Courtier::GBrokerT<GParameterSet> > gbr_;
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

#endif /* GO2_HPP_ */
