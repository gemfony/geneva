/**
 * @file G_OA_SimulatedAnnealing.hpp
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

// Standard headers go here
#include <tuple>

// Boost headers go here

#ifndef G_OA_SIMULATEDANNEALING_HPP_
#define G_OA_SIMULATEDANNEALING_HPP_

// Geneva headers go here
#include "common/GExceptions.hpp"
#include "common/GCommonHelperFunctionsT.hpp"
#include "common/GPlotDesigner.hpp"
#include "geneva/GOptimizationEnums.hpp"
#include "geneva/GParameterSet.hpp"
#include "geneva/G_OptimizationAlgorithm_ParChild.hpp"
#include "geneva/G_OptimizationAlgorithm_SimulatedAnnealing_PersonalityTraits.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This is a specialization of the GParameterSetParChild class. The class adds
 * an infrastructure for simulated annealing (Geneva-style, i.e. with larger populations).
 */
class GSimulatedAnnealing : public G_OptimizationAlgorithm_ParChild
{
	 ///////////////////////////////////////////////////////////////////////
	 friend class boost::serialization::access;

	 template<typename Archive>
	 void serialize(Archive & ar, const unsigned int) {
		 using boost::serialization::make_nvp;

		 ar
		 & make_nvp("G_OptimizationAlgorithm_ParChild", boost::serialization::base_object<G_OptimizationAlgorithm_ParChild>(*this))
		 & BOOST_SERIALIZATION_NVP(m_t0)
		 & BOOST_SERIALIZATION_NVP(m_t)
		 & BOOST_SERIALIZATION_NVP(m_alpha)
		 & BOOST_SERIALIZATION_NVP(m_n_threads);
	 }
	 ///////////////////////////////////////////////////////////////////////

public:
	 /***************************************************************************/
	 /** @brief The default constructor */
	 GSimulatedAnnealing();
	 /** @brief A standard copy constructor */
	 GSimulatedAnnealing(const GSimulatedAnnealing& cp);
	 /** @brief The standard destructor */
	 virtual ~GSimulatedAnnealing() = default;

	 /** @brief The standard assignment operator */
	 GSimulatedAnnealing& operator=(const GSimulatedAnnealing& cp);

	 /** @brief Searches for compliance with expectations with respect to another object of the same type */
	 virtual void compare(
		 const GObject& cp // the other object
		 , const Gem::Common::expectation& e // the expectation for this object, e.g. equality
		 , const double& limit// the limit for allowed deviations of floating point types
	 ) const override;

	 /** @brief Resets the settings of this population to what was configured when the optimize()-call was issued */
	 void resetToOptimizationStart() override;

	 /** @brief Returns information about the type of optimization algorithm */
	 std::string getAlgorithmPersonalityType() const override;

	 /** @brief Returns the name of this optimization algorithm */
	 std::string getAlgorithmName() const override;

	 /** @brief Adds local configuration options to a GParserBuilder object */
	 virtual void addConfigurationOptions (
		 Gem::Common::GParserBuilder& gpb
	 ) override;

	 /** @brief Sets the number of threads this population uses for adaption */
	 void setNThreads(std::uint16_t nThreads);
	 /** @brief Retrieves the number of threads this population uses for adaption */
	 std::uint16_t getNThreads() const;

	 /** @brief Determines the strength of the temperature degradation */
	 void setTDegradationStrength(double alpha);
	 /** @brief Retrieves the temperature degradation strength. This function is used for simulated annealing */
	 double getTDegradationStrength() const;

	 /** @brief Sets the start temperature. This function is used for simulated annealing */
	 void setT0(double t0);
	 /** @brief Retrieves the start temperature. This function is used for simulated annealing */
	 double getT0() const;
	 /** @brief Retrieves the current temperature. This function is used for simulated annealing */
	 double getT() const;

	 /** @brief Emits a name for this class / object */
	 std::string name() const override;

protected:
	 /***************************************************************************/

	 /** @brief Loads the data of another GSimulatedAnnealingT object, camouflaged as a GObject */
	 void load_(const GObject *cp) override;
	 /** @brief Creates a deep copy of this object */
	 GObject *clone_() const override;

	 /** @brief Some error checks related to population sizes */
	 void populationSanityChecks() const override;

	 /** @brief Adapt all children in parallel */
	 void adaptChildren() override;

	 /** @brief  We submit individuals to the broker connector and wait for processed items. */
	 void runFitnessCalculation() override;

	 /** @brief Fixes the population after a job submission */
	 void fixAfterJobSubmission();

	 /** @brief Choose new parents, based on the SA selection scheme. */
	 void selectBest() override;

	 /** @brief Retrieves the evaluation range in a given iteration and sorting scheme */
	 std::tuple<std::size_t,std::size_t> getEvaluationRange() const override;

	 /** @brief Does any necessary initialization work before the optimization loop starts */
	 void init() override;
	 /** @brief Does any necessary finalization work after the optimization loop has ended */
	 void finalize() override;

	 /** @brief Retrieve a GPersonalityTraits object belonging to this algorithm */
	 std::shared_ptr<GPersonalityTraits> getPersonalityTraits() const override;

private:
	 /***************************************************************************/

	 /** @brief Performs a simulated annealing style sorting and selection */
	 void sortSAMode();

	 /** @brief Calculates the simulated annealing probability for a child to replace a parent */
	 double saProb(const double &qParent, const double &qChild);

	 /** @brief Updates the temperature. This function is used for simulated annealing. */
	 void updateTemperature();

	 /***************************************************************************/
	 // Data

	 double m_t0 = SA_T0; ///< The start temperature, used in simulated annealing
	 double m_t = m_t0; ///< The current temperature, used in simulated annealing
	 double m_alpha = SA_ALPHA; ///< A constant used in the cooling schedule in simulated annealing

	 std::uint16_t m_n_threads = boost::numeric_cast<std::uint16_t>(Gem::Common::getNHardwareThreads(
		 Gem::Common::DEFAULTNHARDWARETHREADS
		 , Gem::Common::DEFAULTMAXNHARDWARETHREADS
	 )); ///< The number of threads

	 std::shared_ptr<Gem::Common::GThreadPool> m_tp_ptr; ///< Temporarily holds a thread pool

public:
	 /***************************************************************************/

	 /** @brief Applies modifications to this object */
	 bool modify_GUnitTests() override;
	 /** @brief Performs self tests that are expected to succeed */
	 void specificTestsNoFailureExpected_GUnitTests() override;
	 /** @brief Performs self tests that are expected to fail */
	 void specificTestsFailuresExpected_GUnitTests() override;

	 /***************************************************************************/
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GSimulatedAnnealing)

#endif /* G_OA_SIMULATEDANNEALING_HPP_ */
