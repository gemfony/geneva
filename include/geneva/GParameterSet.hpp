/**
 * @file
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

#pragma once

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard header files go here
#include <map>
#include <typeinfo>
#include <limits>
#include <functional>

// Boost header files go here
#include <boost/numeric/conversion/bounds.hpp>
#include <boost/serialization/split_member.hpp>

// Geneva headers go here
#include "common/GExceptions.hpp"
#include "common/GCommonHelperFunctionsT.hpp"
#include "common/GCommonMathHelperFunctions.hpp"
#include "common/GLogger.hpp"
#include "common/GCommonHelperFunctionsT.hpp"
#include "common/GPtrVectorT.hpp"
#include "hap/GRandomT.hpp"
#include "courtier/GProcessingContainerT.hpp"
#include "geneva/GObject.hpp"
#include "geneva/GParameterBase.hpp"
#include "geneva/GenevaHelperFunctionsT.hpp"
#include "geneva/GPersonalityTraits.hpp"
#include "geneva/G_Interface_Mutable.hpp"
#include "geneva/G_Interface_Rateable.hpp"
#include "geneva/GPersonalityTraits.hpp"
#include "geneva/GMultiConstraintT.hpp"

#ifdef GEM_TESTING

#include "geneva/GBooleanObject.hpp"
#include "geneva/GConstrainedInt32Object.hpp"
#include "geneva/GConstrainedDoubleObject.hpp"
#include "geneva/GConstrainedDoubleObjectCollection.hpp"
#include "geneva/GConstrainedInt32ObjectCollection.hpp"
#include "geneva/GDoubleCollection.hpp"
#include "geneva/GDoubleObject.hpp"
#include "geneva/GParameterObjectCollection.hpp"
#include "geneva/GInt32Collection.hpp"
#include "common/GUnitTestFrameworkT.hpp"
#include "hap/GRandomT.hpp"

#endif /* GEM_TESTING */

namespace Gem {

namespace Tests {
class GTestIndividual1; // forward declaration, needed for testing purposes
} /* namespace Tests */

namespace Geneva {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * Container for fitness and transformed fitness values, as produced by the
 * GParameterSet class.
 */
class parameterset_processing_result
{
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template<typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;
        ar
        & BOOST_SERIALIZATION_NVP(m_raw_fitness)
        & BOOST_SERIALIZATION_NVP(m_transformed_fitness)
        & BOOST_SERIALIZATION_NVP(m_transformed_fitness_set);
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /** @brief The default constuctor */
    parameterset_processing_result() = default;

    /** @brief Initialization with a raw fitness */
    explicit parameterset_processing_result(double);

    /** @brief Initialization with a raw and transformed fitness */
    parameterset_processing_result(double, double);

    /** @brief Initialization with a raw fitness and recalculation of the transformed fitness */
    parameterset_processing_result(double, std::function<double(double)>);

    /** @brief Copy construction */
    parameterset_processing_result(parameterset_processing_result const &) = default;

    /** @brief Move construction */
    parameterset_processing_result(parameterset_processing_result &&) = default;

    /** @brief Destructor */
    ~parameterset_processing_result() = default;

    /** @brief Assignment */
    parameterset_processing_result &operator=(parameterset_processing_result const &) = default;

    /** @brief Move assignment */
    parameterset_processing_result &operator=(parameterset_processing_result &&) = default;

    /** @brief Access to the raw fitness */
    double rawFitness() const;

    /** @brief Access to the transformed fitness */
    double transformedFitness() const;

    /** @brief Updates the transformed fitness using an external function */
    void setTransformedFitnessWith(std::function<double(double)>);

    /** @brief Sets the transformed fitness to a user-defined value */
    void setTransformedFitnessTo(double);

    /** @brief Sets the transformed fitness to the same value as the raw fitness */
    void setTransformedFitnessToRaw();

    /** @brief Checks whether the transformed fitness was set */
    bool transformedFitnessSet() const;

    /** @brief Resets the object and stores a new raw value in the class */
    void reset(double);

    /** @brief Resets the object and stores a new raw and transformed value in the class */
    void reset(double, double);

    /** @brief Resets the object and stores a new raw value in the class and triggers recalculation of the transformed value */
    void reset(double, std::function<double(double)>);

private:
    /***************************************************************************/
    // Data

    double m_raw_fitness = 0.; ///< The fitness as it comes out of the fitnessCalculation() function
    double m_transformed_fitness = m_raw_fitness; ///< The fitness as calculated from m_raw_fitness through
    bool m_transformed_fitness_set = false; ///< Indicates whether a suitable m_transformed_fitness value is available
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class implements a collection of GParameterBase objects. It
 * will form the basis of many user-defined individuals.
 */
class GParameterSet
    : public GObject
    , public G_Interface_Mutable
    , public G_Interface_Rateable
    , public Gem::Common::GPtrVectorT<GParameterBase, GObject>
    , public Gem::Courtier::GProcessingContainerT<GParameterSet, parameterset_processing_result>
{
    friend class Gem::Tests::GTestIndividual1; ///< Needed for testing purposes

    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template<typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;
        ar
        & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GObject)
        & make_nvp(
            "GStdPtrVectorInterfaceT_GParameterBase"
            , boost::serialization::base_object<Gem::Common::GPtrVectorT<GParameterBase, GObject>>(*this))
        & make_nvp(
            "GProcessingContainerT_ParameterSet_double"
            , boost::serialization::base_object<Gem::Courtier::GProcessingContainerT<GParameterSet, parameterset_processing_result>>(*this))
        & BOOST_SERIALIZATION_NVP(m_best_past_primary_fitness)
        & BOOST_SERIALIZATION_NVP(m_n_stalls)
        & BOOST_SERIALIZATION_NVP(m_maxmode)
        & BOOST_SERIALIZATION_NVP(m_assigned_iteration)
        & BOOST_SERIALIZATION_NVP(m_validity_level)
        & BOOST_SERIALIZATION_NVP(m_pt_ptr)
        & BOOST_SERIALIZATION_NVP(m_eval_policy)
        & BOOST_SERIALIZATION_NVP(m_individual_constraint_ptr)
        & BOOST_SERIALIZATION_NVP(m_sigmoid_steepness)
        & BOOST_SERIALIZATION_NVP(m_sigmoid_extremes)
        & BOOST_SERIALIZATION_NVP(m_max_unsuccessful_adaptions)
        & BOOST_SERIALIZATION_NVP(m_max_retries_until_valid)
        & BOOST_SERIALIZATION_NVP(m_n_adaptions)
        & BOOST_SERIALIZATION_NVP(m_useRandomCrash)
        & BOOST_SERIALIZATION_NVP(m_randomCrashProb);
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /** @brief The default constructor */
    G_API_GENEVA GParameterSet();
    /** @brief Initialization with the number of fitness criteria */
    explicit G_API_GENEVA GParameterSet(std::size_t);
    /** @brief The copy constructor */
    G_API_GENEVA GParameterSet(GParameterSet const &);
    /** @brief The destructor */
    G_API_GENEVA ~GParameterSet() override = default;

    /** Swap another object's vector with ours. */
    void swap(GParameterSet &cp);

    /** @brief Allows to randomly initialize parameter members */
    G_API_GENEVA bool randomInit(activityMode const &);

    /** @brief Specify whether we want to work in maximization (maxMode::MAXIMIZE) or minimization (maxMode::MINIMIZE) mode */
    G_API_GENEVA void setMaxMode(maxMode const &);

    /** @brief Transformation of the individual's parameter objects into a boost::property_tree object */
    G_API_GENEVA void toPropertyTree(
        pt::ptree &
        , std::string const & = "parameterset"
    ) const;

    /** @brief Transformation of the individual's parameter objects into a list of comma-separated values */
    G_API_GENEVA std::string toCSV(
        bool= false // withNameAndType
        , bool= true // withCommas
        , bool= true // useRawFitness
        , bool= true // showValidity
    ) const;

    /** @brief Prevent shadowing of std::vector<GParameterBase>::at() */
    G_API_GENEVA Gem::Common::GPtrVectorT<GParameterBase, GObject>::reference at(std::size_t const &pos);

    /** @brief Checks whether this object is better than a given set of evaluations */
    G_API_GENEVA bool isGoodEnough(std::vector<double> const &);

    /** @brief Perform a cross-over operation between this object and another */
    virtual G_API_GENEVA std::shared_ptr<GParameterSet> crossOverWith(
        std::shared_ptr<GParameterSet> const &
    ) const BASE;

    /** @brief Triggers updates of adaptors contained in this object */
    G_API_GENEVA void updateAdaptorsOnStall(std::uint32_t);

    /** @brief Retrieves information from adaptors with a given property */
    G_API_GENEVA void queryAdaptor(
        std::string const &adaptorName
        , std::string const &property
        , std::vector<boost::any> &data
    ) const;

    /** @brief Retrieves parameters relevant for the evaluation from another GParameterSet */
    virtual G_API_GENEVA void cannibalize(GParameterSet &) BASE;

    /** @brief The adaption interface */
    G_API_GENEVA std::size_t adapt() override;

    /** @brief Register another result value of the fitness calculation */
    G_API_GENEVA void setResult(std::size_t, double);
    /** @brief Determines whether more than one fitness criterion is present for this individual */
    G_API_GENEVA bool hasMultipleFitnessCriteria() const;

    /** @brief Retrieve the fitness tuple at a given evaluation position */
    G_API_GENEVA std::tuple<double, double> getFitnessTuple(std::uint32_t = 0) const;

    /** @brief Allows to retrieve the m_maxmode parameter */
    G_API_GENEVA maxMode getMaxMode() const;

    /** @brief Retrieves the worst possible evaluation result, depending on whether we are in maximization or minimization mode */
    virtual G_API_GENEVA double getWorstCase() const BASE;

    /** @brief Retrieves the best possible evaluation result, depending on whether we are in maximization or minimization mode */
    virtual G_API_GENEVA double getBestCase() const BASE;

    /** @brief Retrieves the steepness_ variable (used for the sigmoid transformation) */
    G_API_GENEVA double getSteepness() const;
    /** @brief Sets the steepness variable (used for the sigmoid transformation) */
    G_API_GENEVA void setSteepness(double);

    /** @brief Retrieves the barrier_ variable (used for the sigmoid transformation) */
    G_API_GENEVA double getBarrier() const;
    /** @brief Sets the barrier variable (used for the sigmoid transformation) */
    G_API_GENEVA void setBarrier(double);

    /** @brief Sets the maximum number of adaption attempts that may pass without actual modifications */
    G_API_GENEVA void setMaxUnsuccessfulAdaptions(std::size_t);
    /** @brief Retrieves the maximum number of adaption attempts that may pass without actual modifications */
    G_API_GENEVA std::size_t getMaxUnsuccessfulAdaptions() const;

    /** @brief Set maximum number of retries until a valid individual was found  */
    G_API_GENEVA void setMaxRetriesUntilValid(std::size_t maxRetriesUntilValid);
    /** Retrieves the maximum number of retries until a valid individual was found. */
    G_API_GENEVA std::size_t getMaxRetriesUntilValid() const;

    /** @brief Retrieves the number of adaptions performed during the last call to adapt() */
    G_API_GENEVA std::size_t getNAdaptions() const;

    /** @brief Allows to set the current iteration of the parent optimization algorithm. */
    G_API_GENEVA void setAssignedIteration(std::uint32_t const &);
    /** @brief Gives access to the parent optimization algorithm's iteration */
    G_API_GENEVA std::uint32_t getAssignedIteration() const;

    /** @brief Allows to specify the number of optimization cycles without improvement of the primary fitness criterion */
    G_API_GENEVA void setNStalls(std::uint32_t const &);
    /** @brief Allows to retrieve the number of optimization cycles without improvement of the primary fitness criterion */
    G_API_GENEVA std::uint32_t getNStalls() const;

    /** @brief Retrieves an identifier for the current personality of this object */
    G_API_GENEVA std::string getPersonality() const;

    /** @brief Allows to activate random crashes for debugging purposes */
    G_API_GENEVA void setRandomCrash(bool, double);
    /** @brief Allows to check whether random crashes are activated, and with which probability the occur */
    G_API_GENEVA std::tuple<bool, double> getRandomCrash() const;

    /***************************************************************************/
    /**
     * Retrieves a parameter of a given type at the specified position.
     */
    template<typename val_type>
    val_type getVarVal(
        std::tuple<std::size_t, std::string, std::size_t> const &target
    ) {
        val_type result = val_type(0);

        if (typeid(val_type) == typeid(double)) {
            return boost::numeric_cast<val_type>(
                boost::any_cast<double>(
                    this->getVarVal(
                        "d"
                        , target
                    )));
        } else if (typeid(val_type) == typeid(float)) {
            return boost::numeric_cast<val_type>(
                boost::any_cast<float>(
                    this->getVarVal(
                        "f"
                        , target
                    )));
        }
        if (typeid(val_type) == typeid(std::int32_t)) {
            return boost::numeric_cast<val_type>(
                boost::any_cast<std::int32_t>(
                    this->getVarVal(
                        "i"
                        , target
                    )));
        }
        if (typeid(val_type) == typeid(bool)) {
            return boost::numeric_cast<val_type>(
                boost::any_cast<bool>(
                    this->getVarVal(
                        "b"
                        , target
                    )));
        } else {
            throw gemfony_exception(
                g_error_streamer(
                    DO_LOG
                    , time_and_place
                )
                    << "In GParameterSet::getVarVal<>(): Error!" << std::endl
                    << "Received invalid type descriptor " << std::endl
            );
        }

        return result;
    }

    /***************************************************************************/
    /**
     * The function converts the local personality base pointer to the desired type
     * and returns it for modification by the corresponding optimization algorithm.
     * The base algorithms have been declared "friend" of GParameterSet and
     * can thus access this function. External entities have no need to do so. Note
     * that this function will only be accessible to the compiler if personality_type
     * is a derivative of GPersonalityTraits, thanks to the magic of std::enable_if
     * and type_traits.
     *
     * @return A std::shared_ptr converted to the desired target type
     */
    template<typename personality_type>
    std::shared_ptr<personality_type> getPersonalityTraits(
        typename std::enable_if<std::is_base_of<GPersonalityTraits, personality_type>::value>::type *dummy = nullptr
    ) {
#ifdef DEBUG
        // Check that m_pt_ptr actually points somewhere
        if (not m_pt_ptr) {
            throw gemfony_exception(
                g_error_streamer(
                    DO_LOG
                    , time_and_place
                )
                    << "In GParameterSet::getPersonalityTraits<personality_type>() : Empty personality pointer found"
                    << std::endl
                    << "This should not happen." << std::endl
            );

            // Make the compiler happy
            return std::shared_ptr<personality_type>();
        }
#endif /* DEBUG */

        // Does error checks on the conversion internally
        return Gem::Common::convertSmartPointer<GPersonalityTraits, personality_type>(m_pt_ptr);
    }

    /* ----------------------------------------------------------------------------------
     * Tested in GParameterSet::specificTestsNoFailureExpected_GUnitTests()
     * Tested in GParameterSet::specificTestsFailureExpected_GUnitTests()
     * ----------------------------------------------------------------------------------
     */

    /***************************************************************************/
    /** @brief This function returns the current personality traits base pointer */
    G_API_GENEVA std::shared_ptr<GPersonalityTraits> getPersonalityTraits();

    /** @brief Sets the current personality of this individual */
    G_API_GENEVA void setPersonality(
        std::shared_ptr<GPersonalityTraits>
    );
    /** @brief Resets the current personality to PERSONALITY_NONE */
    G_API_GENEVA void resetPersonality();
    /** @brief Retrieves the mnemonic used for the optimization of this object */
    G_API_GENEVA std::string getMnemonic() const;

    /** @brief Check how valid a given solution is */
    G_API_GENEVA double getValidityLevel() const;
    /** @brief Checks whether all constraints were fulfilled */
    G_API_GENEVA bool constraintsFulfilled() const;
    /** @brief Allows to register a constraint with this individual */
    G_API_GENEVA void registerConstraint(std::shared_ptr<GPreEvaluationValidityCheckT<GParameterSet>>);

    /** @brief Allows to set the policy to use in case this individual represents an invalid solution */
    G_API_GENEVA void setEvaluationPolicy(evaluationPolicy evalPolicy);
    /** @brief Allows to retrieve the current policy in case this individual represents an invalid solution */
    G_API_GENEVA evaluationPolicy getEvaluationPolicy() const;

    /** @brief Checks whether this is a valid solution; meant to be called for "clean" individuals only */
    G_API_GENEVA bool isValid() const;
    /** @brief Checks whether this solution is invalid */
    G_API_GENEVA bool isInValid() const;

    /** @brief Allows to set the globally best known primary fitness */
    G_API_GENEVA void setBestKnownPrimaryFitness(std::tuple<double, double> const &);
    /** @brief Retrieves the value of the globally best known primary fitness */
    G_API_GENEVA std::tuple<double, double> getBestKnownPrimaryFitness() const;

    /***************************************************************************/
    /**
     * This function returns a parameter set at a given position of the data set.
     * Note that this function will only be accessible to the compiler if par_type
     * is a derivative of GParameterBase, thanks to the magic of std::enable_if and
     * type_traits.
     *
     * @param pos The position in our data array that shall be converted
     * @return A converted version of the GParameterBase object, as required by the user
     */
    template<typename par_type>
    const std::shared_ptr<par_type> at(
        std::size_t const &pos
        , typename std::enable_if<std::is_base_of<GParameterBase, par_type>::value>::type *dummy = nullptr
    ) const {
        // Does error checks on the conversion internally
        return Gem::Common::convertSmartPointer<GParameterBase, par_type>(m_data_cnt.at(pos));
    }

    /* ----------------------------------------------------------------------------------
     * So far untested. See also the second version of the at() function.
     * ----------------------------------------------------------------------------------
     */

    /******************************************************************************/
    /**
     * Allows to retrieve a list of all variable names registered with the parameter set
     */
    template<typename par_type>
    std::vector<std::string> getVariableNames() const {
        std::vector<std::string> varNames;
        std::map<std::string, std::vector<par_type>> pMap;
        this->streamline<par_type>(pMap);

        for (const auto &name: pMap) {
            varNames.push_back(name.first);
        }

        return varNames;
    }

    /***************************************************************************/
    /**
     * Retrieves an item according to a description provided by the target tuple
     */
    template<typename par_type>
    boost::any getVarItem(
        std::tuple<std::size_t, std::string, std::size_t> const &target
    ) {
        boost::any result;

        switch (std::get<0>(target)) {
            //---------------------------------------------------------------------
            case 0: {
                std::vector<par_type> vars;
                this->streamline<par_type>(vars);
                result = vars.at(std::get<2>(target));
            }
                break;

                //---------------------------------------------------------------------
            case 1: // var[3]
            case 2: // var    --> treated as var[0]
            {
                std::map<std::string, std::vector<par_type>> varMap;
                this->streamline<par_type>(varMap);
                result = (
                    Gem::Common::getMapItem<std::vector<par_type>>(
                        varMap
                        , std::get<1>(target))).at(std::get<2>(target));
            }
                break;

                //---------------------------------------------------------------------
            default: {
                throw gemfony_exception(
                    g_error_streamer(
                        DO_LOG
                        , time_and_place
                    )
                        << "In GParameterSet::getVarVal(): Error!" << std::endl
                        << "Got invalid mode setting: " << std::get<0>(target) << std::endl
                );
            }
                break;

                //---------------------------------------------------------------------
        }

        return result;
    }

    /***************************************************************************/
    /**
     * Retrieve information about the total number of parameters of type
     * par_type in the individual. Note that the GParameterBase-template
     * function will throw if this function is called for an unsupported type.
     *
     * @param am An enum indicating whether only information about active, inactive or all parameters of this type should be extracted
     */
    template<typename par_type>
    std::size_t countParameters(
        activityMode const &am = activityMode::DEFAULTACTIVITYMODE
    ) const {
        std::size_t result = 0;

        // Loop over all GParameterBase objects. Each object
        // will contribute the amount of its parameters of this type
        // to the result.
        for (const auto &parm_ptr: *this) {
            result += parm_ptr->countParameters<par_type>(am);
        }

        return result;
    }

    /* ----------------------------------------------------------------------------------
     * So far untested.
     * ----------------------------------------------------------------------------------
     */

    /***************************************************************************/
    /**
     * Loops over all GParameterBase objects. Each object will add the
     * values of its parameters to the vector, if they comply with the
     * type of the parameters to be stored in the vector.
     *
     * @param parVec The vector to which the parameters will be added
     * @param am An enum indicating whether only information about active, inactive or all parameters of this type should be extracted
     */
    template<typename par_type>
    void streamline(
        std::vector<par_type> &parVec
        , activityMode const &am = activityMode::DEFAULTACTIVITYMODE
    ) const {
        // Make sure the vector is clean
        parVec.clear();

        // Loop over all GParameterBase objects.
        for (const auto &parm_ptr: *this) {
            parm_ptr->streamline<par_type>(
                parVec
                , am
            );
        }
    }

    /* ----------------------------------------------------------------------------------
     * So far untested.
     * ----------------------------------------------------------------------------------
     */

    /***************************************************************************/
    /**
     * Loops over all GParameterBase objects. Each object will add its name
     * and the values of its parameters to the map, if they comply with the
     * type of the parameters to be stored in the vector.
     *
     * @param parVec The map to which the parameters will be added
     * @param am An enum indicating whether only information about active, inactive or all parameters of this type should be extracted
     */
    template<typename par_type>
    void streamline(
        std::map<std::string, std::vector<par_type>> &parVec
        , activityMode const &am = activityMode::DEFAULTACTIVITYMODE
    ) const {
        // Make sure the vector is clean
        parVec.clear();

        // Loop over all GParameterBase objects.
        for (const auto &parm_ptr: *this) {
            parm_ptr->streamline<par_type>(
                parVec
                , am
            );
        }
    }

    /* ----------------------------------------------------------------------------------
     * So far untested.
     * ----------------------------------------------------------------------------------
     */

    /***************************************************************************/
    /**
     * Assigns values from a std::vector to the parameters in the collection
     *
     * @param parVec A vector of values, to be assigned to be added to GParameterBase derivatives
     * @param am An enum indicating whether only information about active, inactive or all parameters of this type should be assigned
     */
    template<typename par_type>
    void assignValueVector(
        std::vector<par_type> const &parVec
        , activityMode const &am = activityMode::DEFAULTACTIVITYMODE
    ) {
#ifdef DEBUG
        if (countParameters<par_type>() != parVec.size()) {
            throw gemfony_exception(
                g_error_streamer(
                    DO_LOG
                    , time_and_place
                )
                    << "In GParameterSet::assignValueVector(const std::vector<pat_type>&):" << std::endl
                    << "Sizes don't match: " << countParameters<par_type>() << " / " << parVec.size() << std::endl
            );
        }
#endif /* DEBUG */

        // Start assignment at the beginning of parVec
        std::size_t pos = 0;

        // Loop over all GParameterBase objects. Each object will extract the relevant
        // parameters and increment the position counter as required.
        for (const auto &parm_ptr: *this) {
            parm_ptr->assignValueVector<par_type>(
                parVec
                , pos
                , am
            );
        }

        // As we have modified our internal data sets, make sure the item is reprocessed
        this->mark_as_due_for_processing();
    }

    /***************************************************************************/
    /**
     * Assigns values from a std::map<std::string, std::vector<par_type>> to the parameters in the collection
     *
     * @param parMap A map of values, to be assigned to be added to GParameterBase derivatives
     * @param am An enum indicating whether only information about active, inactive or all parameters of this type should be assigned
     */
    template<typename par_type>
    void assignValueVectors(
        std::map<std::string, std::vector<par_type>> const &parMap
        , activityMode const &am = activityMode::DEFAULTACTIVITYMODE
    ) {
        // Loop over all GParameterBase objects. Each object will extract the relevant parameters
        for (const auto &parm_ptr: *this) {
            parm_ptr->assignValueVectors<par_type>(
                parMap
                , am
            );
        }

        // As we have modified our internal data sets, make sure the item is reprocessed
        this->mark_as_due_for_processing();
    }

    /***************************************************************************/
    /**
     * Loops over all GParameterBase objects. Each object will add the
     * lower and upper boundaries of its parameters to the vector, if
     * they comply with the type of the parameters to be stored in the
     * vector.
     *
     * @param lBndVec The vector to which the lower boundaries will be added
     * @param uBndVec The vector to which the upper boundaries will be added
     * @param am An enum indicating whether only information about active, inactive or all parameters of this type should be extracted
     */
    template<typename par_type>
    void boundaries(
        std::vector<par_type> &lBndVec
        , std::vector<par_type> &uBndVec
        , activityMode const &am = activityMode::DEFAULTACTIVITYMODE
    ) const {
        // Make sure the vectors are clean
        lBndVec.clear();
        uBndVec.clear();

        // Loop over all GParameterBase objects.
        for (const auto &parm_ptr: *this) {
            parm_ptr->boundaries<par_type>(
                lBndVec
                , uBndVec
                , am
            );
        }
    }

    /* ----------------------------------------------------------------------------------
     * So far untested.
     * ----------------------------------------------------------------------------------
     */

    /***************************************************************************/
    /**
     * Multiplication with a random value in a given range
     */
    template<typename par_type>
    void multiplyByRandom(
        par_type const &min
        , par_type const &max
        , activityMode const &am
    ) {
        // Loop over all GParameterBase objects.
        for (auto &parm_ptr: *this) {
            parm_ptr->multiplyByRandom<par_type>(
                min
                , max
                , am
                , m_gr
            );
        }

        // As we have modified our internal data sets, make sure the item is reprocessed
        this->mark_as_due_for_processing();
    }

    /***************************************************************************/
    /**
     * Multiplication with a random value in the range [0, 1[
     */
    template<typename par_type>
    void multiplyByRandom(
        activityMode const &am
    ) {
        // Loop over all GParameterBase objects.
        for (auto &parm_ptr: *this) {
            parm_ptr->multiplyByRandom<par_type>(
                am
                , m_gr
            );
        }

        // As we have modified our internal data sets, make sure the item is reprocessed
        this->mark_as_due_for_processing();
    }

    /***************************************************************************/
    /**
     * Multiplication with a constant value
     */
    template<typename par_type>
    void multiplyBy(
        par_type const &val
        , activityMode const &am
    ) {
        // Loop over all GParameterBase objects.
        for (auto &parm_ptr: *this) {
            parm_ptr->multiplyBy<par_type>(
                val
                , am
            );
        }

        // As we have modified our internal data sets, make sure the item is reprocessed
        this->mark_as_due_for_processing();
    }

    /***************************************************************************/
    /**
     * Initializes all parameters of a given type with a constant value
     */
    template<typename par_type>
    void fixedValueInit(
        par_type const &val
        , activityMode const &am
    ) {
        // Loop over all GParameterBase objects.
        for (auto const &item_ptr: *this) {
            item_ptr->fixedValueInit<par_type>(
                val
                , am
            );
        }

        // As we have modified our internal data sets, make sure the item is reprocessed
        this->mark_as_due_for_processing();
    }

    /***************************************************************************/
    /**
     * Adds the parameters of another GParameterSet object to this one
     */
    template<typename par_type>
    void add(
        std::shared_ptr<GParameterSet> const &p
        , activityMode const &am
    ) {
        GParameterSet::iterator it;
        GParameterSet::const_iterator cit;

        // Note that the GParameterBase objects need to accept a
        // std::shared_ptr<GParameterBase>, contrary to the calling conventions
        // of this function.
        for (it = this->begin(), cit = p->begin(); it != this->end(); ++it, ++cit) {
            (*it)->add<par_type>(
                *cit
                , am
            );
        }

        // As we have modified our internal data sets, make sure the item is reprocessed
        this->mark_as_due_for_processing();
    }

    /***************************************************************************/
    /**
     * Subtracts the parameters of another GParameterSet object from this one
     */
    template<typename par_type>
    void subtract(
        std::shared_ptr<GParameterSet> const &p
        , activityMode const &am
    ) {
        GParameterSet::iterator it;
        GParameterSet::const_iterator cit;

        // Note that the GParameterBase objects need to accept a
        // std::shared_ptr<GParameterBase>, contrary to the calling conventions
        // of this function.
        for (it = this->begin(), cit = p->begin(); it != this->end(); ++it, ++cit) {
            (*it)->subtract<par_type>(
                *cit
                , am
            );
        }

        // As we have modified our internal data sets, make sure the item is reprocessed
        this->mark_as_due_for_processing();
    }

    /***************************************************************************/
    // Deleted functions

    explicit G_API_GENEVA GParameterSet(float const &) = delete; ///< Intentionally undefined
    explicit G_API_GENEVA GParameterSet(double const &) = delete; ///< Intentionally undefined

protected:
    /***************************************************************************/
    /**
     * A random number generator. Note that the actual calculation is
     * done in a random number proxy / factory
     */
    Gem::Hap::GRandomT<Gem::Hap::RANDFLAVOURS::RANDOMPROXY> m_gr;

    /***************************************************************************/
    /** @brief Do the required processing for this object */
    G_API_GENEVA void process_() override;

    /** @brief Adds local configuration options to a GParserBuilder object */
    G_API_GENEVA void addConfigurationOptions_(
        Gem::Common::GParserBuilder &
    ) override;
    /** @brief Loads the data of another GObject */
    G_API_GENEVA void load_(const GObject *) override;

    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GParameterSet>(
        GParameterSet const &
        , GParameterSet const &
        , Gem::Common::GToken &
    );

    /** @brief Searches for compliance with expectations with respect to another object of the same type */
    G_API_GENEVA void compare_(
        GObject const & // the other object
        , Gem::Common::expectation const & // the expectation for this object, e.g. equality
        , double const & // the limit for allowed deviations of floating point types
    ) const override;

    /** @brief Random initialization */
    virtual G_API_GENEVA bool randomInit_(activityMode const &) BASE;

    /* @brief The actual adaption operations. */
    virtual G_API_GENEVA std::size_t customAdaptions() BASE;

    /** @brief The fitness calculation for the main quality criterion takes place here */
    G_API_GENEVA double fitnessCalculation() override = 0;
    /** @brief Sets the fitness to a given set of values and clears the dirty flag */
    G_API_GENEVA void setFitness_(std::vector<double> const &);

    /** @brief Combines secondary evaluation results by adding the individual results */
    G_API_GENEVA double sumCombiner() const;
    /** @brief Combines secondary evaluation results by adding the absolute values of individual results */
    G_API_GENEVA double fabsSumCombiner() const;
    /** @brief Combines secondary evaluation results by calculating the square root of the squared sum */
    G_API_GENEVA double squaredSumCombiner() const;
    /** @brief Combines secondary evaluation results by calculation the square root of the weighed squared sum */
    G_API_GENEVA double weighedSquaredSumCombiner(std::vector<double> const &) const;

    /** @brief Checks whether this solution has been rated to be valid; meant to be called by internal functions only */
    G_API_GENEVA bool parameterSetFulfillsConstraints(double &) const;

    /** @brief Applies modifications to this object. This is needed for testing purposes */
    G_API_GENEVA bool modify_GUnitTests_() override;
    /** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
    G_API_GENEVA void specificTestsNoFailureExpected_GUnitTests_() override;
    /** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
    G_API_GENEVA void specificTestsFailuresExpected_GUnitTests_() override;

private:
    /***************************************************************************/
    // Overridden or virtual private functions

    /** @brief Emits a name for this class / object */
    G_API_GENEVA std::string name_() const override;
    /** @brief Creates a deep clone of this object */
    G_API_GENEVA GObject *clone_() const override = 0;

    /** @brief Retrieves the stored raw fitness with a given id */
    G_API_GENEVA double raw_fitness_(std::size_t) const final;
    /** @brief Retrieves the stored transformed fitness with a given id */
    G_API_GENEVA double transformed_fitness_(std::size_t) const final;

    /** @brief Returns all raw fitness results in a std::vector */
    G_API_GENEVA std::vector<double> raw_fitness_vec_() const final;
    /** @brief Returns all transformed fitness results in a std::vector */
    G_API_GENEVA std::vector<double> transformed_fitness_vec_() const final;

    /***************************************************************************/

    /** @brief Retrieves a parameter of a given type at the specified position */
    G_API_GENEVA boost::any getVarVal(
        const std::string &
        , const std::tuple<std::size_t, std::string, std::size_t> &target
    );

    /** @brief  Allows to set all fitnesses to the same value (both raw and transformed values) */
    void setAllFitnessTo(double);

    /** @brief  Allows to set all fitnesses to the same value (raw and transformed values seperately) */
    void setAllFitnessTo(double, double);

    /** @brief Retrieval of a suitable position for cross over inside of a vector */
    std::size_t getCrossOverPos(std::size_t, std::size_t);

    /***************************************************************************/
    // Data

    /** @brief Uniformly distributed integer random numbers */
    std::uniform_int_distribution<std::size_t> m_uniform_int;

    /** @brief Holds the globally best known primary fitness of all individuals */
    std::tuple<double, double> m_best_past_primary_fitness{
        std::make_tuple(
            0.
            , 0.
        )
    };
    /** @brief The number of stalls of the primary fitness criterion in the entire set of individuals */
    std::uint32_t m_n_stalls = 0;
    /** @brief Indicates whether we are using maximization or minimization mode */
    maxMode m_maxmode = maxMode::MINIMIZE;
    /** @brief The iteration of the parent algorithm's optimization cycle */
    std::uint32_t m_assigned_iteration = 0;
    /** @brief Indicates how valid a given solution is */
    double m_validity_level = 0.;
    /** @brief Holds the actual personality information */
    std::shared_ptr<GPersonalityTraits> m_pt_ptr;

    /** @brief Specifies what to do when the individual is marked as invalid */
    evaluationPolicy m_eval_policy = Gem::Geneva::evaluationPolicy::USESIMPLEEVALUATION;
    /** @brief Determines the "steepness" of a sigmoid function used by optimization algorithms */
    double m_sigmoid_steepness = Gem::Geneva::FITNESSSIGMOIDSTEEPNESS;
    /** @brief Determines the extreme values of a sigmoid function used by optimization algorithms */
    double m_sigmoid_extremes = Gem::Geneva::WORSTALLOWEDVALIDFITNESS;

    /** @brief A constraint-check to be applied to one or more components of this individual */
    std::shared_ptr<GPreEvaluationValidityCheckT<GParameterSet>> m_individual_constraint_ptr;

    std::size_t m_max_unsuccessful_adaptions
        = Gem::Geneva::DEFMAXUNSUCCESSFULADAPTIONS; ///< The maximum number of calls to customAdaptions() in a row without actual modifications
    std::size_t m_max_retries_until_valid
        = Gem::Geneva::DEFMAXRETRIESUNTILVALID; ///< The maximum number an adaption of an individual should be performed until a valid parameter set was found
    std::size_t m_n_adaptions = 0; ///< Stores the actual number of adaptions after a call to "adapt()"

    bool m_useRandomCrash
        = false; ///< Indicates whether the individual should crash at random intervals for debugging purposes
    double m_randomCrashProb = 0.; ///< The probability for a random crash
};

} /* namespace Geneva */
} /* namespace Gem */

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * @brief Needed for Boost.Serialization
 */
BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GParameterSet)
BOOST_CLASS_EXPORT_KEY(Gem::Geneva::parameterset_processing_result)

/******************************************************************************/

