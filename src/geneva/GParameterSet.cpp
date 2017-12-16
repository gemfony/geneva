/**
 * @file GParameterSet.cpp
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

#include "geneva/GParameterSet.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GParameterSet)

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * Initialization with the number of fitness criteria
 */
GParameterSet::GParameterSet(const std::size_t &n_fitness_criteria)
	: m_n_fitness_criteria(n_fitness_criteria)
{ /* nothing */ }

/******************************************************************************/
/**
 * The copy ructor. Note that we cannot rely on the operator=() of the vector
 * here, as we do not know the actual type of T objects.
 *
 * @param cp A copy of another GParameterSet object
 */
GParameterSet::GParameterSet(const GParameterSet &cp)
	: GObject(cp)
   , G_Interface_Mutable(cp)
   , G_Interface_Rateable(cp)
   , Gem::Common::GStdPtrVectorInterfaceT<GParameterBase, GObject>(cp)
   , Gem::Courtier::GProcessingContainerT<GParameterSet, double>(cp)
   , m_perItemCrossOverProbability(cp.m_perItemCrossOverProbability)
   , m_n_fitness_criteria(cp.m_n_fitness_criteria)
   , m_current_fitness_vec(cp.m_current_fitness_vec)
   , m_worst_known_valids_vec(cp.m_worst_known_valids_vec)
   , m_marked_as_invalid_by_user(cp.m_marked_as_invalid_by_user)
   , m_best_past_primary_fitness(cp.m_best_past_primary_fitness)
   , m_n_stalls(cp.m_n_stalls)
   , m_dirty_flag(cp.m_dirty_flag)
   , m_maximize(cp.m_maximize)
   , m_assigned_iteration(cp.m_assigned_iteration)
   , m_validity_level(cp.m_validity_level)
   , m_eval_policy(cp.m_eval_policy)
   , m_sigmoid_steepness(cp.m_sigmoid_steepness)
   , m_sigmoid_extremes(cp.m_sigmoid_extremes)
   , m_max_unsuccessful_adaptions(cp.m_max_unsuccessful_adaptions)
   , m_max_retries_until_valid(cp.m_max_retries_until_valid)
   , m_n_adaptions(cp.m_n_adaptions)
   , m_evaluation_id(cp.m_evaluation_id)
{
	// Copy the personality pointer over
	Gem::Common::copyCloneableSmartPointer(cp.m_pt_ptr, m_pt_ptr);
	// Make sure any constraints are copied over
	Gem::Common::copyCloneableSmartPointer(cp.m_individual_constraint_ptr, m_individual_constraint_ptr);
}

/***************************************************************************/
/**
 * The standard assignment operator
 */
 GParameterSet &GParameterSet::operator=(const GParameterSet &cp) {
	this->load_(&cp);
	return *this;
}

/******************************************************************************/
/**
 * Searches for compliance with expectations with respect to another object
 * of the same type
 *
 * TODO: Compare GProcessingContainerT ?
 *
 * @param cp A constant reference to another GObject object
 * @param e The expected outcome of the comparison
 * @param limit The maximum deviation for floating point values (important for similarity checks)
 */
void GParameterSet::compare(
	const GObject &cp, const Gem::Common::expectation &e, const double &limit
) const {
	using namespace Gem::Common;

	// Check that we are dealing with a GParameterSet reference independent of this object and convert the pointer
	const GParameterSet *p_load = Gem::Common::g_convert_and_compare<GObject, GParameterSet>(cp, this);

	GToken token("GParameterSet", e);

	// Compare our parent data ...
	Gem::Common::compare_base<GObject>(IDENTITY(*this, *p_load), token);

	// ... and then the local data
	compare_t(IDENTITY(this->data,  p_load->data), token); // data is actually contained in a parent class
	compare_t(IDENTITY(m_perItemCrossOverProbability, p_load->m_perItemCrossOverProbability), token);
	compare_t(IDENTITY(m_n_fitness_criteria, p_load->m_n_fitness_criteria), token);
	compare_t(IDENTITY(m_current_fitness_vec, p_load->m_current_fitness_vec), token);
	compare_t(IDENTITY(m_worst_known_valids_vec, p_load->m_worst_known_valids_vec), token);
	compare_t(IDENTITY(m_marked_as_invalid_by_user, p_load->m_marked_as_invalid_by_user), token);
	compare_t(IDENTITY(m_best_past_primary_fitness, p_load->m_best_past_primary_fitness), token);
	compare_t(IDENTITY(m_n_stalls, p_load->m_n_stalls), token);
	compare_t(IDENTITY(m_dirty_flag, p_load->m_dirty_flag), token);
	compare_t(IDENTITY(m_maximize, p_load->m_maximize), token);
	compare_t(IDENTITY(m_assigned_iteration, p_load->m_assigned_iteration), token);
	compare_t(IDENTITY(m_validity_level, p_load->m_validity_level), token);
	compare_t(IDENTITY(m_eval_policy, p_load->m_eval_policy), token);
	compare_t(IDENTITY(m_pt_ptr, p_load->m_pt_ptr), token);
	compare_t(IDENTITY(m_individual_constraint_ptr, p_load->m_individual_constraint_ptr), token);
	compare_t(IDENTITY(m_sigmoid_steepness, p_load->m_sigmoid_steepness), token);
	compare_t(IDENTITY(m_sigmoid_extremes, p_load->m_sigmoid_extremes), token);
	compare_t(IDENTITY(m_max_unsuccessful_adaptions, p_load->m_max_unsuccessful_adaptions), token);
	compare_t(IDENTITY(m_max_retries_until_valid, p_load->m_max_retries_until_valid), token);
	compare_t(IDENTITY(m_n_adaptions, p_load->m_n_adaptions), token);
	compare_t(IDENTITY(m_evaluation_id, p_load->m_evaluation_id), token);

	// React on deviations from the expectation
	token.evaluate();
}

/******************************************************************************/
/**
 * Swap another object's vector with ours. We need to set the dirty flag of both
 * individuals in this case.
 */
void GParameterSet::swap(GParameterSet& cp) {
	Gem::Common::GStdPtrVectorInterfaceT<GParameterBase, GObject>::swap(cp.data);
	GParameterSet::setDirtyFlag();
	cp.setDirtyFlag();
}

/* ----------------------------------------------------------------------------------
 * Tested in GTestIndividual1::specificTestsNoFailureExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Allows to randomly initialize parameter members
 *
 * @return A boolean indicating whether modifications where made
 */
bool GParameterSet::randomInit(const activityMode &am) {
	// Trigger random initialization of all our parameter objects
	for(auto& parm_ptr: *this) {
		parm_ptr->randomInit(am, m_gr);
	}

	// As we have modified our internal data sets, make sure the dirty flag is set
	GParameterSet::setDirtyFlag();

	if(!this->empty()) return true;
	else return false;
}

/* ----------------------------------------------------------------------------------
 * Tested in GParameterSet::specificTestsNoFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Specify whether we want to work in maximization (true) or minimization
 * (false) mode. This function is protected. The idea is that GParameterSet provides a public
 * wrapper for this function, so that a user can specify whether he wants to maximize or
 * minimize a given evaluation function. Optimization algorithms, in turn, only check the
 * maximization-mode of the individuals stored in them and set their own maximization mode
 * internally accordingly, using the protected, overloaded function.
 *
 * @param mode A boolean which indicates whether we want to work in maximization or minimization mode
 */
void GParameterSet::setMaxMode(const bool &mode) {
	this->setMaxMode_(mode);
}

/* ----------------------------------------------------------------------------------
 * So far untested
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Provides access to all data stored in the individual in a user defined selection
 *
 * @param var_vec A std::vector of user-defined types
 */
void GParameterSet::custom_streamline(std::vector<boost::any>&)
{ /* nothing -- override in user-code */ }

/******************************************************************************/
/**
 * Transformation of the individual's parameter objects into a boost::property_tree object
 */
void GParameterSet::toPropertyTree(
	pt::ptree &ptr
	, const std::string &baseName
) const {
#ifdef DEBUG
	// Check if the object is empty. If so, complain
	if(this->empty()) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GParameterSet::toPropertyTree(): Error!" << std::endl
				<< "Object is empty." << std::endl
		);
	}
#endif

	bool dirtyFlag = this->isDirty();
	double rawFitness = 0., transformedFitness = 0.;

	ptr.put(baseName + ".iteration", this->getAssignedIteration());
	ptr.put(baseName + ".id", this->getCurrentEvaluationID());
	ptr.put(baseName + ".isDirty", dirtyFlag);
	ptr.put(baseName + ".isValid", dirtyFlag ? false : this->isValid());
	ptr.put(baseName + ".type", std::string("GParameterSet"));

	// Loop over all parameter objects and ask them to add their data to our ptree object
	ptr.put(baseName + ".nVars", this->size());
	std::string base;
	std::size_t pos = 0;
	for(const auto& item_ptr: *this) {
		base = baseName + ".vars.var" + Gem::Common::to_string(pos);
		item_ptr->toPropertyTree(ptr, base);
		pos++;
	}

	// Output the transformation policy
	switch (this->getEvaluationPolicy()) {
		case evaluationPolicy::USESIMPLEEVALUATION:
			ptr.put(baseName + ".transformationPolicy", "USESIMPLEEVALUATION");
			break;

		case evaluationPolicy::USESIGMOID:
			ptr.put(baseName + ".transformationPolicy", "USESIGMOID");
			break;

		case evaluationPolicy::USEWORSTKNOWNVALIDFORINVALID:
			ptr.put(baseName + ".transformationPolicy", "USEWORSTKNOWNVALIDFORINVALID");
			break;

		default: {
			throw gemfony_exception(
				g_error_streamer(DO_LOG,  time_and_place)
					<< "In GParameterSet::toPropertyTree(): Error!" << std::endl
					<< "Got invalid evaluation policy: " << this->getEvaluationPolicy() << std::endl
			);
		}
			break;
	}

	// Output all fitness criteria. We do not enforce re-calculation of the fitness here,
	// as the property is meant to capture the current state of the individual.
	// Check the "isDirty" tag, if you need to know whether the results are current.
	ptr.put(baseName + ".nResults", this->getNumberOfFitnessCriteria());
	for (std::size_t f = 0; f < this->getNumberOfFitnessCriteria(); f++) {
		rawFitness = dirtyFlag ? this->getWorstCase() : this->fitness(f, PREVENTREEVALUATION, USERAWFITNESS);
		transformedFitness = dirtyFlag ? this->getWorstCase() : this->transformedFitness(f);

		base = baseName + ".results.result" + Gem::Common::to_string(f);
		ptr.put(base, transformedFitness);
		base = baseName + ".results.rawResult" + Gem::Common::to_string(f);
		ptr.put(base, rawFitness);
	}
}

/******************************************************************************/
/**
 * Transformation of the individual's parameter objects into a list of
 * comma-separated values and fitness plus possibly the validity
 *
 * @param  withNameAndType Indicates whether a list of names and types should be prepended
 * @param  withCommas Indicates, whether commas should be printed
 * @param  useRawFitness Indicates, whether the true fitness instead of the transformed fitness should be returned
 * @return A string holding the parameter values and possibly the types
 */
std::string GParameterSet::toCSV(
	bool withNameAndType
	, bool withCommas
	, bool useRawFitness
	, bool showValidity
) const {
	std::map<std::string, std::vector<double>> dData;
	std::map<std::string, std::vector<float>> fData;
	std::map<std::string, std::vector<std::int32_t>> iData;
	std::map<std::string, std::vector<bool>> bData;

	// Retrieve the parameter maps
	this->streamline<double>(dData);
	this->streamline<float>(fData);
	this->streamline<std::int32_t>(iData);
	this->streamline<bool>(bData);

	std::vector<std::string> varNames;
	std::vector<std::string> varTypes;
	std::vector<std::string> varValues;



	// Extract the data
	for(const auto& item: dData) {
		for (std::size_t pos = 0; pos < (item.second).size(); pos++) {
			if (withNameAndType) {
				varNames.push_back(item.first + "_" + Gem::Common::to_string(pos));
				varTypes.push_back("double");
			}
			varValues.push_back(Gem::Common::to_string((item.second).at(pos)));
		}
	}

	for(const auto& item: fData) {
		for (std::size_t pos = 0; pos < (item.second).size(); pos++) {
			if (withNameAndType) {
				varNames.push_back(item.first + "_" + Gem::Common::to_string(pos));
				varTypes.push_back("float");
			}
			varValues.push_back(Gem::Common::to_string((item.second).at(pos)));
		}
	}

	for(const auto& item: iData) {
		for (std::size_t pos = 0; pos < (item.second).size(); pos++) {
			if (withNameAndType) {
				varNames.push_back(item.first + "_" + Gem::Common::to_string(pos));
				varTypes.push_back("int32");
			}
			varValues.push_back(Gem::Common::to_string((item.second).at(pos)));
		}
	}

	for(const auto& item: bData) {
		for (std::size_t pos = 0; pos < (item.second).size(); pos++) {
			if (withNameAndType) {
				varNames.push_back(item.first + "_" + Gem::Common::to_string(pos));
				varTypes.push_back("bool");
			}
			varValues.push_back(Gem::Common::to_string((item.second).at(pos)));
		}
	}

	// Note: The following will output the string "dirty" if the individual is in a "dirty" state
	for (std::size_t f = 0; f < this->getNumberOfFitnessCriteria(); f++) {
		if (withNameAndType) {
			varNames.push_back(std::string("Fitness_") + Gem::Common::to_string(f));
			varTypes.push_back("double");
		}
		if(!this->isDirty()) { // The individual has already been evaluated
			if (useRawFitness) {
				varValues.push_back(
					Gem::Common::to_string(
						this->fitness(
							f
							, PREVENTREEVALUATION
							, USERAWFITNESS
						)));
			} else { // Output potentially transformed fitness
				varValues.push_back(Gem::Common::to_string(this->transformedFitness(f)));
			}
		} else { // No evaluation was performed so far
			varValues.push_back("dirty");
		}
	}

	if (showValidity) {
		if (withNameAndType) {
			varNames.push_back(std::string("validity"));
			varTypes.push_back("bool");
		}

		if(!this->isDirty()) { // The individual has already been evaluated
			varValues.push_back(Gem::Common::to_string(this->isValid()));
		} else {
			varValues.push_back(Gem::Common::to_string(false));
		}
	}

	// Transfer the data into the result string
	std::ostringstream result;
	std::vector<std::string>::const_iterator s_it;
	if (withNameAndType) {
		for (s_it = varNames.begin(); s_it != varNames.end(); ++s_it) {
			result << *s_it;
			if (s_it + 1 != varNames.end()) {
				result << (withCommas ? ",\t" : "\t");
			}
		}
		result << std::endl;

		for (s_it = varTypes.begin(); s_it != varTypes.end(); ++s_it) {
			result << *s_it;
			if (s_it + 1 != varTypes.end()) {
				result << (withCommas ? ",\t" : "\t");
			}
		}
		result << std::endl;
	}

	for (s_it = varValues.begin(); s_it != varValues.end(); ++s_it) {
		result << *s_it;
		if (s_it + 1 != varValues.end()) {
			result << (withCommas ? ",\t" : "\t");
		}
	}
	result << std::endl;

	return result.str();
}

/******************************************************************************/
/**
 * Emits a name for this class / object
 *
 * @return The name of this class / object
 */
std::string GParameterSet::name() const {
	return std::string("GParameterSet");
}

/******************************************************************************/
/**
 * Retrieves a parameter of a given type at the specified position
 */
boost::any GParameterSet::getVarVal(
	const std::string &descr
	, const std::tuple<std::size_t, std::string, std::size_t> &target
) {
	boost::any result;

	if (descr == "d") {
		result = GParameterSet::getVarItem<double>(target);
	} else if (descr == "f") {
		result = GParameterSet::getVarItem<float>(target);
	} else if (descr == "i") {
		result = GParameterSet::getVarItem<std::int32_t>(target);
	} else if (descr == "b") {
		result = GParameterSet::getVarItem<bool>(target);
	} else {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GParameterSet::getVarVal(): Error!" << std::endl
				<< "Received invalid type description" << std::endl
		);
	}

	return result;
}

/******************************************************************************/
/**
 * Prevent shadowing of std::vector<GParameterBase>::at()
 *
 * @param pos The position of the item we aim to retrieve from the std::vector<GParameterBase>
 * @return The item we aim to retrieve from the std::vector<GParameterBase>
 */
Gem::Common::GStdPtrVectorInterfaceT<GParameterBase, GObject>::reference GParameterSet::at(const std::size_t &pos) {
	return Gem::Common::GStdPtrVectorInterfaceT<GParameterBase, GObject>::at(pos);
}

/* ----------------------------------------------------------------------------------
 * So far untested
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Checks whether this object is better than a given set of evaluations. This
 * function compares "real" boundaries with evaluations, hence we use "raw"
 * measurements here instead of transformed measurements.
 */
bool GParameterSet::isGoodEnough(const std::vector<double> &boundaries) {
#ifdef DEBUG
	// Does the number of fitness criteria match the number of boundaries ?
	if(boundaries.size() != this->getNumberOfFitnessCriteria()) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GParameterSet::isGoodEnough(): Error!" << std::endl
				<< "Number of boundaries does not match number of fitness criteria" << std::endl
		);
	}

	// Is the dirty flag set ?
	if(this->isDirty()) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GParameterSet::isGoodEnough(): Error!" << std::endl
				<< "Trying to compare fitness values although dirty flag is set" << std::endl
		);
	}
#endif /* DEBUG */

	// Check the fitness values. If we find at least one
	// which is worse than the one supplied by the boundaries
	// vector, then this individual fails the test
	if (true == this->getMaxMode()) { // Maximization
		for (std::size_t i = 0; i < boundaries.size(); i++) {
			if (this->fitness(i, PREVENTREEVALUATION, USERAWFITNESS) < boundaries.at(i)) {
				return false;
			}
		}
	} else { // Minimization
		for (std::size_t i = 0; i < boundaries.size(); i++) {
			if (this->fitness(i, PREVENTREEVALUATION, USERAWFITNESS) > boundaries.at(i)) {
				return false;
			}
		}
	}

	// All fitness values are better than those supplied by boundaries
	return true;
}

/******************************************************************************/
/**
 * Perform a fusion operation between this object and another.
 */
std::shared_ptr <GParameterSet> GParameterSet::amalgamate(std::shared_ptr<GParameterSet> cp) const {
	// Create a copy of this object
	std::shared_ptr<GParameterSet> this_cp = this->GObject::clone<GParameterSet>();

	this_cp->perItemCrossOver(*cp, m_perItemCrossOverProbability);

	return this_cp;
}

/******************************************************************************/
/**
 * This function performs a cross-over with another GParameterSet object with a given likelihood.
 * Items subject to cross-over may either be located in the GParameterSet-root or in one of the
 * GParmeterBase-derivatives stored in this object. Hence the cross-over operation is propagated to
 * these. The whole procedure happens on an "per item" basis, i.e., each item is swapped with the
 * corresponding "foreign" item with a given likelihood. The procedure requires both objects to have
 * the same "architecture" and will throw, if this is not the case.
 */
void GParameterSet::perItemCrossOver(
	const GParameterSet &cp
	, const double &likelihood
) {
#ifdef DEBUG
	// Do some error checking
	if(likelihood < 0. || likelihood > 1.) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GParameterSet::crossOver(): Error!" << std::endl
				<< "Received invalid likelihood: " << likelihood << std::endl
		);
	}
#endif /* DEBUG */

	// Extract all data items
	std::vector<double> this_double_vec, cp_double_vec;
	std::vector<float> this_float_vec, cp_float_vec;
	std::vector<bool> this_bool_vec, cp_bool_vec;
	std::vector<std::int32_t> this_int_vec, cp_int_vec;

	this->streamline(this_double_vec);
	this->streamline(this_float_vec);
	this->streamline(this_bool_vec);
	this->streamline(this_int_vec);

	cp.streamline(cp_double_vec);
	cp.streamline(cp_float_vec);
	cp.streamline(cp_bool_vec);
	cp.streamline(cp_int_vec);

#ifdef DEBUG
	// Do some error checking
	if(this_double_vec.size() != cp_double_vec.size()) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GParameterSet::perItemCrossOver(): Error!" << std::endl
				<< "Got invalid sizes (double): " << this_double_vec.size() << " / " << cp_double_vec.size() << std::endl
		);
	}
	if(this_float_vec.size() != cp_float_vec.size()) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GParameterSet::perItemCrossOver(): Error!" << std::endl
				<< "Got invalid sizes (float): " << this_float_vec.size() << " / " << cp_float_vec.size() << std::endl
		);
	}
	if(this_bool_vec.size() != cp_bool_vec.size()) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GParameterSet::perItemCrossOver(): Error!" << std::endl
				<< "Got invalid sizes (bool): " << this_bool_vec.size() << " / " << cp_bool_vec.size() << std::endl
		);
	}
	if(this_int_vec.size() != cp_int_vec.size()) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GParameterSet::perItemCrossOver(): Error!" << std::endl
				<< "Got invalid sizes (std::int32_t): " << this_int_vec.size() << " / " << cp_int_vec.size() << std::endl
		);
	}
#endif /* DEBUG */

	// Do the actual cross-over
	if (!this_double_vec.empty()) {
		// Calculate a suitable position for the cross-over
		std::size_t pos = this->m_uniform_int(m_gr, std::uniform_int_distribution<std::size_t>::param_type(std::size_t(0), this_double_vec.size() - std::size_t(1)));

		// Perform the actual cross-over operation
		for (std::size_t i = pos; i < this_double_vec.size(); i++) {
			this_double_vec[i] = cp_double_vec[i];
		}
	}

	if (!this_float_vec.empty()) {
		// Calculate a suitable position for the cross-over
		std::size_t pos = this->m_uniform_int(m_gr, std::uniform_int_distribution<std::size_t>::param_type(std::size_t(0), this_float_vec.size() - std::size_t(1)));

		// Perform the actual cross-over operation
		for (std::size_t i = pos; i < this_float_vec.size(); i++) {
			this_float_vec[i] = cp_float_vec[i];
		}
	}

	if (!this_bool_vec.empty()) {
		// Calculate a suitable position for the cross-over
		std::size_t pos = this->m_uniform_int(m_gr, std::uniform_int_distribution<std::size_t>::param_type(std::size_t(0), this_bool_vec.size() - std::size_t(1)));

		// Perform the actual cross-over operation
		for (std::size_t i = pos; i < this_bool_vec.size(); i++) {
			this_bool_vec[i] = cp_bool_vec[i];
		}
	}

	if (!this_int_vec.empty()) {
		// Calculate a suitable position for the cross-over
		std::size_t pos = this->m_uniform_int(m_gr, std::uniform_int_distribution<std::size_t>::param_type(std::size_t(0), this_int_vec.size() - std::size_t(1)));

		// Perform the actual cross-over operation
		for (std::size_t i = pos; i < this_int_vec.size(); i++) {
			this_int_vec[i] = cp_int_vec[i];
		}
	}

	// Load the data vectors back into this object
	this->assignValueVector(this_double_vec);
	this->assignValueVector(this_float_vec);
	this->assignValueVector(this_bool_vec);
	this->assignValueVector(this_int_vec);

	// Mark this individual as "dirty"
	this->setDirtyFlag(true);
}

/******************************************************************************/
/**
 * Allows to set the "per item" cross-over probability
 */
void GParameterSet::setPerItemCrossOverProbability(double perItemCrossOverProbability) {
	if (perItemCrossOverProbability < 0. || perItemCrossOverProbability > 1.) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GParameterSet::setPerItemCrossOverProbability(" << perItemCrossOverProbability << "): Error!" << std::endl
				<< "Variable outside of allowed ranged [0:1]" << std::endl
		);
	}

	m_perItemCrossOverProbability = perItemCrossOverProbability;
}

/******************************************************************************/
/**
 * Allows to retrieve the "per item" cross-over probability
 */
double GParameterSet::getPerItemCrossOverProbability() const {
	return m_perItemCrossOverProbability;
}

/******************************************************************************/
/**
 * Triggers updates of adaptors contained in this object.
 */
void GParameterSet::updateAdaptorsOnStall(const std::uint32_t &nStalls) {
	for(auto& item_ptr: *this) {
		item_ptr->updateAdaptorsOnStall(nStalls);
	}
}

/******************************************************************************/
/**
 * Retrieves information from adaptors with a given property
 *
 * @param adaoptorName The name of the adaptor to be queried
 * @param property The property for which information is sought
 * @param data A vector, to which the properties should be added
 */
void GParameterSet::queryAdaptor(
	const std::string &adaptorName
	, const std::string &property
	, std::vector<boost::any> &data
) const {
	for(const auto& item_ptr: *this) {
		item_ptr->queryAdaptor(adaptorName, property, data);
	}
}

/******************************************************************************/
/**
 * Retrieves parameters relevant for the evaluation from another GParameterSet.
 * NOTE: The other parameter set will be an empty shell afterwards. The function may
 * only be called for "clean" foreign parameter sets
 */
void GParameterSet::cannibalize(GParameterSet& cp) {
	// Check whether the "foreign" entity has the dirty flag set
	if(cp.isDirty()) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GParameterSet::cannibalize(const GParameterSet& cp): Error!" << std::endl
				<< "cp has the dirty flag set" << std::endl
		);
	}

	// Make sure we have no local parameters
	this->clear();

	// Copy all "foreign" parameters over
	for(const auto& t_ptr: cp) {
		this->push_back(t_ptr);
	}

	// Empty the foreign GParmaeterSet object
	cp.clear();

	// Set our own fitness according to the foreign individual. This will also
	// clear our local dirty flag (if set).
	this->setFitness_(cp.fitnessVec());
}

/******************************************************************************/
/**
 * The adaption interface. Triggers adaption of the individual, using each parameter object's
 * adaptor. Sets the dirty flag, as the parameters have been changed. This facility is mostly
 * used in Evolutionary Algorithms and Simulated Annealing. Other algorithms, such as
 * PSO and Gradient Descents, may choose to change parameters directly. Adaptions will be performed
 * until actual changes were done to the object AND a valid parameter set was found.
 */
std::size_t GParameterSet::adapt() {
	std::size_t nAdaptionAttempts = 0;
	std::size_t nAdaptions = 0; // This is a measure of the "effective" adaption probability
	std::size_t nInvalidAdaptions = 0;
	double validity = 0;

	// Perform adaptions until a valid solution was find. In the context
	// of evolutionary algorithms, this process is indeed equivalent to
	// a larger population, if invalid solutions were produced. The downside
	// may be, that the algorithm moves closer to MUPLUSNU. Thus, if you find
	// yourself stuck in local optima too often, consider setting m_max_retries_until_valid
	// to 0, using the appropriate function.
	while (true) {
		// Make sure at least one modification is performed. E.g., for low
		// adaption probabilities combined with few parameters, it may happen
		// otherwise that individuals remain unchanged after a call to adapt()
		while (true) { // Try again if no adaption has taken place
			// Perform the actual adaption
			nAdaptions = this->customAdaptions();

			// Terminate, if at least one adaption was performed
			if (nAdaptions > 0) {
				break;
			}

			// Terminate, if the maximum number of adaptions has been exceeded
			if (m_max_unsuccessful_adaptions > 0 && ++nAdaptionAttempts > m_max_unsuccessful_adaptions) {
				break;
			}
		}

		if (
			this->parameterSetFulfillsConstraints(validity)
			|| ++nInvalidAdaptions > m_max_retries_until_valid
			) {
			break;
		}
	}

	// Make sure the individual is re-evaluated when fitness(...) is called next time
	this->setDirtyFlag();

	// Store the number of adaptions for later use and let the audience know
	return (m_n_adaptions=nAdaptions);
}

/* ----------------------------------------------------------------------------------
 * Tested in GTestIndividual1::specificTestsNoFailureExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Returns the cached result of the fitness function with id 0. This function
 * will always return the raw fitness, as it is likely the one called by users
 * directly -- they will expect untransformed values. This is the const version
 */
double GParameterSet::fitness() const {
	return fitness(0, PREVENTREEVALUATION, USERAWFITNESS);
}

/******************************************************************************/
/**
 * Calculate or returns the result of a fitness function with a given id.This
 * function will always return the raw fitness, as it is likely the one called by users
 * directly -- they will expect untransformed values. This is the const version
 */
double GParameterSet::fitness(const std::size_t &id) const {
	return fitness(id, PREVENTREEVALUATION, USERAWFITNESS);
}

/******************************************************************************/
/**
 * Returns the last known fitness calculations of this object. Re-calculation
 * of the fitness is triggered, if the "dirty flag" is set, unless this is the server mode.
 * By means of supplying an id it is possible to distinguish between different target functions.
 * 0 denotes the main fitness criterion. The user can specify whether he/she is interested
 * in the transformed or the raw fitness value.
 *
 * @param id The id of the fitness criterion
 * @param reevaluationAllowed Explicit permission to re-evaluate the individual
 * @param useTransformedFitness Whether the transformed or the raw fitness should be returned
 * @return The fitness of this individual
 */
double GParameterSet::fitness(
	const std::size_t &id
	, bool reevaluationAllowed
	, bool useTransformedFitness
) {
	// Check whether we need to recalculate the fitness
	if (true==m_dirty_flag) {
		if (reevaluationAllowed) {
			this->enforceFitnessUpdate();

#ifdef DEBUG
			// Check if the dirty flag is still set. This should only happen in special cases
			if(true==m_dirty_flag) { // Note that m_dirty_flag may also assume the state boost::logic::indeterminate, if evaluation was delayed
				throw gemfony_exception(
					g_error_streamer(DO_LOG,  time_and_place)
						<< "In GParameterSet::fitness(...): Error!" << std::endl
						<< "Dirty flag is still set in a location where it shouldn't be" << std::endl
				);
			}
#endif /* DEBUG */
		} else {
			throw gemfony_exception(
				g_error_streamer(DO_LOG,  time_and_place)
					<< "In GParameterSet::fitness():" << std::endl
					<< "Tried to perform re-evaluation when this action was not allowed" << std::endl
			);
		}
	}

	// Return the desired result -- there should be no situation where the dirtyFlag is still set
	return getCachedFitness(id, useTransformedFitness);
}

/* ----------------------------------------------------------------------------------
 * Tested in GTestIndividual1::specificTestsNoFailureExpected_GUnitTests()
 * Test for throw in serverMode tested in GTestIndividual1::specificTestsFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Returns the last known fitness calculations of this object. This is the
 * const version of the general fitness() function, which consequently cannot
 * trigger re-evaluation, if the individual is dirty. Hence the function will
 * throw, when it is called on a dirty individual (unless we use the
 * USEWORSTKNOWNVALIDFORINVALID policy)
 *
 * @param id The id of the fitness criterion
 * @param reevaluationAllowed Explicit permission to re-evaluate the individual
 * @param useTransformedFitness Whether the transformed or the raw fitness should be returned
 * @return The fitness of this individual
 */
double GParameterSet::fitness(
	const std::size_t &id
	, bool reevaluationAllowed
	, bool useTransformedFitness
) const {
#ifdef DEBUG
	// This function should only be called for clean (i.e. evaluated) individuals
	if(!this->isClean()) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GParameterSet::fitness(...) const: Error!" << std::endl
				<< "Dirty flag is still set in a location where it shouldn't be" << std::endl
		);
	}
#endif /* DEBUG */

	// Return the desired result -- there should be no situation where the dirtyFlag is still set
	return getCachedFitness(id, useTransformedFitness);
}

/******************************************************************************/
/**
 * Returns the transformed result of the fitness function with id 0
 */
double GParameterSet::transformedFitness() const {
	return this->transformedFitness(0);
}

/******************************************************************************/
/**
 * Returns the transformed result of the fitness function with id 0
 */
double GParameterSet::transformedFitness(const std::size_t &id) const {
	return fitness(id, PREVENTREEVALUATION, USETRANSFORMEDFITNESS);
}

/******************************************************************************/
/**
 * Returns a fitness targetted at optimization algorithms, taking into account maximization and minimization
 */
double GParameterSet::minOnly_fitness() const {
	return this->minOnly_fitness(0);
}

/******************************************************************************/
/**
 * Returns a fitness targetted at optimization algorithms, taking into account maximization and minimization
 */
double GParameterSet::minOnly_fitness(const std::size_t &id) const {
	double f = fitness(id, PREVENTREEVALUATION, USETRANSFORMEDFITNESS);

	if (true == this->getMaxMode()) { // Negation will transform maximization problems into minimization problems
		if (boost::numeric::bounds<double>::highest() == f) {
			return boost::numeric::bounds<double>::lowest();
		} else if (boost::numeric::bounds<double>::lowest() == f) {
			return boost::numeric::bounds<double>::highest();
		} else {
			return -f;
		}
	} else {
		return f;
	}
}

/******************************************************************************/
/**
 * Returns all raw fitness results in a std::vector
 */
std::vector<double> GParameterSet::fitnessVec() const {
	std::size_t nFitnessCriteria = this->getNumberOfFitnessCriteria();
	std::vector<double> resultVec;

	for (std::size_t i = 0; i < nFitnessCriteria; i++) {
		resultVec.push_back(this->fitness(i));
	}

	return resultVec;
}

/******************************************************************************/
/**
 * Returns all raw or transformed fitness results in a std::vector
 */
std::vector<double> GParameterSet::fitnessVec(bool useRawFitness) const {
	std::size_t nFitnessCriteria = this->getNumberOfFitnessCriteria();
	std::vector<double> resultVec;

	for (std::size_t i = 0; i < nFitnessCriteria; i++) {
		if (useRawFitness) {
			resultVec.push_back(this->fitness(i));
		} else {
			resultVec.push_back(this->transformedFitness(i));
		}
	}

	return resultVec;
}

/******************************************************************************/
/**
 * Returns all transformed fitness results in a std::vector
 */
std::vector<double> GParameterSet::transformedFitnessVec() const {
	std::size_t nFitnessCriteria = this->getNumberOfFitnessCriteria();
	std::vector<double> resultVec;

	for (std::size_t i = 0; i < nFitnessCriteria; i++) {
		resultVec.push_back(this->transformedFitness(i));
	}

	return resultVec;
}

/******************************************************************************/
/**
 * A wrapper for the non-const fitness function, so we can bind to it. It is
 * needed as bind cannot distinguish between the non-const and const
 * overload of the fitness() function.
 */
double GParameterSet::nonConstFitness(
	const std::size_t &id, bool reevaluationAllowed, bool useTransformedFitness
) {
	return this->fitness(id, reevaluationAllowed, useTransformedFitness);
}

/******************************************************************************/
/**
 * A wrapper for the const fitness function, so we can bind to it. It is
 * needed as bind cannot distinguish between the non-const and const
 * overload of the fitness() function.
 */
double GParameterSet::constFitness(
	const std::size_t &id, bool reevaluationAllowed, bool useTransformedFitness
) const {
	return this->fitness(id, reevaluationAllowed, useTransformedFitness);
}

/******************************************************************************/
/**
 * Retrieve the current (not necessarily up-to-date) fitness
 */
double GParameterSet::getCachedFitness(
	const std::size_t &id, const bool &useTransformedFitness
) const {
	if (useTransformedFitness) {
		return std::get<G_TRANSFORMED_FITNESS>(m_current_fitness_vec.at(id));
	} else {
		return std::get<G_RAW_FITNESS>(m_current_fitness_vec.at(id));
	}
}

/******************************************************************************/
/**
 * Enforces re-calculation of the fitness values.
 */
void GParameterSet::enforceFitnessUpdate(std::function<std::vector<double>()> f) {
	// Assign a new evaluation id
	m_evaluation_id = std::string("eval_") + Gem::Common::to_string(boost::uuids::random_generator()());

	// We start a new evaluation. Make sure the object isn't marked as invalid,
	// and that this state cannot be changed.
	m_marked_as_invalid_by_user.reset();

	// Find out, whether this is a valid solution
	if (
		this->parameterSetFulfillsConstraints(m_validity_level) // Needs to be called first, or else the m_validity_level will not be filled
		|| evaluationPolicy::USESIMPLEEVALUATION == m_eval_policy
		) {
		// Marking individuals as invalid happens inside of the user-supplied fitness
		// calculation (if at all) so we want to reset the corresponding "invalid" flag
		m_marked_as_invalid_by_user.unlockWithValue(OE_NOT_MARKED_AS_INVALID);

		if (f) {
			std::vector<double> fitnessVec = f();

			// Use the external evaluation function (needed e.g. when using a GPGPU for the evaluation step)
#ifdef DEBUG
			if(fitnessVec.size() != getNumberOfFitnessCriteria()) {
				throw gemfony_exception(
					g_error_streamer(DO_LOG,  time_and_place)
						<< "In GParameterSet::enforceFitnessUpdate(): Error!" << std::endl
						<< "Invalid size of external evaluation criteria: " << fitnessVec.size() << " (expected " << getNumberOfFitnessCriteria() << ")" << std::endl
				);
			}
#endif

			// Assign the actual fitness values
			for (std::size_t i = 0; i < getNumberOfFitnessCriteria(); i++) {
				std::get<G_RAW_FITNESS>(m_current_fitness_vec.at(0)) = fitnessVec.at(i);
			}


		} else {
			// Trigger actual fitness calculation using the user-supplied function. This will
			// also register secondary "raw" fitness values used in multi-criterion optimization.
			// Transformed values are taken care of below
			std::get<G_RAW_FITNESS>(m_current_fitness_vec.at(0)) = fitnessCalculation();
		}

		// Lock setting of the variable again, so the invalidity state can only be changed
		// upon re-calculation of the object's values
		m_marked_as_invalid_by_user.lock();

		if (m_marked_as_invalid_by_user || this->allRawResultsAtWorst()) { // Is this an invalid result ?
			// Fill the raw and transformed vectors with the worst case scenario. It is assumed
			// here that marking entire solutions as invalid after the evaluation happens relatively
			// rarely so that a flat "worst" quality surface for such solutions does not hinder
			// progress of the optimization procedure too much
			for (std::size_t i = 0; i < getNumberOfFitnessCriteria(); i++) {
				std::get<G_RAW_FITNESS>(m_current_fitness_vec.at(i)) = this->getWorstCase();
				std::get<G_TRANSFORMED_FITNESS>(m_current_fitness_vec.at(i)) = this->getWorstCase();
			}
		} else { // This is a valid solution nonetheless
			for (std::size_t i = 0; i < this->getNumberOfFitnessCriteria(); i++) {
				if (evaluationPolicy::USESIGMOID == m_eval_policy) { // Update the fitness value to use sigmoidal values
					std::get<G_TRANSFORMED_FITNESS>(m_current_fitness_vec.at(i)) =
						Gem::Common::gsigmoid(std::get<G_RAW_FITNESS>(m_current_fitness_vec.at(i)), m_sigmoid_extremes, m_sigmoid_steepness);
				} else { // All other transformation policies leave valid solutions intact
					std::get<G_TRANSFORMED_FITNESS>(m_current_fitness_vec.at(i)) = std::get<G_RAW_FITNESS>(
						m_current_fitness_vec.at(i));
				}
			}
		}

		// Clear the dirty flag -- all possible evaluation work was done
		//--------------
		setDirtyFlag(false);
		//--------------
	} else { // Some constraints were violated. Act on the chosen policy
		if (evaluationPolicy::USEWORSTCASEFORINVALID == m_eval_policy) {
			for (std::size_t i = 0; i < this->getNumberOfFitnessCriteria(); i++) {
				std::get<G_RAW_FITNESS>(m_current_fitness_vec.at(i)) = this->getWorstCase();
				std::get<G_TRANSFORMED_FITNESS>(m_current_fitness_vec.at(i)) = this->getWorstCase();
			}

			// Clear the dirty flag -- all possible evaluation work was done
			//--------------
			setDirtyFlag(false);
			//--------------
		} else if (evaluationPolicy::USESIGMOID == m_eval_policy) {
			double uniformFitnessValue = 0.;
			if (true == this->getMaxMode()) { // maximize
				if (boost::numeric::bounds<double>::highest() == m_validity_level) {
					uniformFitnessValue = this->getWorstCase();
				} else {
					uniformFitnessValue = -m_validity_level * m_sigmoid_extremes;
				}
			} else { // minimize
				if (boost::numeric::bounds<double>::highest() == m_validity_level) {
					uniformFitnessValue = this->getWorstCase();
				} else {
					uniformFitnessValue = m_validity_level * m_sigmoid_extremes;
				}
			}

			for (std::size_t i = 0; i < getNumberOfFitnessCriteria(); i++) {
				std::get<G_RAW_FITNESS>(m_current_fitness_vec.at(i)) = this->getWorstCase();
				std::get<G_TRANSFORMED_FITNESS>(m_current_fitness_vec.at(i)) = uniformFitnessValue;
			}

			// Clear the dirty flag -- all possible evaluation work was done
			//--------------
			setDirtyFlag(false);
			//--------------
		} else if (evaluationPolicy::USEWORSTKNOWNVALIDFORINVALID == m_eval_policy) {
			// Some of this will be reset later, in  GParameterSet::postEvaluationUpdate().
			// The caller needs to tell us about the worst solution known up to now. It is only
			// known once all individuals of this iteration have been evaluated, i.e. not at this
			// place.
			for (std::size_t i = 0; i < getNumberOfFitnessCriteria(); i++) {
				std::get<G_RAW_FITNESS>(m_current_fitness_vec.at(i)) = this->getWorstCase();
				std::get<G_TRANSFORMED_FITNESS>(m_current_fitness_vec.at(i)) = this->getWorstCase();
			}

			// As only place-holders have been stored in the fitness criteria, the individual
			// is not clean. However, we can tell the audience that evaluation was delayed
			//--------------
			setDirtyFlag(boost::logic::indeterminate);
			//--------------
		}
	}
}

/* ----------------------------------------------------------------------------------
 * untested
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Registers a new, "raw" secondary result value of the custom fitness calculation. This is used in
 * multi-criterion optimization. fitnessCalculation() returns the main fitness value, but may also add further,
 * secondary results. Note that, whether these are actually used, depends on the optimization algorithm being
 * used. Transformation for the second fitness value will be done in the enforceFitnessUpdate function.
 *
 * @param id The position of the fitness criterion (must be > 0 !)
 * @param secondaryValue The secondary fitness value to be registered
 */
void GParameterSet::registerSecondaryResult(
	const std::size_t &id, const double &secondaryValue
) {
#ifdef DEBUG
	if(m_current_fitness_vec.size() <= id || 0==id) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GParameterSet::registerSecondaryResult(...): Error!" << std::endl
				<< "Invalid position in vector: " << id << " (expected min 1 and max " <<  m_current_fitness_vec.size()-1 << ")" << std::endl
		);
	}
#endif /* DEBUG */

	std::get<G_RAW_FITNESS>(m_current_fitness_vec.at(id)) = secondaryValue;
}

/******************************************************************************/
/**
 * Determines the number of fitness criteria present for the individual.
 *
 * @return The number of fitness criteria registered with this individual
 */
std::size_t GParameterSet::getNumberOfFitnessCriteria() const {
	return m_n_fitness_criteria;
}

/******************************************************************************/
/**
 * Allows to reset the number of fitness criteria. Note that this should only
 * be done before the first evaluation takes place. One valid use-case for this
 * function is a factory class associated with an individual. Calling this function
 * will likely result in resized worstKnownValids_ and currentFitnessVec_ vectors.
 * This will result in a need to add best- and worst-case values or the removal
 * of existing values.
 */
void GParameterSet::setNumberOfFitnessCriteria(std::size_t nFitnessCriteria) {
	if (nFitnessCriteria < m_n_fitness_criteria) {
		m_current_fitness_vec.resize(nFitnessCriteria);
		m_worst_known_valids_vec.resize(nFitnessCriteria);
	} else if (nFitnessCriteria > m_n_fitness_criteria) {
		std::tuple<double, double> worstVal, bestVal;

		std::get<G_RAW_FITNESS>(worstVal) = this->getWorstCase();
		std::get<G_TRANSFORMED_FITNESS>(worstVal) = this->getWorstCase();

		std::get<G_RAW_FITNESS>(bestVal) = this->getBestCase();
		std::get<G_TRANSFORMED_FITNESS>(bestVal) = this->getBestCase();

		m_current_fitness_vec.resize(nFitnessCriteria, worstVal);
		m_worst_known_valids_vec.resize(nFitnessCriteria, bestVal);
	} // else do nothing

	m_n_fitness_criteria = nFitnessCriteria;
}

/******************************************************************************/
/**
 * Determines whether more than one fitness criterion is present for this individual
 *
 * @return A boolean indicating whether more than one target function is present
 */
bool GParameterSet::hasMultipleFitnessCriteria() const {
	return (getNumberOfFitnessCriteria() > 1 ? true : false);
}

/******************************************************************************/
/**
 * Checks the worst valid fitness and updates it when needed
 */
void GParameterSet::challengeWorstValidFitness(
	std::tuple<double, double> &worstCandidate, const std::size_t &id
) {
#ifdef DEBUG
	if(id >= this->getNumberOfFitnessCriteria()) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GParameterSet::challengeWorstValidFitness(): Error!" << std::endl
				<< "Requested fitness id " << id << " exceeds allowed range " << this->getNumberOfFitnessCriteria() << std::endl
		);
	}

	if(!this->isClean()) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GParameterSet::challengeWorstValidFitness(): Error!" << std::endl
				<< "Function called for dirty individual or with delayed evaluation" << std::endl
		);
	}

	if(!this->isValid()) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GParameterSet::challengeWorstValidFitness(): Error!" << std::endl
				<< "Function called for invalid individual" << std::endl
		);
	}
#endif /* DEBUG */

	double rawFitness = this->fitness(id, PREVENTREEVALUATION, USERAWFITNESS);
	double transformedFitness = this->fitness(id, PREVENTREEVALUATION, USETRANSFORMEDFITNESS);

	// This rather verbose way of creating eval is done so we do not make mistakes later
	// if raw and transformed fitness change their order
	std::tuple<double, double> eval;
	std::get<G_RAW_FITNESS>(eval) = rawFitness;
	std::get<G_TRANSFORMED_FITNESS>(eval) = transformedFitness;

	if (isWorse(std::get<G_TRANSFORMED_FITNESS>(worstCandidate), std::get<G_TRANSFORMED_FITNESS>(eval))) {
		worstCandidate = eval;
	}
}

/******************************************************************************/
/**
 * Retrieve the fitness tuple at a given evaluation position.
 */
std::tuple<double, double> GParameterSet::getFitnessTuple(const std::uint32_t &id) const {
	return m_current_fitness_vec.at(id);
}

/******************************************************************************/
/**
 * Checks whether this individual is "clean", i.e neither "dirty" nor has a delayed evaluation
 */
bool GParameterSet::isClean() const {
	if (true == m_dirty_flag || boost::logic::indeterminate(m_dirty_flag)) {
		return false;
	} else {
		return true;
	}
}

/******************************************************************************/
/**
 * Checks whether the dirty flag is set
 *
 * @return The value of the dirtyFlag_ variable
 */
bool GParameterSet::isDirty() const {
	return true == m_dirty_flag;
}

/* ----------------------------------------------------------------------------------
 * Tested in GTestIndividual1::specificTestsNoFailureExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Sets the dirtyFlag_. This is a "one way" function, accessible to derived classes. Once the dirty flag
 * has been set, the only way to reset it is to calculate the fitness of this object.
 */
void GParameterSet::setDirtyFlag() {
	this->setDirtyFlag(true);
}

/* ----------------------------------------------------------------------------------
 * Tested in GTestIndividual1::specificTestsNoFailureExpected_GUnitTests()
 * Tested in GParameterSet::specificTestsNoFailureExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Checks whether evaluation was delayed
 */
bool GParameterSet::evaluationDelayed() const {
	if (boost::logic::indeterminate(m_dirty_flag)) {
		return true;
	} else {
		return false;
	}
}

/******************************************************************************/
/**
 * Allows to retrieve the maximize_ parameter
 *
 * @return The current value of the maximize_ parameter
 */
bool GParameterSet::getMaxMode() const {
	return m_maximize;
}

/* ----------------------------------------------------------------------------------
 * Retrieval is tested in GParameterSet::specificTestsNoFailureExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/***************************************************************************/
/**
 * Helper function that emits the worst case value depending on whether maximization
 * or minimization is performed.
 *
 * @return The worst case value, depending on maximization or minimization
 */
double GParameterSet::getWorstCase() const {
	return (this->getMaxMode() ? boost::numeric::bounds<double>::lowest() : boost::numeric::bounds<double>::highest());
}

/******************************************************************************/
/**
 * Retrieves the best possible evaluation result, depending on whether we are in
 * maximization or minimization mode
 */
double GParameterSet::getBestCase() const {
	return (this->getMaxMode() ? boost::numeric::bounds<double>::highest() : boost::numeric::bounds<double>::lowest());
}

/******************************************************************************/
/**
 * Retrieves the steepness_ variable (used for the sigmoid transformation)
 */
double GParameterSet::getSteepness() const {
	return m_sigmoid_steepness;
}

/******************************************************************************/
/**
 * Sets the steepness variable (used for the sigmoid transformation)
 */
void GParameterSet::setSteepness(double steepness) {
	if (steepness <= 0.) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GParameterSet::setSteepness(double steepness): Error!" << std::endl
				<< "Invalid value of steepness parameter: " << steepness << std::endl
		);
	}

	m_sigmoid_steepness = steepness;
}

/******************************************************************************/
/**
 * Retrieves the barrier_ variable (used for the sigmoid transformation)
 */
double GParameterSet::getBarrier() const {
	return m_sigmoid_extremes;
}

/******************************************************************************/
/**
 * Sets the barrier variable (used for the sigmoid transformation)
 */
void GParameterSet::setBarrier(double barrier) {
	if (barrier <= 0.) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GParameterSet::setBarrier(double barrier): Error!" << std::endl
				<< "Invalid value of barrier parameter: " << barrier << std::endl
		);
	}

	m_sigmoid_extremes = barrier;
}

/******************************************************************************/
/**
 * Sets the maximum number of calls to customAdaptions() that may pass without
 * actual modifications. Setting this to 0 disables this check. You should only
 * do this if you are sure that an adaption will eventually happen. Otherwise
 * you would get an endless loop.
 */
void GParameterSet::setMaxUnsuccessfulAdaptions(std::size_t maxUnsuccessfulAdaptions) {
	m_max_unsuccessful_adaptions = maxUnsuccessfulAdaptions;
}

/******************************************************************************/
/**
 * Retrieves the maximum number of calls to customAdaptions that may pass without
 * actual modifications
 */
std::size_t GParameterSet::getMaxUnsuccessfulAdaptions() const {
	return m_max_unsuccessful_adaptions;
}

/******************************************************************************/
/**
 * Allows to set the maximum number of retries during the adaption of individuals
 * until a valid individual was found. Setting this value to 0 will disable retries.
 */
void GParameterSet::setMaxRetriesUntilValid(
	std::size_t maxRetriesUntilValid
) {
	m_max_retries_until_valid = maxRetriesUntilValid;
}

/******************************************************************************/
/**
 * Allows to retrieve the current maximum number of retries during the adaption of
 * individuals until a valid individual was found.
 */
std::size_t GParameterSet::getMaxRetriesUntilValid() const {
	return m_max_retries_until_valid;
}

/******************************************************************************/
/**
 * Retrieves the number of adaptions performed during the last call to adapt()
 * (or 0, if no adaptions were performed so far).
 */
std::size_t GParameterSet::getNAdaptions() const {
	return m_n_adaptions;
}

/******************************************************************************/
/**
 * Allows to set the current iteration of the parent optimization algorithm.
 *
 * @param parentAlgIteration The current iteration of the optimization algorithm
 */
void GParameterSet::setAssignedIteration(const std::uint32_t &parentAlgIteration) {
	m_assigned_iteration = parentAlgIteration;
}

/* ----------------------------------------------------------------------------------
 * Tested in GParameterSet::specificTestsNoFailureExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Gives access to the parent optimization algorithm's iteration
 *
 * @return The parent optimization algorithm's current iteration
 */
std::uint32_t GParameterSet::getAssignedIteration() const {
	return m_assigned_iteration;
}

/* ----------------------------------------------------------------------------------
 * Tested in GParameterSet::specificTestsNoFailureExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Allows to specify the number of optimization cycles without improvement of the primary fitness criterion
 *
 * @param nStalls The number of optimization cycles without improvement in the parent algorithm
 */
void GParameterSet::setNStalls(const std::uint32_t &nStalls) {
	m_n_stalls = nStalls;
}

/* ----------------------------------------------------------------------------------
 * Tested in GParameterSet::specificTestsNoFailureExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Allows to retrieve the number of optimization cycles without improvement of the primary fitness criterion
 *
 * @return The number of optimization cycles without improvement in the parent algorithm
 */
std::uint32_t GParameterSet::getNStalls() const {
	return m_n_stalls;
}

/* ----------------------------------------------------------------------------------
 * Tested in GParameterSet::specificTestsNoFailureExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Retrieves the current personality of this individual
 *
 * @return An identifier for the current personality of this object
 */
std::string GParameterSet::getPersonality() const {
	if (m_pt_ptr) {
		return m_pt_ptr->name();
	} else {
		return std::string("PERSONALITY_NONE");
	}
}

/* ----------------------------------------------------------------------------------
 * Tested in GParameterSet::specificTestsNoFailureExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * This function returns the current personality traits base pointer. Note that there
 * is another version of the same command that does on-the-fly conversion of the
 * personality traits to the derived class.
 *
 * @return A shared pointer to the personality traits base class
 */
std::shared_ptr <GPersonalityTraits> GParameterSet::getPersonalityTraits() {
#ifdef DEBUG
	// Do some error checking
	if(!m_pt_ptr) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GParameterSet::getPersonalityTraits():" << std::endl
				<< "Pointer to personality traits object is empty." << std::endl
		);
	}
#endif

	return m_pt_ptr;
}

/* ----------------------------------------------------------------------------------
 * Tested in GParameterSet::specificTestsNoFailureExpected_GUnitTests()
 * Tested in GParameterSet::specificTestsFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Sets the current personality of this individual
 *
 * @param gpt A pointer to an object representing the new personality of this object
 */
void GParameterSet::setPersonality(
	std::shared_ptr < GPersonalityTraits > gpt
) {
	// Make sure we haven't been given an empty pointer
	if (!gpt) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GParameterSet::setPersonality(): Error!" << std::endl
				<< "Received empty personality traits pointer" << std::endl
		);
	}

	// Add the personality traits object to our local pointer
	m_pt_ptr = gpt;
}


/* ----------------------------------------------------------------------------------
 * Tested in GParameterSet::specificTestsNoFailureExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Resets the current personality to PERSONALITY_NONE
 */
void GParameterSet::resetPersonality() {
	m_pt_ptr.reset();
}

/* ----------------------------------------------------------------------------------
 * Tested in GParameterSet::specificTestsNoFailureExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Retrieves the mnemonic used for the optimization of this object
 */
std::string GParameterSet::getMnemonic() const {
	if(m_pt_ptr) {
		return m_pt_ptr->getMnemonic();
	} else {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GParameterSet::getMnemonic():" << std::endl
				<< "Pointer to personality traits object is empty." << std::endl
		);
	}

	// Make the compiler happy
	return std::string();
}

/******************************************************************************/
/**
 * Adds local configuration options to a GParserBuilder object
 *
 * @param gpb The GParserBuilder object to which configuration options should be added
 */
void GParameterSet::addConfigurationOptions(
	Gem::Common::GParserBuilder &gpb
) {
// Call our parent class'es function
	GObject::addConfigurationOptions(gpb);

	// Add local data
	gpb.registerFileParameter<evaluationPolicy>(
		"evalPolicy" // The name of the variable
		, Gem::Geneva::evaluationPolicy::USESIMPLEEVALUATION // The default value
		, [this](evaluationPolicy ep) { this->setEvaluationPolicy(ep); }
	)
		<< "Specifies which strategy should be used to calculate the evaluation:" << std::endl
		<< "0 (a.k.a. USESIMPLEEVALUATION): Always call the evaluation function, even for invalid solutions" << std::endl
		<< "1 (a.k.a. USEWORSTCASEFORINVALID) : Assign the worst possible value to our fitness and evaluate only valid solutions" << std::endl
		<< "2 (a.k.a. USESIGMOID): Assign a multiple of m_validity_level and sigmoid barrier to invalid solutions, apply a sigmoid function to valid evaluations" << std::endl
		<< "3 (a.k.a. USEWORSTKNOWNVALIDFORINVALID): Assign \"invalidityLevel*worstKnownValid\" to invalid individuals, using normal evaluation otherwise";

	gpb.registerFileParameter<double>(
		"steepness" // The name of the variable
		, Gem::Geneva::FITNESSSIGMOIDSTEEPNESS // The default value
		, [this](double ss) { this->setSteepness(ss); }
	)
		<< "When using a sigmoid function to transform the individual's fitness," << std::endl
		<< "this parameter influences the steepness of the function at the center of the sigmoid." << std::endl
		<< "The parameter must have a value > 0.";

	gpb.registerFileParameter<double>(
		"barrier" // The name of the variable
		, Gem::Geneva::WORSTALLOWEDVALIDFITNESS // The default value
		, [this](double barrier) { this->setBarrier(barrier); }
	)
		<< "When using a sigmoid function to transform the individual's fitness," << std::endl
		<< "this parameter sets the upper/lower boundary of the sigmoid." << std::endl
		<< "The parameter must have a value > 0.;";

	gpb.registerFileParameter<std::size_t>(
		"maxUnsuccessfulAdaptions" // The name of the variable
		, DEFMAXUNSUCCESSFULADAPTIONS // The default value
		, [this](std::size_t mua) { this->setMaxUnsuccessfulAdaptions(mua); }
	)
		<< "The maximum number of unsuccessful adaptions in a row for one call to adapt()";

	gpb.registerFileParameter<std::size_t>(
		"maxRetriesUntilValid" // The name of the variable
		, DEFMAXRETRIESUNTILVALID // The default value
		, [this](std::size_t mruv) { this->setMaxRetriesUntilValid(mruv); }
	)
		<< "The maximum allowed number of retries during the" << std::endl
		<< "adaption of individuals until a valid solution was found" << std::endl
		<< "A parameter set is considered to be \"valid\" if" << std::endl
		<< "it passes all validity checks;";

	// Add local data
	gpb.registerFileParameter<bool>(
		"maximize" // The name of the variable
		, false // The default value
		, [this](bool mm) { this->setMaxMode(mm); }
	)
		<< "Specifies whether the individual should be maximized (1) or minimized (0)" << std::endl
		<< "Note that minimization is the by far most common option.";

	gpb.registerFileParameter<double>(
		"perItemCrossOverProbability" // The name of the variable
		, DEFAULTPERITEMEXCHANGELIKELIHOOD // The default value
		, [this](double piel) { this->setPerItemCrossOverProbability(piel); }
	)
		<< "The likelihood for two data items to be exchanged";
}

/******************************************************************************/
/**
 * Check how valid a given solution is
 *
 * @return The validity level of this solution
 */
double GParameterSet::getValidityLevel() const {
	return m_validity_level;
}

/******************************************************************************/
/**
 * @return A boolean indicating, whether all constraints were fulfilled
 */
bool GParameterSet::constraintsFulfilled() const {
	if (m_validity_level <= 1.) return true;
	else return false;
}

/******************************************************************************/
/**
 * Allows to register a constraint with this individual. Note that the constraint
 * object will be cloned.
 */
void GParameterSet::registerConstraint(
	std::shared_ptr<GPreEvaluationValidityCheckT<GParameterSet>> c_ptr
) {
	if (!c_ptr) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GParameterSet::registerConstraint(): Error!" << std::endl
				<< "Tried to register empty constraint object" << std::endl
		);
	}

	// We store clones, so individual objects do not share the same object
	m_individual_constraint_ptr = c_ptr->GObject::clone<GPreEvaluationValidityCheckT<GParameterSet>>();
}

/******************************************************************************/
/**
 * Allows to set the policy to use in case this individual represents an invalid solution
 */
void GParameterSet::setEvaluationPolicy(evaluationPolicy evalPolicy) {
	m_eval_policy = evalPolicy;
}

/******************************************************************************/
/**
 * Allows to retrieve the current policy in case this individual represents an invalid solution
 */
evaluationPolicy GParameterSet::getEvaluationPolicy() const {
	return m_eval_policy;
}


/******************************************************************************/
/**
 * Checks whether this solution is valid. This function is meant to be called
 * for "clean" individuals only and will throw when called for individuals, whose
 * dirty flag is set. Note that it is well possible to call the function if
 * evaluation was delayed.
 */
bool GParameterSet::isValid() const {
	if (this->isDirty()) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GParameterSet::isValid(): Error!" << std::endl
				<< "Function was called while dirty flag was set" << std::endl
		);
	}

	if (m_validity_level <= 1. && !m_marked_as_invalid_by_user && !this->allRawResultsAtWorst()) {
		return true;
	} else {
		return false;
	}
}

/******************************************************************************/
/**
 * Checks whether this solution is invalid
 */
bool GParameterSet::isInValid() const {
	if (m_validity_level > 1. || m_marked_as_invalid_by_user || this->allRawResultsAtWorst()) {
		return true;
	} else {
		return false;
	}
}

/******************************************************************************/
/**
 * Allows an optimization algorithm to set the worst known valid (primary and secondary
 * evaluation up to the current iteration. Note that these are not the best evaluations
 * for a single evaluation criterion, but the worst evaluations for all individuals that
 * were visited so far. Of the std::tuple, the first value signifies the untransformed
 * value, the second value the (possibly transformed) evaluation.
 */
void GParameterSet::setWorstKnownValid(
	const std::vector<std::tuple<double, double>> &worstKnownValid
) {
	m_worst_known_valids_vec = worstKnownValid;
}

/******************************************************************************/
/**
 * Allows to retrieve the worst known valid evaluation up to the current iteration,
 * as set by an external optimization algorithm, at a given position.
 */
std::tuple<double, double> GParameterSet::getWorstKnownValid(
	const std::uint32_t &id
) const {
#ifdef DEBUG
	throw gemfony_exception(
		g_error_streamer(DO_LOG,  time_and_place)
			<< "In GParameterSet::getWorstKnownValid(" << id << "): Error!" << std::endl
			<< "Expected id of max " << m_worst_known_valids_vec.size() - 1 << std::endl
	);
#endif /* DEBUG */

	return m_worst_known_valids_vec.at(id);
}

/******************************************************************************/
/**
 * Allows to retrieve all worst known valid evaluations up to the
 * current iteration, as set by an external optimization algorithm
 */
std::vector<std::tuple<double, double>> GParameterSet::getWorstKnownValids() const {
	return m_worst_known_valids_vec;
}

/******************************************************************************/
/**
 * Fills the worstKnownValid-vector with best values. This function assumes all
 * fitness criteria have been made known already.
 */
void GParameterSet::populateWorstKnownValid() {
#ifdef DEBUG
	if(m_worst_known_valids_vec.size() != m_n_fitness_criteria) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GParameterSet::populateWorstKnownValid(): Error!" << std::endl
				<< "Invalid size of m_worst_known_valids_vec: " << m_worst_known_valids_vec.size() << " (expected " << m_n_fitness_criteria << ")" << std::endl
		);
	}
#endif /* DEBUG */

	for (std::size_t i = 0; i < m_n_fitness_criteria; i++) {
		m_worst_known_valids_vec.at(i) = std::tuple<double, double>(this->getBestCase(), this->getBestCase());
	}
}

/******************************************************************************/
/**
 * Triggers an update of the internal evaluation, if necessary.
 */
void GParameterSet::postEvaluationUpdate() {
#ifdef DEBUG
	if((m_n_fitness_criteria) != m_current_fitness_vec.size()) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GParameterSet::postEvaluationUpdate(): Error!" << std::endl
				<< "Number of expected fitness criteria " << m_n_fitness_criteria << " does not match actual number " << m_current_fitness_vec.size() << std::endl
		);
	}

	if(m_worst_known_valids_vec.empty()) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GParameterSet::postEvaluationUpdate(): Error!" << std::endl
				<< "m_worst_known_valids_vec does not seem to be initialized" << std::endl
		);
	}
#endif /* DEBUG */

	if (evaluationPolicy::USEWORSTKNOWNVALIDFORINVALID == m_eval_policy && this->isInValid()) {
		if (true == m_maximize) {
			for (std::size_t i = 0; i < m_n_fitness_criteria; i++) {
				if (boost::numeric::bounds<double>::highest() == m_validity_level ||
					 boost::numeric::bounds<double>::lowest() == m_validity_level) {
					std::get<G_TRANSFORMED_FITNESS>(m_current_fitness_vec.at(i)) = this->getWorstCase();
				} else {
					std::get<G_TRANSFORMED_FITNESS>(m_current_fitness_vec.at(i))
						= -(std::max)(std::get<G_TRANSFORMED_FITNESS>(m_worst_known_valids_vec.at(i)), (std::max)(m_sigmoid_extremes, 1.)) *
						  m_validity_level;
				}
			}
		} else {
			for (std::size_t i = 1; i < m_n_fitness_criteria; i++) {
				if (boost::numeric::bounds<double>::highest() == m_validity_level ||
					 boost::numeric::bounds<double>::lowest() == m_validity_level) {
					std::get<G_TRANSFORMED_FITNESS>(m_current_fitness_vec.at(i)) = this->getWorstCase();
				} else {
					std::get<G_TRANSFORMED_FITNESS>(m_current_fitness_vec.at(i))
						= (std::max)(std::get<G_TRANSFORMED_FITNESS>(m_worst_known_valids_vec.at(i)), (std::max)(m_sigmoid_extremes, 1.)) *
						  m_validity_level;
				}
			}
		}

		// Note: the "rawFitness" values have already been set in enforceFitnessUpdate() to the worst known evaluation

		// Make sure the dirty flag is set to false, so our results do not get overwritten
		this->setDirtyFlag(false);
	}
}

/******************************************************************************/
/**
 * Allows to set the globally best known primary fitness so far
 *
 * @param bnf The best known primary fitness so far
 */
void GParameterSet::setBestKnownPrimaryFitness(
	const std::tuple<double, double> &bnf
) {
	m_best_past_primary_fitness = bnf;
}

/* ----------------------------------------------------------------------------------
 * Tested in GParameterSet::specificTestsNoFailureExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Retrieves the value of the globally best known primary fitness so far
 *
 * @return The best known primary fitness so far
 */
std::tuple<double, double> GParameterSet::getBestKnownPrimaryFitness() const {
	return m_best_past_primary_fitness;
}

/* ----------------------------------------------------------------------------------
 * Tested in GParameterSet::specificTestsNoFailureExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Retrieve the id assigned to the current evaluation
 */
std::string GParameterSet::getCurrentEvaluationID() const {
	return m_evaluation_id;
}
/******************************************************************************/
/**
 * Helps to determine whether a given value is strictly better (i.e. better than equal)
 * than another one. As "better" means something different for maximization and minimization,
 * this function helps to make the code easier to understand.
 *
 * @param newValue The new value
 * @param oldValue The old value
 * @return true if newValue is better than oldValue, otherwise false.
 */
bool GParameterSet::isBetter(double newValue, const double &oldValue) const {
	if (this->getMaxMode()) {
		if (newValue > oldValue) return true;
		else return false;
	} else { // minimization
		if (newValue < oldValue) return true;
		else return false;
	}
}

/******************************************************************************/
/**
 * Checks whether this object is better than the argument, depending on the maxMode
 */
bool GParameterSet::isBetterThan(std::shared_ptr<GParameterSet> p) const {
	if (this->getMaxMode()) {
		if (this->transformedFitness() > p->transformedFitness()) return true;
		else return false;
	} else { // minimization
		if (this->transformedFitness() < p->transformedFitness()) return true;
		else return false;
	}
}

/******************************************************************************/
/**
 * Checks whether this object is worse than the argument, depending on the maxMode
 */
bool GParameterSet::isWorseThan(std::shared_ptr<GParameterSet> p) const {
	if (this->getMaxMode()) {
		if (this->transformedFitness() < p->transformedFitness()) return true;
		else return false;
	} else { // minimization
		if (this->transformedFitness() > p->transformedFitness()) return true;
		else return false;
	}
}

/******************************************************************************/
/**
 * Helps to determine whether a given value is strictly worse (i.e. worse than equal)
 * than another one. As "worse" means something different for maximization and minimization,
 * this function helps to make the code easier to understand.
 *
 * @param newValue The new value
 * @param oldValue The old value
 * @return true of newValue is worse than oldValue, otherwise false.
 */
bool GParameterSet::isWorse(double newValue, const double &oldValue) const {
	if (this->getMaxMode()) {
		if (newValue < oldValue) return true;
		else return false;
	} else { // minimization
		if (newValue > oldValue) return true;
		else return false;
	}
}



/******************************************************************************/
/**
 * Performs all necessary (remote-)processing steps for this object.
 */
void GParameterSet::process_() {
	this->fitness(0, Gem::Geneva::ALLOWREEVALUATION, Gem::Geneva::USETRANSFORMEDFITNESS);
}

/******************************************************************************/
/**
 * Overloaded from GProcessingContainerT as a means to return a result from the
 * process()-function after pre-/postprocessing.
 */
double GParameterSet::get_processing_result() const noexcept {
	try {
		return this->fitness();
	} catch(...) {
		glogger
			<< "In GParameterSet::get_processing_result(: Caught exception from const fitness function." << std::endl
			<< "Cannot continue" << std::endl
			<< GTERMINATION;
	}

	// Make the compiker happy
	return 0.;
}

/******************************************************************************/
/**
 * Loads the data of another GParameterSet object, camouflaged as a GObject.
 *
 * TODO: Load GProcessingContainerT data
 *
 * @param cp A copy of another GParameterSet object, camouflaged as a GObject
 */
void GParameterSet::load_(const GObject *cp) {
	// Check that we are dealing with a GParameterSet reference independent of this object and convert the pointer
	const GParameterSet *p_load = Gem::Common::g_convert_and_compare<GObject, GParameterSet>(cp, this);

	// Load the parent class'es data
	GObject::load_(cp);
	Gem::Common::GStdPtrVectorInterfaceT<GParameterBase, GObject>::operator=(*p_load);
	Gem::Courtier::GProcessingContainerT<GParameterSet, double>::load_pc(p_load);

	// and then our local data
	m_perItemCrossOverProbability = p_load->m_perItemCrossOverProbability;
	m_n_fitness_criteria = p_load->m_n_fitness_criteria;
	m_current_fitness_vec = p_load->m_current_fitness_vec;
	m_worst_known_valids_vec = p_load->m_worst_known_valids_vec;
	m_marked_as_invalid_by_user.setValue((p_load->m_marked_as_invalid_by_user).value());
	m_best_past_primary_fitness = p_load->m_best_past_primary_fitness;
	m_n_stalls = p_load->m_n_stalls;
	m_dirty_flag = p_load->m_dirty_flag;
	m_maximize = p_load->m_maximize;
	m_assigned_iteration = p_load->m_assigned_iteration;
	m_validity_level = p_load->m_validity_level;
	m_eval_policy = p_load->m_eval_policy;
	m_sigmoid_steepness = p_load->m_sigmoid_steepness;
	m_sigmoid_extremes = p_load->m_sigmoid_extremes;
	m_max_unsuccessful_adaptions = p_load->m_max_unsuccessful_adaptions;
	m_max_retries_until_valid = p_load->m_max_retries_until_valid;
	m_n_adaptions = p_load->m_n_adaptions;
	m_evaluation_id = p_load->m_evaluation_id;

	Gem::Common::copyCloneableSmartPointer(p_load->m_pt_ptr, m_pt_ptr);
	Gem::Common::copyCloneableSmartPointer(p_load->m_individual_constraint_ptr, m_individual_constraint_ptr);
}

/**********************************************************************************/
/**
 * The actual adaption operations. Easy, as we know that all objects
 * in this collection must implement the adapt() function, as they are
 * derived from the GMutableI class / interface.
 */
std::size_t GParameterSet::customAdaptions() {
	std::size_t nAdaptions = 0;

	GParameterSet::iterator it;
	for (auto par_ptr: *this) {
		nAdaptions += par_ptr->adapt(m_gr);
	}

	return nAdaptions;
}

/* ----------------------------------------------------------------------------------
 * Tested in GTestIndividual1::specificTestsNoFailureExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Specify whether we want to work in maximization (true) or minimization
 * (false) mode. This function is protected. The idea is that GParameterSet provides a public
 * wrapper for this function, so that a user can specify whether he wants to maximize or
 * minimize a given evaluation function. Optimization algorithms, in turn, only check the
 * maximization-mode of the individuals stored in them and set their own maximization mode
 * internally accordingly, using the protected, overloaded function.
 *
 * @param mode A boolean which indicates whether we want to work in maximization or minimization mode
 */
void GParameterSet::setMaxMode_(const bool &mode) {
	m_maximize = mode;
}

/* ----------------------------------------------------------------------------------
 * Setting is tested in GOptimizableEntity::specificTestsNoFailureExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * The actual fitness calculation takes place here. Note: This function is a trap. You need to overload
 * this function in derived classes.
 *
 * @return The fitness of this object
 */
double GParameterSet::fitnessCalculation() {
	throw gemfony_exception(
		g_error_streamer(DO_LOG,  time_and_place)
			<< "In GParameterSet::fitnessCalculation()" << std::endl
			<< "Function called directly which should not happen" << std::endl
	);

	// Make the compiler happy
	return 0.;
}

/******************************************************************************/
/**
 * Sets the fitness to a given set of values and clears the dirty flag. This is meant
 * to be used by external methods of performing the actual evaluation, such as the
 * OpenCL-Consumer. Note that this function assumes that the individual and solution
 * is valid, so it does not currently try to take into account situations where for
 * example constraints are violated. The fitness vector is interpreted as raw fitness
 * values. Hence only SIGMOIDAL transformations are taken into account.
 *
 * @param f_vec A vector of fitness values
 */
void GParameterSet::setFitness_(const std::vector<double> &f_vec) {
#ifdef DEBUG
	if(
		f_vec.size() != this->getNumberOfFitnessCriteria()
		|| m_current_fitness_vec.size() != this->getNumberOfFitnessCriteria()
		) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GParameterSet::setFitness_(...): Error!" << std::endl
				<< "Invalid size of fitness vector: " << std::endl
				<< f_vec.size() << " / " << m_current_fitness_vec.size() << " / expected: " << this->getNumberOfFitnessCriteria() << std::endl
		);
	}
#endif /* DEBUG */

	for (std::size_t i = 0; i < this->getNumberOfFitnessCriteria(); i++) {
		std::get<G_RAW_FITNESS>(m_current_fitness_vec.at(i)) = f_vec.at(i);

		// We need to update the transformed fitness for the USESIGMOID
		// case. All other transformations are handled elsewhere.
		std::get<G_TRANSFORMED_FITNESS>(m_current_fitness_vec.at(i)) =
			evaluationPolicy::USESIGMOID == m_eval_policy
			? Gem::Common::gsigmoid(f_vec.at(i), m_sigmoid_extremes, m_sigmoid_steepness)
			: f_vec.at(i);
	}

	// Clear the dirty flag
	setDirtyFlag(false);
}

/* ----------------------------------------------------------------------------------
 * Tested in GExternalSetterIndividual::specificTestsNoFailureExpected_GUnitTests()
 * and GExternalSetterIndividual::specificTestsFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Sets the dirtyFlag_ to any desired value
 *
 * @param dirtyFlag The new value for the dirtyFlag_ variable
 * @return The previous value of the dirtyFlag_ variable
 */
boost::logic::tribool GParameterSet::setDirtyFlag(
	const boost::logic::tribool &dirtyFlag
) {
	bool previous = m_dirty_flag;
	m_dirty_flag = dirtyFlag;

	if(true==m_dirty_flag) {
		this->reset_processing_status(Gem::Courtier::processingStatus::DO_PROCESS);
	} // TODO: Is this correct for delayed execution ?

	return previous;
}

/* ----------------------------------------------------------------------------------
 * Tested in GParameterSet::specificTestsNoFailureExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Combines evaluation results by adding the individual results
 *
 *  @return The result of the combination
 */
double GParameterSet::sumCombiner() const {
	double result = 0.;
	std::vector<std::tuple<double, double>>::const_iterator cit;
	for (cit = m_current_fitness_vec.begin(); cit != m_current_fitness_vec.end(); ++cit) {
		result += std::get<G_TRANSFORMED_FITNESS>(*cit);
	}
	return result;
}

/******************************************************************************/
/**
 * Combines evaluation results by adding the absolute values of individual results
 *
 *  @return The result of the combination
 */
double GParameterSet::fabsSumCombiner() const {
	double result = 0.;
	std::vector<std::tuple<double, double>>::const_iterator cit;
	for (cit = m_current_fitness_vec.begin(); cit != m_current_fitness_vec.end(); ++cit) {
		result += fabs(std::get<G_TRANSFORMED_FITNESS>(*cit));
	}
	return result;
}

/******************************************************************************/
/**
 * Combines evaluation results by calculating the square root of the squared sum.
 * It is assumed that the result of this function is returned as
 * the main result of the fitnessCalculation() function.
 *
 * @return The result of the combination
 */
double GParameterSet::squaredSumCombiner() const {
	double result = 0.;
	std::vector<std::tuple<double, double>>::const_iterator cit;
	for (cit = m_current_fitness_vec.begin(); cit != m_current_fitness_vec.end(); ++cit) {
		result += GSQUARED(std::get<G_TRANSFORMED_FITNESS>(*cit));
	}
	return sqrt(result);
}

/******************************************************************************/
/**
 * Combines evaluation results by calculating the square root of the weighed squared sum. Note that we
 * only evaluate the secondary results here. It is assumed that the result of this function is returned as
 * the main result of the fitnessCalculation() function.
 *
 * @param weights The weights to be multiplied with the cached results
 * @return The result of the combination
 */
double GParameterSet::weighedSquaredSumCombiner(const std::vector<double> &weights) const {
	double result = 0.;
	std::vector<std::tuple<double, double>>::const_iterator cit_eval;
	std::vector<double>::const_iterator cit_weights;

	if (m_current_fitness_vec.size() != weights.size()) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GParameterSet::weighedSquaredSumCombine(): Error!" << std::endl
				<< "Sizes of transformedCurrentFitnessVec_ and the weights vector don't match: " << m_current_fitness_vec.size() <<
				" / " << weights.size() << std::endl
		);
	}

	for (cit_eval = m_current_fitness_vec.begin(), cit_weights = weights.begin();
		  cit_eval != m_current_fitness_vec.end();
		  ++cit_eval, ++cit_weights
		) {
		result += GSQUARED((*cit_weights) * (std::get<G_TRANSFORMED_FITNESS>(*cit_eval)));
	}

	return sqrt(result);
}

/******************************************************************************/
/**
 * Allows users to mark this solution as invalid in derived classes (usually
 * from within the evaluation function)
 */
void GParameterSet::markAsInvalid() {
	if (!m_marked_as_invalid_by_user.isLocked()) {
		m_marked_as_invalid_by_user = true;
	} else {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GParameterSet::markAsInvalid(): Error!" << std::endl
				<< "Tried to mark individual as invalid while changes to this property" << std::endl
				<< "were not allowed. This function may only be called from inside the" << std::endl
				<< "fitness evaluation!" << std::endl
		);
	}
}

/******************************************************************************/
/**
 * Allows to check whether this solution was marked as invalid
 */
bool GParameterSet::markedAsInvalidByUser() const {
	return m_marked_as_invalid_by_user;
}


/******************************************************************************/
/**
 * Checks whether this solution fulfills the set of constraints. Note that this
 * function may be called prior to evaluation in order to check
 */
bool GParameterSet::parameterSetFulfillsConstraints(double &validityLevel) const {
	if (m_individual_constraint_ptr) {
		return m_individual_constraint_ptr->isValid(this, validityLevel);
	} else { // Always valid, if no constraint object has been registered
		validityLevel = 0.;
		return true;
	}

	// Make the compiler happy
	return false;
}

/******************************************************************************/
/**
 * Creates a deep clone of this object
 *
 * @return A deep clone of this object
 */
GObject *GParameterSet::clone_() const {
	return new GParameterSet(*this);
}

/******************************************************************************/
/**
 * Checks whether all results are at the worst possible value
 */
bool GParameterSet::allRawResultsAtWorst() const {
	for (std::size_t i = 0; i < getNumberOfFitnessCriteria(); i++) {
		if (this->getWorstCase() != std::get<G_RAW_FITNESS>(m_current_fitness_vec.at(i))) return false;
	}

	// O.k., so all results are at their worst value
	return true;
}


/******************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GParameterSet::modify_GUnitTests() {
#ifdef GEM_TESTING
	using boost::unit_test_framework::test_suite;
	using boost::unit_test_framework::test_case;

	bool result = false;

	// Call the parent class'es function
	if (GObject::modify_GUnitTests()) result = true;
	if(Gem::Common::GStdPtrVectorInterfaceT<GParameterBase, GObject>::modify_GUnitTests()) result = true;

	for(auto o_ptr: *this) {
		if(o_ptr->modify_GUnitTests()) result = true;
	}

	if(this->randomInit(activityMode::ALLPARAMETERS)) {
		result = true;
	}

	// A relatively harmless change
	m_n_stalls++;
	result = true;

	return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
	Gem::Common::condnotset("GParameterSet::modify_GUnitTests", "GEM_TESTING");
   return false;
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GParameterSet::specificTestsNoFailureExpected_GUnitTests() {
#ifdef GEM_TESTING

	// Access to uniformly distributed double random numbers
	std::uniform_real_distribution<double> uniform_real_distribution;

	// Call the parent class'es function
	GObject::specificTestsNoFailureExpected_GUnitTests();
	Gem::Common::GStdPtrVectorInterfaceT<GParameterBase, GObject>::specificTestsNoFailureExpected_GUnitTests();

	// --------------------------------------------------------------------------

	{ // Test setting and retrieval of the maximization mode flag
		std::shared_ptr<GParameterSet> p_test = this->clone<GParameterSet>();

		BOOST_CHECK_NO_THROW(p_test->setMaxMode_(true));
		BOOST_CHECK(p_test->getMaxMode() == true);
		BOOST_CHECK_NO_THROW(p_test->setMaxMode_(false));
		BOOST_CHECK(p_test->getMaxMode() == false);
	}

	// --------------------------------------------------------------------------

	{ // Check setting of the dirty flag
		std::shared_ptr<GParameterSet> p_test = this->clone<GParameterSet>();

		BOOST_CHECK_NO_THROW(p_test->setDirtyFlag(true));
		BOOST_CHECK(p_test->isDirty() == true);
		BOOST_CHECK_NO_THROW(p_test->setDirtyFlag(false));
		BOOST_CHECK(p_test->isDirty() == false);
		BOOST_CHECK_NO_THROW(p_test->setDirtyFlag());
		BOOST_CHECK(p_test->isDirty() == true); // Note the missing argument -- this is a different function
		BOOST_CHECK_NO_THROW(p_test->setDirtyFlag(false));
		BOOST_CHECK(p_test->isDirty() == false);
		BOOST_CHECK_NO_THROW(p_test->setDirtyFlag(boost::logic::indeterminate));
		BOOST_CHECK(p_test->evaluationDelayed() == true);
		BOOST_CHECK(p_test->isDirty() == false);
		BOOST_CHECK(p_test->isClean() == false);
	}

	// --------------------------------------------------------------------------

	{ // Test setting and retrieval of the surrounding optimization algorithm's current iteration
		std::shared_ptr <GParameterSet> p_test = this->clone<GParameterSet>();

		for (std::uint32_t i = 1; i < 10; i++) {
			BOOST_CHECK_NO_THROW(p_test->setAssignedIteration(i));
			BOOST_CHECK_MESSAGE(
				p_test->getAssignedIteration() == i, "\n"
				<< "p_test->getAssignedIteration() = " <<
				p_test->getAssignedIteration() << "\n"
				<< "i = " << i << "\n"
			);
		}
	}

	// --------------------------------------------------------------------------

	{ // Test setting and retrieval of the best known fitness so far
		std::shared_ptr <GParameterSet> p_test = this->clone<GParameterSet>();

		for (double d = 0.; d < 1.; d += 0.1) {
			BOOST_CHECK_NO_THROW(p_test->setBestKnownPrimaryFitness(std::make_tuple(d, d)));
			BOOST_CHECK_MESSAGE(
				p_test->getBestKnownPrimaryFitness() == std::make_tuple(d, d), "\n"
				<< "p_test->getBestKnownPrimaryFitness() = " <<
				Gem::Common::g_to_string(p_test->getBestKnownPrimaryFitness()) <<
				"\n"
				<< "d = " << d << "\n"
			);
		}
	}

	// --------------------------------------------------------------------------

	{ // Test setting and retrieval of the number of consecutive stalls
		std::shared_ptr <GParameterSet> p_test = this->clone<GParameterSet>();

		for (std::uint32_t i = 1; i < 10; i++) {
			BOOST_CHECK_NO_THROW(p_test->setNStalls(i));
			BOOST_CHECK_MESSAGE(
				p_test->getNStalls() == i, "\n"
				<< "p_test->getNStalls() = " << p_test->getNStalls() << "\n"
				<< "i = " << i << "\n"
			);
		}
	}

	//---------------------------------------------------------------------

	{ // All tests below use the same, cloned collection
		// Some settings for the collection of tests below
		const double MINGCONSTRDOUBLE = -4.;
		const double MAXGCONSTRDOUBLE = 4.;
		const double MINGDOUBLE = -5.;
		const double MAXGDOUBLE = 5.;
		const double MINGDOUBLECOLL = -3.;
		const double MAXGDOUBLECOLL = 3.;
		const std::size_t NGDOUBLECOLL = 10;
		const std::size_t FPLOOPCOUNT = 5;
		const double FPFIXEDVALINITMIN = -3.;
		const double FPFIXEDVALINITMAX = 3.;
		const double FPMULTIPLYBYRANDMIN = -5.;
		const double FPMULTIPLYBYRANDMAX = 5.;
		const double FPADD = 2.;
		const double FPSUBTRACT = 2.;

		// Create a GParameterSet object as a clone of this object for further usage
		std::shared_ptr <GParameterSet> p_test_0 = this->clone<GParameterSet>();
		// Clear the collection
		p_test_0->clear();
		// Make sure it is really empty
		BOOST_CHECK(p_test_0->empty());
		// Add some floating pount parameters
		for (std::size_t i = 0; i < FPLOOPCOUNT; i++) {
			p_test_0->push_back(
				std::shared_ptr<GConstrainedDoubleObject>(
					new GConstrainedDoubleObject(
						uniform_real_distribution(m_gr, std::uniform_real_distribution<double>::param_type(MINGCONSTRDOUBLE, MAXGCONSTRDOUBLE))
						, MINGCONSTRDOUBLE
						, MAXGCONSTRDOUBLE
					)
				)
			);
			p_test_0->push_back(
				std::shared_ptr<GDoubleObject>(
					new GDoubleObject(
						uniform_real_distribution(
							m_gr
							, std::uniform_real_distribution<double>::param_type(MINGDOUBLE, MAXGDOUBLE)
						)
					)
				)
			);
			p_test_0->push_back(std::shared_ptr<GDoubleCollection>(new GDoubleCollection(NGDOUBLECOLL, MINGDOUBLECOLL, MAXGDOUBLECOLL)));
		}

		// Attach a few other parameter types
		p_test_0->push_back(std::shared_ptr<GConstrainedInt32Object>(new GConstrainedInt32Object(7, -10, 10)));
		p_test_0->push_back(std::shared_ptr<GBooleanObject>(new GBooleanObject(true)));

		//-----------------------------------------------------------------

		{ // Test random initialization
			// Create a GParameterSet object as a clone of p_test_0 for further usage
			std::shared_ptr <GParameterSet> p_test = p_test_0->clone<GParameterSet>();

			BOOST_CHECK_NO_THROW(p_test->randomInit(activityMode::ALLPARAMETERS));
			BOOST_CHECK(*p_test != *p_test_0);
		}

		//-----------------------------------------------------------------
		{ // Test initialization of all fp parameters with a fixed value
			for (double d = FPFIXEDVALINITMIN; d < FPFIXEDVALINITMAX; d += 1.) {
				// Create a GParameterSet object as a clone of p_test_0 for further usage
				std::shared_ptr <GParameterSet> p_test = p_test_0->clone<GParameterSet>();

				// Initialize all fp-values with 0.
				p_test->fixedValueInit<double>(d, activityMode::ALLPARAMETERS);

				// Make sure the dirty flag is set
				BOOST_CHECK(p_test->isDirty() == true);

				// Cross-check
				std::size_t counter = 0;
				for (std::size_t i = 0; i < FPLOOPCOUNT; i++) {
					BOOST_CHECK(p_test->at<GConstrainedDoubleObject>(counter)->value() == d);
					counter++;
					BOOST_CHECK(p_test->at<GDoubleObject>(counter)->value() == d);
					counter++;
					std::shared_ptr <GDoubleCollection> p_gdc;
					BOOST_CHECK_NO_THROW(p_gdc = p_test->at<GDoubleCollection>(counter));
					for (std::size_t gdc_cnt = 0; gdc_cnt < NGDOUBLECOLL; gdc_cnt++) {
						BOOST_CHECK_MESSAGE (
							p_gdc->at(gdc_cnt) == d, "\n"
							<< "p_gdc->at(gdc_cnt) = " << p_gdc->at(gdc_cnt) << "\n"
							<< "expected " << d << "\n"
							<< "iteration = " << gdc_cnt << "\n"
						);
					}
					counter++;
				}

				// The int32 parameter should have stayed the same
				std::shared_ptr <GConstrainedInt32Object> p_int32_0;
				std::shared_ptr <GConstrainedInt32Object> p_int32;
				BOOST_CHECK_NO_THROW(p_int32_0 = p_test_0->at<GConstrainedInt32Object>(counter));
				BOOST_CHECK_NO_THROW(p_int32 = p_test->at<GConstrainedInt32Object>(counter));
				BOOST_CHECK(*p_int32_0 == *p_int32);
				counter++;

				// Likewise, the boolean parameter should have stayed the same
				std::shared_ptr <GBooleanObject> p_boolean_orig;
				std::shared_ptr <GBooleanObject> p_boolean_cloned;
				BOOST_CHECK_NO_THROW(p_boolean_orig = p_test_0->at<GBooleanObject>(counter));
				BOOST_CHECK_NO_THROW(p_boolean_cloned = p_test->at<GBooleanObject>(counter));
				BOOST_CHECK(*p_boolean_orig == *p_boolean_cloned);
				counter++;
			}
		}

		//-----------------------------------------------------------------

		{ // Test setting and retrieval of the maximization mode flag
			std::shared_ptr <GParameterSet> p_test = p_test_0->clone<GParameterSet>();

			BOOST_CHECK_NO_THROW(p_test->setMaxMode(true));
			BOOST_CHECK(p_test->getMaxMode() == true);
			BOOST_CHECK_NO_THROW(p_test->setMaxMode(false));
			BOOST_CHECK(p_test->getMaxMode() == false);
		}

		//-----------------------------------------------------------------

		{ // Test multiplication of all fp parameters with a fixed value
			for (double d = -3; d < 3; d += 1.) {
				// Create a GParameterSet object as a clone of p_test_0 for further usage
				std::shared_ptr <GParameterSet> p_test = p_test_0->clone<GParameterSet>();

				// Initialize all fp-values with FPFIXEDVALINITMAX
				BOOST_CHECK_NO_THROW(p_test->fixedValueInit<double>(FPFIXEDVALINITMAX, activityMode::ALLPARAMETERS));

				// Multiply this fixed value by d
				BOOST_CHECK_NO_THROW(p_test->multiplyBy<double>(d, activityMode::ALLPARAMETERS));

				// Make sure the dirty flag is set
				BOOST_CHECK(p_test->isDirty() == true);

				// Cross-check
				std::size_t counter = 0;
				for (std::size_t i = 0; i < FPLOOPCOUNT; i++) {
					// A constrained value does not have to assume the value d*FPFIXEDVALINITMAX,
					// but needs to stay within its boundaries
					BOOST_CHECK(p_test->at<GConstrainedDoubleObject>(counter)->value() >= MINGCONSTRDOUBLE);
					BOOST_CHECK(p_test->at<GConstrainedDoubleObject>(counter)->value() <= MAXGCONSTRDOUBLE);
					counter++;
					BOOST_CHECK(p_test->at<GDoubleObject>(counter)->value() == d * FPFIXEDVALINITMAX);
					counter++;
					std::shared_ptr <GDoubleCollection> p_gdc;
					BOOST_CHECK_NO_THROW(p_gdc = p_test->at<GDoubleCollection>(counter));
					for (std::size_t gdc_cnt = 0; gdc_cnt < NGDOUBLECOLL; gdc_cnt++) {
						BOOST_CHECK_MESSAGE (
							p_gdc->at(gdc_cnt) == d * FPFIXEDVALINITMAX, "\n"
							<< "p_gdc->at(gdc_cnt) = " << p_gdc->at(gdc_cnt) <<
							"\n"
							<< "expected " << d * FPFIXEDVALINITMAX << "\n"
							<< "iteration = " << gdc_cnt << "\n"
						);
					}
					counter++;
				}

				// The int32 parameter should have stayed the same
				std::shared_ptr <GConstrainedInt32Object> p_int32_0;
				std::shared_ptr <GConstrainedInt32Object> p_int32;
				BOOST_CHECK_NO_THROW(p_int32_0 = p_test_0->at<GConstrainedInt32Object>(counter));
				BOOST_CHECK_NO_THROW(p_int32 = p_test->at<GConstrainedInt32Object>(counter));
				BOOST_CHECK(*p_int32_0 == *p_int32);
				counter++;

				// Likewise, the boolean parameter should have stayed the same
				std::shared_ptr <GBooleanObject> p_boolean_orig;
				std::shared_ptr <GBooleanObject> p_boolean_cloned;
				BOOST_CHECK_NO_THROW(p_boolean_orig = p_test_0->at<GBooleanObject>(counter));
				BOOST_CHECK_NO_THROW(p_boolean_cloned = p_test->at<GBooleanObject>(counter));
				BOOST_CHECK(*p_boolean_orig == *p_boolean_cloned);
				counter++;
			}
		}

		//-----------------------------------------------------------------

		{ // Test that fpMultiplyByRandom(min,max) changes every single parameter
			// Create a GParameterSet object as a clone of p_test_0 for further usage
			std::shared_ptr <GParameterSet> p_test = p_test_0->clone<GParameterSet>();

			// Multiply each floating point value with a constrained random value
			BOOST_CHECK_NO_THROW(
				p_test->multiplyByRandom<double>(FPMULTIPLYBYRANDMIN, FPMULTIPLYBYRANDMAX, activityMode::ALLPARAMETERS));

			// Make sure the dirty flag is set
			BOOST_CHECK(p_test->isDirty() == true);

			// Cross-check
			std::size_t counter = 0;
			for (std::size_t i = 0; i < FPLOOPCOUNT; i++) {
				BOOST_CHECK(p_test->at<GConstrainedDoubleObject>(counter)->value() !=
								p_test_0->at<GConstrainedDoubleObject>(counter)->value());
				counter++;
				BOOST_CHECK(p_test->at<GDoubleObject>(counter)->value() != p_test_0->at<GDoubleObject>(counter)->value());
				counter++;
				std::shared_ptr <GDoubleCollection> p_gdc;
				std::shared_ptr <GDoubleCollection> p_gdc_0;
				BOOST_CHECK_NO_THROW(p_gdc = p_test->at<GDoubleCollection>(counter));
				BOOST_CHECK_NO_THROW(p_gdc_0 = p_test_0->at<GDoubleCollection>(counter));
				for (std::size_t gdc_cnt = 0; gdc_cnt < NGDOUBLECOLL; gdc_cnt++) {
					BOOST_CHECK_MESSAGE (
						p_gdc->at(gdc_cnt) != p_gdc_0->at(gdc_cnt), "\n"
						<< "p_gdc->at(gdc_cnt) = " << p_gdc->at(gdc_cnt) << "\n"
						<< "p_gdc_0->at(gdc_cnt) = " << p_gdc_0->at(gdc_cnt) <<
						"\n"
						<< "iteration = " << gdc_cnt << "\n"
					);
				}
				counter++;
			}

			// The int32 parameter should have stayed the same
			std::shared_ptr <GConstrainedInt32Object> p_int32_0;
			std::shared_ptr <GConstrainedInt32Object> p_int32;
			BOOST_CHECK_NO_THROW(p_int32_0 = p_test_0->at<GConstrainedInt32Object>(counter));
			BOOST_CHECK_NO_THROW(p_int32 = p_test->at<GConstrainedInt32Object>(counter));
			BOOST_CHECK(*p_int32_0 == *p_int32);
			counter++;

			// Likewise, the boolean parameter should have stayed the same
			std::shared_ptr <GBooleanObject> p_boolean_orig;
			std::shared_ptr <GBooleanObject> p_boolean_cloned;
			BOOST_CHECK_NO_THROW(p_boolean_orig = p_test_0->at<GBooleanObject>(counter));
			BOOST_CHECK_NO_THROW(p_boolean_cloned = p_test->at<GBooleanObject>(counter));
			BOOST_CHECK(*p_boolean_orig == *p_boolean_cloned);
			counter++;
		}

		//-----------------------------------------------------------------

		{ // Test that fpMultiplyByRandom() changes every single parameter
			// Create a GParameterSet object as a clone of p_test_0 for further usage
			std::shared_ptr <GParameterSet> p_test = p_test_0->clone<GParameterSet>();

			// Multiply each floating point value with a constrained random value
			BOOST_CHECK_NO_THROW(p_test->multiplyByRandom<double>(activityMode::ALLPARAMETERS));

			// Make sure the dirty flag is set
			BOOST_CHECK(p_test->isDirty() == true);

			// Cross-check
			std::size_t counter = 0;
			for (std::size_t i = 0; i < FPLOOPCOUNT; i++) {
				BOOST_CHECK(p_test->at<GConstrainedDoubleObject>(counter)->value() !=
								p_test_0->at<GConstrainedDoubleObject>(counter)->value());
				counter++;
				BOOST_CHECK(p_test->at<GDoubleObject>(counter)->value() != p_test_0->at<GDoubleObject>(counter)->value());
				counter++;
				std::shared_ptr <GDoubleCollection> p_gdc;
				std::shared_ptr <GDoubleCollection> p_gdc_0;
				BOOST_CHECK_NO_THROW(p_gdc = p_test->at<GDoubleCollection>(counter));
				BOOST_CHECK_NO_THROW(p_gdc_0 = p_test_0->at<GDoubleCollection>(counter));
				for (std::size_t gdc_cnt = 0; gdc_cnt < NGDOUBLECOLL; gdc_cnt++) {
					BOOST_CHECK_MESSAGE (
						p_gdc->at(gdc_cnt) != p_gdc_0->at(gdc_cnt), "\n"
						<< "p_gdc->at(gdc_cnt) = " << p_gdc->at(gdc_cnt) << "\n"
						<< "p_gdc_0->at(gdc_cnt) = " << p_gdc_0->at(gdc_cnt) <<
						"\n"
						<< "iteration = " << gdc_cnt << "\n"
					);
				}
				counter++;
			}

			// The int32 parameter should have stayed the same
			std::shared_ptr <GConstrainedInt32Object> p_int32_0;
			std::shared_ptr <GConstrainedInt32Object> p_int32;
			BOOST_CHECK_NO_THROW(p_int32_0 = p_test_0->at<GConstrainedInt32Object>(counter));
			BOOST_CHECK_NO_THROW(p_int32 = p_test->at<GConstrainedInt32Object>(counter));
			BOOST_CHECK(*p_int32_0 == *p_int32);
			counter++;

			// Likewise, the boolean parameter should have stayed the same
			std::shared_ptr <GBooleanObject> p_boolean_orig;
			std::shared_ptr <GBooleanObject> p_boolean_cloned;
			BOOST_CHECK_NO_THROW(p_boolean_orig = p_test_0->at<GBooleanObject>(counter));
			BOOST_CHECK_NO_THROW(p_boolean_cloned = p_test->at<GBooleanObject>(counter));
			BOOST_CHECK(*p_boolean_orig == *p_boolean_cloned);
			counter++;
		}

		//-----------------------------------------------------------------

		{ // Check adding of individuals
			// Create two GParameterSet objects as a clone of p_test_0 for further usage
			std::shared_ptr <GParameterSet> p_test = p_test_0->clone<GParameterSet>();
			std::shared_ptr <GParameterSet> p_test_fixed = p_test_0->clone<GParameterSet>();

			// Initialize all fp-values of the "add" individual with a fixed value
			BOOST_CHECK_NO_THROW(p_test_fixed->fixedValueInit<double>(FPADD, activityMode::ALLPARAMETERS));

			// Add p_test_fixed to p_test
			BOOST_CHECK_NO_THROW(p_test->add<double>(p_test_fixed, activityMode::ALLPARAMETERS));

			// Check the results
			std::size_t counter = 0;
			for (std::size_t i = 0; i < FPLOOPCOUNT; i++) {
				// A constrained value does not have to assume the value value()+FPADD
				// but needs to stay within its boundaries
				BOOST_CHECK(p_test->at<GConstrainedDoubleObject>(counter)->value() >= MINGCONSTRDOUBLE);
				BOOST_CHECK(p_test->at<GConstrainedDoubleObject>(counter)->value() <= MAXGCONSTRDOUBLE);
				counter++;
				BOOST_CHECK(
					p_test->at<GDoubleObject>(counter)->value() == p_test_0->at<GDoubleObject>(counter)->value() + FPADD);
				counter++;
				std::shared_ptr <GDoubleCollection> p_gdc;
				std::shared_ptr <GDoubleCollection> p_gdc_0;
				BOOST_CHECK_NO_THROW(p_gdc = p_test->at<GDoubleCollection>(counter));
				BOOST_CHECK_NO_THROW(p_gdc_0 = p_test_0->at<GDoubleCollection>(counter));
				for (std::size_t gdc_cnt = 0; gdc_cnt < NGDOUBLECOLL; gdc_cnt++) {
					BOOST_CHECK_MESSAGE (
						p_gdc->at(gdc_cnt) == p_gdc_0->at(gdc_cnt) + FPADD, "\n"
						<< "p_gdc->at(gdc_cnt) = " <<
						p_gdc->at(gdc_cnt) << "\n"
						<< "p_gdc_0->at(gdc_cnt) = " <<
						p_gdc_0->at(gdc_cnt) << "\n"
						<< "FPADD = " << FPADD
						<< "p_gdc_0->at(gdc_cnt) + FPADD = " <<
						p_gdc_0->at(gdc_cnt) + FPADD << "\n"
						<< "iteration = " << gdc_cnt << "\n"
					);
				}
				counter++;
			}

			// The int32 parameter should have stayed the same
			std::shared_ptr <GConstrainedInt32Object> p_int32_0;
			std::shared_ptr <GConstrainedInt32Object> p_int32;
			BOOST_CHECK_NO_THROW(p_int32_0 = p_test_0->at<GConstrainedInt32Object>(counter));
			BOOST_CHECK_NO_THROW(p_int32 = p_test->at<GConstrainedInt32Object>(counter));
			BOOST_CHECK(*p_int32_0 == *p_int32);
			counter++;

			// Likewise, the boolean parameter should have stayed the same
			std::shared_ptr <GBooleanObject> p_boolean_orig;
			std::shared_ptr <GBooleanObject> p_boolean_cloned;
			BOOST_CHECK_NO_THROW(p_boolean_orig = p_test_0->at<GBooleanObject>(counter));
			BOOST_CHECK_NO_THROW(p_boolean_cloned = p_test->at<GBooleanObject>(counter));
			BOOST_CHECK(*p_boolean_orig == *p_boolean_cloned);
			counter++;
		}

		//-----------------------------------------------------------------

		{ // Check subtraction of individuals
			// Create two GParameterSet objects as a clone of p_test_0 for further usage
			std::shared_ptr <GParameterSet> p_test = p_test_0->clone<GParameterSet>();
			std::shared_ptr <GParameterSet> p_test_fixed = p_test_0->clone<GParameterSet>();

			// Initialize all fp-values of the "add" individual with a fixed valie
			BOOST_CHECK_NO_THROW(p_test_fixed->fixedValueInit<double>(FPSUBTRACT, activityMode::ALLPARAMETERS));

			// Add p_test_fixed to p_test
			BOOST_CHECK_NO_THROW(p_test->subtract<double>(p_test_fixed, activityMode::ALLPARAMETERS));

			// Check the results
			std::size_t counter = 0;
			for (std::size_t i = 0; i < FPLOOPCOUNT; i++) {
				// A constrained value does not have to assume the value value()-FPSUBTRACT
				// but needs to stay within its boundaries
				BOOST_CHECK(p_test->at<GConstrainedDoubleObject>(counter)->value() >= MINGCONSTRDOUBLE);
				BOOST_CHECK(p_test->at<GConstrainedDoubleObject>(counter)->value() <= MAXGCONSTRDOUBLE);
				counter++;
				BOOST_CHECK(p_test->at<GDoubleObject>(counter)->value() ==
								p_test_0->at<GDoubleObject>(counter)->value() - FPSUBTRACT);
				counter++;
				std::shared_ptr <GDoubleCollection> p_gdc;
				std::shared_ptr <GDoubleCollection> p_gdc_0;
				BOOST_CHECK_NO_THROW(p_gdc = p_test->at<GDoubleCollection>(counter));
				BOOST_CHECK_NO_THROW(p_gdc_0 = p_test_0->at<GDoubleCollection>(counter));
				for (std::size_t gdc_cnt = 0; gdc_cnt < NGDOUBLECOLL; gdc_cnt++) {
					BOOST_CHECK_MESSAGE (
						p_gdc->at(gdc_cnt) == p_gdc_0->at(gdc_cnt) - FPSUBTRACT, "\n"
						<< "p_gdc->at(gdc_cnt) = " <<
						p_gdc->at(gdc_cnt) << "\n"
						<< "p_gdc_0->at(gdc_cnt) = " <<
						p_gdc_0->at(gdc_cnt) << "\n"
						<< "FPSUBTRACT = " << FPSUBTRACT
						<< "p_gdc_0->at(gdc_cnt) - FPSUBTRACT = " <<
						p_gdc_0->at(gdc_cnt) - FPSUBTRACT << "\n"
						<< "iteration = " << gdc_cnt << "\n"
					);
				}
				counter++;
			}

			// The int32 parameter should have stayed the same
			std::shared_ptr <GConstrainedInt32Object> p_int32_0;
			std::shared_ptr <GConstrainedInt32Object> p_int32;
			BOOST_CHECK_NO_THROW(p_int32_0 = p_test_0->at<GConstrainedInt32Object>(counter));
			BOOST_CHECK_NO_THROW(p_int32 = p_test->at<GConstrainedInt32Object>(counter));
			BOOST_CHECK(*p_int32_0 == *p_int32);
			counter++;

			// Likewise, the boolean parameter should have stayed the same
			std::shared_ptr <GBooleanObject> p_boolean_orig;
			std::shared_ptr <GBooleanObject> p_boolean_cloned;
			BOOST_CHECK_NO_THROW(p_boolean_orig = p_test_0->at<GBooleanObject>(counter));
			BOOST_CHECK_NO_THROW(p_boolean_cloned = p_test->at<GBooleanObject>(counter));
			BOOST_CHECK(*p_boolean_orig == *p_boolean_cloned);
			counter++;
		}

		//-----------------------------------------------------------------
	}

	//---------------------------------------------------------------------

	{ // Check counting of active and inactive parameters
		// Some settings for the collection of tests below
		const double MINGCONSTRDOUBLE = -4.;
		const double MAXGCONSTRDOUBLE = 4.;
		const double MINGDOUBLE = -5.;
		const double MAXGDOUBLE = 5.;
		const double MINGDOUBLECOLL = -3.;
		const double MAXGDOUBLECOLL = 3.;
		const std::size_t NGDOUBLECOLL = 10;
		const std::size_t NINTCOLL = 10;
		const std::size_t NINTBOOLOBJ = 10;
		const std::int32_t MINGINT = -100;
		const std::int32_t MAXGINT = 100;
		const std::size_t FPLOOPCOUNT = 5;
		const double FPFIXEDVALINITMIN = -3.;
		const double FPFIXEDVALINITMAX = 3.;
		const double FPMULTIPLYBYRANDMIN = -5.;
		const double FPMULTIPLYBYRANDMAX = 5.;
		const double FPADD = 2.;
		const double FPSUBTRACT = 2.;

		// Create a GParameterSet object as a clone of this object for further usage
		std::shared_ptr <GParameterSet> p_test_0 = this->clone<GParameterSet>();
		// Clear the collection
		p_test_0->clear();
		// Make sure it is really empty
		BOOST_CHECK(p_test_0->empty());

		// Add some floating point parameters
		for (std::size_t i = 0; i < FPLOOPCOUNT; i++) {
			std::shared_ptr <GConstrainedDoubleObject> gcdo_ptr = std::shared_ptr<GConstrainedDoubleObject>(
				new GConstrainedDoubleObject(
					uniform_real_distribution(m_gr, std::uniform_real_distribution<double>::param_type(MINGCONSTRDOUBLE, MAXGCONSTRDOUBLE))
					, MINGCONSTRDOUBLE
					, MAXGCONSTRDOUBLE
				)
			);
			std::shared_ptr <GDoubleObject> gdo_ptr = std::shared_ptr<GDoubleObject>(
				new GDoubleObject(
					uniform_real_distribution(m_gr, std::uniform_real_distribution<double>::param_type(MINGDOUBLE, MAXGDOUBLE))
				)
			);
			std::shared_ptr <GDoubleCollection> gdc_ptr = std::shared_ptr<GDoubleCollection>(new GDoubleCollection(NGDOUBLECOLL, MINGDOUBLECOLL, MAXGDOUBLECOLL));

			// Mark the last parameter type as inactive
			gdc_ptr->setAdaptionsInactive();

			// Add the parameter objects to the parameter set
			p_test_0->push_back(gcdo_ptr);
			p_test_0->push_back(gdo_ptr);
			p_test_0->push_back(gdc_ptr);
		}

		// Attach a few other parameter types
		for (std::size_t i = 0; i < NINTBOOLOBJ; i++) {
			p_test_0->push_back(
				std::shared_ptr<GConstrainedInt32Object>(new GConstrainedInt32Object(7, MINGINT, MAXGINT)));
			p_test_0->push_back(std::shared_ptr<GBooleanObject>(new GBooleanObject(true)));
		}

		// Finally we add a tree structure
		std::shared_ptr <GParameterObjectCollection> poc_ptr =
			std::shared_ptr<GParameterObjectCollection>(new GParameterObjectCollection());

		for (std::size_t i = 0; i < FPLOOPCOUNT; i++) {
			std::shared_ptr <GConstrainedDoubleObject> gcdo_ptr = std::shared_ptr<GConstrainedDoubleObject>(
				new GConstrainedDoubleObject(
					uniform_real_distribution(m_gr, std::uniform_real_distribution<double>::param_type(MINGCONSTRDOUBLE, MAXGCONSTRDOUBLE))
					, MINGCONSTRDOUBLE
					, MAXGCONSTRDOUBLE
				)
			);
			std::shared_ptr <GDoubleObject> gdo_ptr = std::shared_ptr<GDoubleObject>(
				new GDoubleObject(
					uniform_real_distribution(m_gr, std::uniform_real_distribution<double>::param_type(MINGDOUBLE, MAXGDOUBLE))
				)
			);
			std::shared_ptr <GConstrainedInt32ObjectCollection> gcioc_ptr
				= std::shared_ptr<GConstrainedInt32ObjectCollection>(
					new GConstrainedInt32ObjectCollection()
				);

			std::shared_ptr <GParameterObjectCollection> sub_poc_ptr =
				std::shared_ptr<GParameterObjectCollection>(new GParameterObjectCollection());

			for (std::size_t ip = 0; ip < NINTCOLL; ip++) {
				std::shared_ptr <GConstrainedInt32Object> gci32o_ptr
					= std::shared_ptr<GConstrainedInt32Object>(new GConstrainedInt32Object(MINGINT, MAXGINT));
				gci32o_ptr->setAdaptionsInactive(); // The parameter should not be modifiable now

				sub_poc_ptr->push_back(gci32o_ptr);
			}

			std::shared_ptr <GDoubleObject> gdo2_ptr = std::shared_ptr<GDoubleObject>(
				new GDoubleObject(
					uniform_real_distribution(m_gr, std::uniform_real_distribution<double>::param_type(MINGDOUBLE, MAXGDOUBLE))
				)
			);
			gdo2_ptr->setAdaptionsInactive();
			sub_poc_ptr->push_back(gdo2_ptr);

			// Add the parameter objects to the parameter set
			poc_ptr->push_back(gcdo_ptr);
			poc_ptr->push_back(gdo_ptr);
			poc_ptr->push_back(gcioc_ptr);
			poc_ptr->push_back(sub_poc_ptr);
		}

		p_test_0->push_back(poc_ptr);

		// The amount of parameters of a given category
		std::size_t NDOUBLEACTIVE = 2 * FPLOOPCOUNT + 2 * FPLOOPCOUNT;
		std::size_t NDOUBLEINACTIVE = NGDOUBLECOLL * FPLOOPCOUNT + FPLOOPCOUNT;
		std::size_t NDOUBLEALL = NDOUBLEINACTIVE + NDOUBLEACTIVE;
		std::size_t NINTACTIVE = NINTBOOLOBJ;
		std::size_t NINTINACTIVE = NINTCOLL * FPLOOPCOUNT;
		std::size_t NINTALL = NINTINACTIVE + NINTACTIVE;
		std::size_t NBOOLACTIVE = NINTBOOLOBJ;
		std::size_t NBOOLINACTIVE = 0;
		std::size_t NBOOLALL = NBOOLINACTIVE + NBOOLACTIVE;

		//-----------------------------------------------------------------

		{ // Test counting of parameters
			// Create a GParameterSet object as a clone of p_test_0 for further usage
			std::shared_ptr <GParameterSet> p_test = p_test_0->clone<GParameterSet>();

			// Count the number of parameters and compare with the expected number
			BOOST_CHECK(p_test->countParameters<double>(activityMode::ACTIVEONLY) == NDOUBLEACTIVE);
			BOOST_CHECK(p_test->countParameters<double>(activityMode::INACTIVEONLY) == NDOUBLEINACTIVE);
			BOOST_CHECK(p_test->countParameters<double>(activityMode::ALLPARAMETERS) == NDOUBLEALL);
			BOOST_CHECK(p_test->countParameters<std::int32_t>(activityMode::ACTIVEONLY) == NINTACTIVE);
			BOOST_CHECK(p_test->countParameters<std::int32_t>(activityMode::INACTIVEONLY) == NINTINACTIVE);
			BOOST_CHECK(p_test->countParameters<std::int32_t>(activityMode::ALLPARAMETERS) == NINTALL);
			BOOST_CHECK(p_test->countParameters<bool>(activityMode::ACTIVEONLY) == NBOOLACTIVE);
			BOOST_CHECK(p_test->countParameters<bool>(activityMode::INACTIVEONLY) == NBOOLINACTIVE);
			BOOST_CHECK(p_test->countParameters<bool>(activityMode::ALLPARAMETERS) == NBOOLALL);
		}

		//-----------------------------------------------------------------

		{ // Check that streamline(activityMode::INACTIVEONLY) yields unchanged results before and after randomInit(activityMode::ACTIVEONLY)
			// Create two GParameterSet objects as clones of p_test_0 for further usage
			std::shared_ptr <GParameterSet> p_test_orig = p_test_0->clone<GParameterSet>();
			std::shared_ptr <GParameterSet> p_test_rand = p_test_0->clone<GParameterSet>();

			// Randomly initialize active components of p_test2
			BOOST_CHECK_NO_THROW(p_test_rand->randomInit(activityMode::ACTIVEONLY));

			std::vector<double> orig_d_inactive;
			std::vector<double> rand_d_inactive;

			std::vector<std::int32_t> orig_i_inactive;
			std::vector<std::int32_t> rand_i_inactive;

			std::vector<bool> orig_b_inactive;
			std::vector<bool> rand_b_inactive;

			// Extract the parameters
			BOOST_CHECK_NO_THROW(p_test_orig->streamline<double>(orig_d_inactive, activityMode::INACTIVEONLY));
			BOOST_CHECK_NO_THROW(p_test_rand->streamline<double>(rand_d_inactive, activityMode::INACTIVEONLY));
			BOOST_CHECK_NO_THROW(p_test_orig->streamline<std::int32_t>(orig_i_inactive, activityMode::INACTIVEONLY));
			BOOST_CHECK_NO_THROW(p_test_rand->streamline<std::int32_t>(rand_i_inactive, activityMode::INACTIVEONLY));
			BOOST_CHECK_NO_THROW(p_test_orig->streamline<bool>(orig_b_inactive, activityMode::INACTIVEONLY));
			BOOST_CHECK_NO_THROW(p_test_rand->streamline<bool>(rand_b_inactive, activityMode::INACTIVEONLY));

			// Check that the "inactive" vectors have the expected characteristics
			BOOST_CHECK(orig_d_inactive.size() == NDOUBLEINACTIVE);
			BOOST_CHECK(orig_d_inactive == rand_d_inactive);
			BOOST_CHECK(orig_i_inactive.size() == NINTINACTIVE);
			BOOST_CHECK(orig_i_inactive == rand_i_inactive);
			BOOST_CHECK(orig_b_inactive.size() == NBOOLINACTIVE);
			BOOST_CHECK(orig_b_inactive == rand_b_inactive);
		}

		//-----------------------------------------------------------------

		{ // Check that streamline(activityMode::ACTIVEONLY) yields changed results after randomInit(activityMode::ACTIVEONLY)
			// Create a GParameterSet object as a clone of p_test_0 for further usage
			// Create two GParameterSet objects as clones of p_test_0 for further usage
			std::shared_ptr <GParameterSet> p_test_orig = p_test_0->clone<GParameterSet>();
			std::shared_ptr <GParameterSet> p_test_rand = p_test_0->clone<GParameterSet>();

			// Randomly initialize active components of p_test2
			BOOST_CHECK_NO_THROW(p_test_rand->randomInit(activityMode::ACTIVEONLY));

			std::vector<double> orig_d_active;
			std::vector<double> rand_d_active;

			std::vector<std::int32_t> orig_i_active;
			std::vector<std::int32_t> rand_i_active;

			std::vector<bool> orig_b_active;
			std::vector<bool> rand_b_active;

			// Extract the parameters
			BOOST_CHECK_NO_THROW(p_test_orig->streamline<double>(orig_d_active, activityMode::ACTIVEONLY));
			BOOST_CHECK_NO_THROW(p_test_rand->streamline<double>(rand_d_active, activityMode::ACTIVEONLY));
			BOOST_CHECK_NO_THROW(p_test_orig->streamline<std::int32_t>(orig_i_active, activityMode::ACTIVEONLY));
			BOOST_CHECK_NO_THROW(p_test_rand->streamline<std::int32_t>(rand_i_active, activityMode::ACTIVEONLY));
			BOOST_CHECK_NO_THROW(p_test_orig->streamline<bool>(orig_b_active, activityMode::ACTIVEONLY));
			BOOST_CHECK_NO_THROW(p_test_rand->streamline<bool>(rand_b_active, activityMode::ACTIVEONLY));

			// Check that the "active" vectors' contents indeed differ
			BOOST_CHECK(orig_d_active.size() == NDOUBLEACTIVE);
			BOOST_CHECK(rand_d_active.size() == NDOUBLEACTIVE);
			BOOST_CHECK(orig_d_active != rand_d_active);
			BOOST_CHECK(orig_i_active.size() == NINTACTIVE);
			BOOST_CHECK(rand_i_active.size() == NINTACTIVE);
			BOOST_CHECK_MESSAGE(
				orig_i_active != rand_i_active,
				"orig_i_active: " << Gem::Common::vecToString(orig_i_active) << "\n" << "rand_i_active: " <<
										Gem::Common::vecToString(rand_i_active) << "\n"
			);
			BOOST_CHECK(orig_b_active.size() == NBOOLACTIVE);
			BOOST_CHECK(rand_b_active.size() == NBOOLACTIVE);

			// We do not compare the (single) boolean value here, as there are just
			// two distinct values it may assume, so the likelihood for identical values
			// and thus failure of this test is high.
			// BOOST_CHECK(orig_b_active != rand_b_active);
		}

		//-----------------------------------------------------------------
	}

	//---------------------------------------------------------------------

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
	Gem::Common::condnotset("GParameterSet::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GParameterSet::specificTestsFailuresExpected_GUnitTests() {
#ifdef GEM_TESTING
	using boost::unit_test_framework::test_suite;
	using boost::unit_test_framework::test_case;

	// Call the parent classes' functions
	GObject::specificTestsFailuresExpected_GUnitTests();
	Gem::Common::GStdPtrVectorInterfaceT<GParameterBase, GObject>::specificTestsFailuresExpected_GUnitTests();

	// no tests here yet

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
	Gem::Common::condnotset("GParameterSet::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

