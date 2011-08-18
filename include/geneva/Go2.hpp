/**
 * @file Go2.hpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) com
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
 * http://www.gemfony.com .
 */

// Standard header files go here
#include <vector>

// Boost header files go here
#include <boost/shared_ptr.hpp>

#ifndef GO2_HPP_
#define GO2_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif


// Geneva headers go here
#include "geneva/GOptimizableI.hpp"
#include "geneva/GMutableSetT.hpp"
#include "geneva/GParameterSet.hpp"
#include "geneva/GStdPtrVectorInterfaceT.hpp"
#include "courtier/GAsioTCPClientT.hpp"

// TODO: Add all common Geneva headers here

namespace Gem {
namespace Geneva {

/**************************************************************************************/
/**
 * This class allows to aggregate different optimization algorithms and to treat
 * all execution modes (multi-threaded, networked, ...) alike.
 */
template <typename ind_type = GParameterSet>
class Go2
	: public GMutableSetT<ind_type>
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int) {
	  using boost::serialization::make_nvp;

	  ar & make_nvp("GMutableSetT_GParameterSet", boost::serialization::base_object<GMutableSetT<GParameterSet> >(*this));
		 // & BOOST_SERIALIZATION_NVP(pers_);
	}
	///////////////////////////////////////////////////////////////////////

public:
	/** @brief The default constructor */
	Go2();
	/** @brief A constructor that first parses the command line for relevant parameters and then loads data from a config file */
	Go2(int, char **, const std::string& = GO2_DEF_DEFAULTCONFIGFILE);
	/** @brief A constructor that is given the usual command line parameters, then loads the rest of the data from a config file */
	Go2(
		const personality& pers
		, const parMode&
		, const bool&
		, const Gem::Common::serializationMode&
		, const std::string&
		, const unsigned short&
		, const std::string&
		, const bool&
	);
	/** @brief A copy constructor */
	Go2(const Go2&);

	/** @brief The destructor */
	virtual ~Go2();

	/** @brief Standard assignment operator */
	const Go2& operator=(const Go2&);

	/** @brief Checks for equality with another Go object */
	bool operator==(const Go2&) const;
	/** @brief Checks for inequality with another Go object */
	bool operator!=(const Go2&) const;

	/** @brief Checks whether this object fulfills a given expectation in relation to another object */
	virtual boost::optional<std::string> checkRelationshipWith(
			const GObject&
			, const Gem::Common::expectation&
			, const double&
			, const std::string&
			, const std::string&
			, const bool&
	) const;

	/** @brief Triggers execution of the client loop */
	bool clientRun();

	/** @brief Checks whether server mode has been requested for this object */
	bool serverMode() const;
	/** @brief Checks whether this object is running in client mode */
	bool clientMode() const;

	/* @brief Specifies whether only the best individuals of a population should be copied */
	void setCopyBestIndividualsOnly(bool);
	/** @brief Checks whether only the best individuals are copied */
	bool onlyBestIndividualsAreCopied() const;

	/** @brief Allows to randomly initialize parameter members. Unused in this wrapper object */
	virtual void randomInit();
	/** @brief Triggers fitness calculation (i.e. optimization) for this object */
	virtual double fitnessCalculation();

	/** @brief Loads some configuration data from arguments passed on the command line (or another char ** that is presented to it) */
	void parseCommandLine(int, char **);

	/** @brief Adds a solver to the chain of algorithms */
	void addSolver(boost::shared_ptr<GOptimizableI>);
	/** @brief Adds a solver to the chain of algorithms */
	Go2& operator+=(boost::shared_ptr<GOptimizableI>);

	/** @brief Start the optimization cycle and retrieve the best solution */
	boost::shared_ptr<ind_type> optimize();

	void setServerMode(const bool&);
	bool getServerMode() const;

	void setMaxStalledDataTransfers(boost::uint32_t);
	boost::uint32_t getMaxStalledDataTransfers() const;

	void setMaxConnectionAttempts(boost::uint32_t);
	boost::uint32_t getMaxConnectionAttempts() const;

	void setReturnRegardless(bool);
	bool getReturnRegardless() const;

	void setNProducerThreads(boost::uint16_t);
	boost::uint16_t getNProducerThreads() const;

	void setNEvaluationThreads(boost::uint16_t);
	boost::uint16_t getNEvaluationThreads() const;

	/************************************************************************************/
	/**
	 * Initialization code for the Geneva library collection. Most notably, we enforce
	 * the initialization of various singletons needed for Geneva.
	 */
	static void init() {
		GRANDOMFACTORY->init();
		GBROKER(Gem::Geneva::GIndividual)->init();
	}

	/************************************************************************************/
	/**
	 * Finalization code for the Geneva library collection. Most notably, we enforce
	 * shutdown of various singleton services needed for Geneva. Note that we shut down
	 * in reverse order to the startup procedure.
	 */
	static void finalize() {
		GBROKER(Gem::Geneva::GIndividual)->finalize();
		RESETGBROKER(Gem::Geneva::GIndividual);

		GRANDOMFACTORY->finalize();
		RESETGRANDOMFACTORY;

#ifdef FORCETERMINATION // Defined in GGlobalDefines.hpp.in
		std::set_terminate(GTerminate);
		std::terminate();
#endif /* FORCETERMINATION */
	}

	/************************************************************************************/

protected:
	/** @brief Adds local configuration options to a GParserBuilder object */
	virtual void addConfigurationOptions(Gem::Common::GParserBuilder&, const bool&);

private:
	std::vector<boost::shared_ptr<GOptimizableI> > algorithms_; ///< Holds pointers to the actual optimization algorithms

	bool serverMode_; ///< Specifies whether this object is in server (true) or client (false) mode

	boost::uint32_t maxStalledDataTransfers_; ///< Specifies how often a client may try to unsuccessfully retrieve data from the server (0 means endless)
	boost::uint32_t maxConnectionAttempts_; ///< Specifies how often a client may try to connect unsuccessfully to the server (0 means endless)
	bool returnRegardless_; ///< Specifies whether unsuccessful processing attempts should be returned to the server

	boost::uint16_t nProducerThreads_; ///< The number of threads that will simultaneously produce random numbers
    boost::uint16_t nEvaluationThreads_; ///< The number of threads used for evaluations in multi-threaded execution
};

/**************************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::Go2)

#endif /* GO2_HPP_ */
