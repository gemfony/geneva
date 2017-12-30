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

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GParameterSet) // NOLINT

namespace Gem {
namespace Geneva {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * Initialization with a raw fitness
 */
parameterset_processing_result::parameterset_processing_result(double raw_fitness)
	: m_raw_fitness(raw_fitness)
	, m_transformed_fitness(m_raw_fitness)
	, m_transformed_fitness_set(false)
{ /* nothing */ }

/******************************************************************************/
/** @brief Initialization with a raw and transformed fitness */
parameterset_processing_result::parameterset_processing_result(
	double raw_fitness
	, double transformed_fitness
)
	: m_raw_fitness(raw_fitness)
	, m_transformed_fitness(transformed_fitness)
	, m_transformed_fitness_set(true)
{ /* nothing */ }

/******************************************************************************/
/** @brief Initialization with a raw fitness and recalculation of the transformed fitness */
parameterset_processing_result::parameterset_processing_result(
	double raw_fitness
	, std::function<double(double)> f
)
	: m_raw_fitness(raw_fitness)
{
	if(f) {
		m_transformed_fitness = f(m_raw_fitness);
		m_transformed_fitness_set = true;
	} else {
		glogger
			<< "In parameterset_processing_result(double, std::function<double(double)>)" << std::endl
			<< "Got empty function object" << std::endl
		   << GTERMINATION;
	}
}

/******************************************************************************/
/**
 * Access to the raw fitness
 */
double parameterset_processing_result::rawFitness() const {
	return m_raw_fitness;
}

/******************************************************************************/
/**
 * Access to the transformed fitness
 */
double parameterset_processing_result::transformedFitness() const {
	return m_transformed_fitness;
}

/******************************************************************************/
/**
 * Updates the transformed fitness using an external function
 */
void parameterset_processing_result::setTransformedFitnessWith(
	std::function<double(double)> f
){
	if(f) {
		m_transformed_fitness = f(m_raw_fitness);
		m_transformed_fitness_set = true;
	} else {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In parameterset_processing_result::setTransformedFitnessWith():" << std::endl
				<< "Function object f is empty." << std::endl
		);
	}
}

/******************************************************************************/
/**
 * Sets the transformed fitness to a user-defined value
 */
void parameterset_processing_result::setTransformedFitnessTo(double transformed_fitness){
	m_transformed_fitness = transformed_fitness;
	m_transformed_fitness_set = true;
}

/******************************************************************************/
/**
 * Sets the transformed fitness to the same value as the raw fitness
 */
void parameterset_processing_result::setTransformedFitnessToRaw() {
	m_transformed_fitness = m_raw_fitness;
	m_transformed_fitness_set = true;
}

/******************************************************************************/
/**
 * Checks whether the transformed fitness was set
 */
bool parameterset_processing_result::transformedFitnessSet() const {
	return m_transformed_fitness_set;
}

/******************************************************************************/
/**
 * Resets the object and stores a new raw value in the class
 */
void parameterset_processing_result::reset(double raw_fitness) {
	m_raw_fitness = raw_fitness;
	m_transformed_fitness = m_raw_fitness;
	m_transformed_fitness_set = false;
}

/******************************************************************************/
/**
 * Resets the object and stores a new raw and transformed value in the class
 */
void parameterset_processing_result::reset(
	double raw_fitness
	, double transformed_fitness
) {
	m_raw_fitness = raw_fitness;
	m_transformed_fitness = transformed_fitness;
	m_transformed_fitness_set = true;
}

/******************************************************************************/
/**
 * Resets the object and stores a new raw value in the class and triggers recalculation of the transformed value
 */
void parameterset_processing_result::reset(
	double raw_fitness
	, std::function<double(double)> f
) {
	if(f) {
		m_transformed_fitness = f(m_raw_fitness);
		m_transformed_fitness_set = true;
	} else {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In parameterset_processing_result::reset():" << std::endl
				<< "Function object f is empty." << std::endl
		);
	}
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * The default constructor. Using this constructor will result in a single
 * fitness criterion.
 */
GParameterSet::GParameterSet()
	: Gem::Courtier::GProcessingContainerT<GParameterSet, parameterset_processing_result>(1)
{ /* nothing */ }

/******************************************************************************/
/**
 * Initialization with the number of fitness criteria
 */
GParameterSet::GParameterSet(std::size_t n_fitness_criteria)
	: Gem::Courtier::GProcessingContainerT<GParameterSet, parameterset_processing_result>(n_fitness_criteria)
{ /* nothing */ }

/******************************************************************************/
/**
 * The copy constructor.
 *
 * @param cp A copy of another GParameterSet object
 */
GParameterSet::GParameterSet(const GParameterSet &cp)
	: GObject(cp)
   , G_Interface_Mutable(cp)
   , G_Interface_Rateable(cp)
   , Gem::Common::GStdPtrVectorInterfaceT<GParameterBase, GObject>(cp)
   , Gem::Courtier::GProcessingContainerT<GParameterSet, parameterset_processing_result>(cp)
   , m_perItemCrossOverProbability(cp.m_perItemCrossOverProbability)
   , m_best_past_primary_fitness(cp.m_best_past_primary_fitness)
   , m_n_stalls(cp.m_n_stalls)
   , m_maxmode(cp.m_maxmode)
   , m_assigned_iteration(cp.m_assigned_iteration)
   , m_validity_level(cp.m_validity_level)
   , m_eval_policy(cp.m_eval_policy)
   , m_sigmoid_steepness(cp.m_sigmoid_steepness)
   , m_sigmoid_extremes(cp.m_sigmoid_extremes)
   , m_max_unsuccessful_adaptions(cp.m_max_unsuccessful_adaptions)
   , m_max_retries_until_valid(cp.m_max_retries_until_valid)
   , m_n_adaptions(cp.m_n_adaptions)
{
	// Copy the personality pointer over
	Gem::Common::copyCloneableSmartPointer(cp.m_pt_ptr, m_pt_ptr);
	// Make sure any constraints are copied over
	Gem::Common::copyCloneableSmartPointer(cp.m_individual_constraint_ptr, m_individual_constraint_ptr);
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
	compare_t(IDENTITY(m_best_past_primary_fitness, p_load->m_best_past_primary_fitness), token);
	compare_t(IDENTITY(m_n_stalls, p_load->m_n_stalls), token);
	compare_t(IDENTITY(m_maxmode, p_load->m_maxmode), token);
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
	this->mark_as_due_for_processing();
	cp.mark_as_due_for_processing();
}

/* ----------------------------------------------------------------------------------
 * Tested in GTestIndividual1::specificTestsNoFailureExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Allows to randomly initialize parameter members. This function is responsible
 * for setting the dirty flag, so overloaded randomInit_ functions do not need
 * to take care of this. Note though that overloads of randomInit_() need to take
 * care to indicate whether modifications were made.
 *
 * @return A boolean indicating whether modifications where made
 */
bool GParameterSet::randomInit(const activityMode &am) {
	bool modifications_made = this->randomInit_(am);

	if(modifications_made) {
		this->mark_as_due_for_processing();
	}

	return modifications_made;
}

/******************************************************************************/
/**
 * Allows to specify whether we want to work in maximization (maxMode::MAXIMIZE) or minimization
 * (maxMode::MINIMIZE) mode (the default). The idea is that GParameterSet, depending on the maxMode,
 * changes its evaluation in such a way that the optimization algorithm always sees a
 * minimization problem.
 *
 * @param mode An enum class which indicates whether we want to work in maximization or minimization mode
 */
void GParameterSet::setMaxMode(const maxMode& mode) {
	m_maxmode = mode;
}

/******************************************************************************/
/**
 * Transformation of the individual's parameter objects into a boost::property_tree object .
 * This is e.g. used in GExternalEvaluatorIndividual for the communication with external
 * evaluation programs.
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

	bool dirtyFlag = (Gem::Courtier::processingStatus::DO_PROCESS == this->getProcessingStatus());
	bool hasErrors = this->has_errors();

	double rawFitness = 0., transformedFitness = 0.;

	ptr.put(baseName + ".iteration", this->getAssignedIteration());
	ptr.put(baseName + ".id", this->getCurrentEvaluationID());
	ptr.put(baseName + ".isDirty", dirtyFlag);
	ptr.put(baseName + ".hasErrors", hasErrors);
	ptr.put(baseName + ".isValid", hasErrors || dirtyFlag ? false : this->isValid());
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

		case evaluationPolicy::USEWORSTCASEFORINVALID:
			ptr.put(baseName + ".transformationPolicy", "USEWORSTCASEFORINVALID");
			break;
	}

	// Output all fitness criteria. We do not enforce re-calculation of the fitness here,
	// as the property is meant to capture the current state of the individual.
	// Check the "isDirty" tag, if you need to know whether the results are current.
	ptr.put(baseName + ".nResults", this->getNStoredResults());
	for (std::size_t i = 0; i < this->getNStoredResults(); i++) {
		rawFitness = (dirtyFlag || hasErrors) ? this->getWorstCase() : this->raw_fitness(i);
		transformedFitness = (dirtyFlag || hasErrors) ? this->getWorstCase() : this->transformed_fitness(i);

		base = baseName + ".results.result" + Gem::Common::to_string(i);
		ptr.put(base, transformedFitness);
		base = baseName + ".results.rawResult" + Gem::Common::to_string(i);
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
				varTypes.emplace_back("double");
			}
			varValues.push_back(Gem::Common::to_string((item.second).at(pos)));
		}
	}

	for(const auto& item: fData) {
		for (std::size_t pos = 0; pos < (item.second).size(); pos++) {
			if (withNameAndType) {
				varNames.push_back(item.first + "_" + Gem::Common::to_string(pos));
				varTypes.emplace_back("float");
			}
			varValues.push_back(Gem::Common::to_string((item.second).at(pos)));
		}
	}

	for(const auto& item: iData) {
		for (std::size_t pos = 0; pos < (item.second).size(); pos++) {
			if (withNameAndType) {
				varNames.push_back(item.first + "_" + Gem::Common::to_string(pos));
				varTypes.emplace_back("int32");
			}
			varValues.push_back(Gem::Common::to_string((item.second).at(pos)));
		}
	}

	for(const auto& item: bData) {
		for (std::size_t pos = 0; pos < (item.second).size(); pos++) {
			if (withNameAndType) {
				varNames.push_back(item.first + "_" + Gem::Common::to_string(pos));
				varTypes.emplace_back("bool");
			}
			varValues.push_back(Gem::Common::to_string((item.second).at(pos)));
		}
	}

	// Note: The following will output the string "dirty" if the individual is in a "dirty" state
	for (std::size_t i = 0; i < this->getNStoredResults(); i++) {
		if (withNameAndType) {
			varNames.push_back(std::string("Fitness_") + Gem::Common::to_string(i));
			varTypes.emplace_back("double");
		}
		if(this->is_processed()) { // The individual has already been evaluated
			if (useRawFitness) {
				varValues.push_back(
					Gem::Common::to_string(
						this->raw_fitness(i)
					)
				);
			} else { // Output potentially transformed fitness
				varValues.push_back(
					Gem::Common::to_string(
						this->transformed_fitness(i)
					)
				);
			}
		} else { // No evaluation was performed so far
			if(this->has_errors()) {
				varValues.emplace_back("has_errors");
			} else { // "only" dirty / unevaluated
				varValues.emplace_back("dirty");
			}
		}
	}

	if (showValidity) {
		if (withNameAndType) {
			varNames.emplace_back(std::string("validity"));
			varTypes.emplace_back("bool");
		}

		if(this->is_processed()) { // The individual has already been evaluated
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
	if(boundaries.size() != this->getNStoredResults()) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GParameterSet::isGoodEnough(): Error!" << std::endl
				<< "Number of boundaries does not match number of fitness criteria" << std::endl
		);
	}

	// Has the individual been processed
	if(!this->is_processed()) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GParameterSet::isGoodEnough(): Error!" << std::endl
				<< "Trying to compare fitness values although the individual isn't processed" << std::endl
		);
	}
#endif /* DEBUG */

	// Check the fitness values. If we find at least one
	// which is worse than the one supplied by the boundaries
	// vector, then this individual fails the test
	if (maxMode::MAXIMIZE == this->getMaxMode()) { // Maximization
		for (std::size_t i = 0; i < boundaries.size(); i++) {
			if(this->raw_fitness(i) < boundaries.at(i)) {
				return false;
			}
		}
	} else { // maxMode::MINIMIZE
		for (std::size_t i = 0; i < boundaries.size(); i++) {
			if (this->raw_fitness(i) > boundaries.at(i)) {
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
std::shared_ptr<GParameterSet> GParameterSet::amalgamate(const std::shared_ptr<GParameterSet>& cp) const {
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
	, double likelihood
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
	this->mark_as_due_for_processing();
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
	// Check whether the "foreign" entity is processed
	if(cp.is_due_for_processing() || cp.has_errors()) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GParameterSet::cannibalize(const GParameterSet& cp)" << std::endl
				<< "cp isn't processed or has errors" << std::endl
		);
	}

	// Make sure we have no local parameters
	this->clear();

	// Copy all "foreign" parameters over
	for(const auto& t_ptr: cp) {
		this->push_back(t_ptr);
	}

	// Empty the foreign GParmeterSet object
	cp.clear();

	// Set our own fitness according to the foreign individual. This will also
	// clear our local dirty flag (if set).
	this->setFitness_(cp.raw_fitness_vec());
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
			// Perform the actual adaption; Terminate, if at least one adaption was performed
			if ((nAdaptions = this->customAdaptions()) > 0) {
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
	if(nAdaptions > 0) {
		this->mark_as_due_for_processing();
	}

	// Store the number of adaptions for later use and let the audience know
	return (m_n_adaptions=nAdaptions);
}

/* ----------------------------------------------------------------------------------
 * Tested in GTestIndividual1::specificTestsNoFailureExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Retrieves the stored raw fitness with a given id
 */
double GParameterSet::raw_fitness(std::size_t id) const {
	return this->getConstStoredResult(id).rawFitness();
}

/******************************************************************************/
/**
 * Retrieves the stored transformed fitness with a given id
 */
double GParameterSet::transformed_fitness(std::size_t id) const {
	return this->getConstStoredResult(id).transformedFitness();
}

/******************************************************************************/
/**
 * Returns all raw fitness results in a std::vector
 */
std::vector<double> GParameterSet::raw_fitness_vec() const {
	std::size_t nFitnessCriteria = this->getNStoredResults();
	std::vector<double> resultVec;

	for (std::size_t i = 0; i < nFitnessCriteria; i++) {
		resultVec.push_back(this->raw_fitness(i));
	}

	return resultVec;
}

/******************************************************************************/
/**
 * Returns all transformed fitness results in a std::vector
 */
std::vector<double> GParameterSet::transformed_fitness_vec() const {
	std::size_t nFitnessCriteria = this->getNStoredResults();
	std::vector<double> resultVec;

	for (std::size_t i = 0; i < nFitnessCriteria; i++) {
		resultVec.push_back(this->transformed_fitness(i));
	}

	return resultVec;
}

/******************************************************************************/
/**
 * Returns the result of the fitness calculation for a fitness value with a given id.This
 * function will always return the raw fitness, as it is likely the one called by users
 * directly -- they will expect untransformed values.
 */
double GParameterSet::fitness(std::size_t id) const {
	return this->raw_fitness(id);
}

/******************************************************************************/
/**
 * Returns the last known fitness calculations of this object. Re-calculation
 * of the fitness is triggered, if the "dirty flag" is set, unless this is the server mode.
 * By means of supplying an id it is possible to distinguish between different target functions.
 * 0 denotes the main fitness criterion. The user can specify whether he/she is interested
 * in the transformed or the raw fitness value.
 *
 * TODO: cross check logic of this function
 *
 * @param id The id of the fitness criterion
 * @param reevaluationAllowed Explicit permission to re-evaluate the individual
 * @param useTransformedFitness Whether the transformed or the raw fitness should be returned
 * @return The fitness of this individual
 */
double GParameterSet::fitness(
	std::size_t id
	, bool reevaluationAllowed
	, bool useTransformedFitness
) {
	// Check whether we need to recalculate the fitness
	if (this->is_due_for_processing()) {
		if (reevaluationAllowed) {
			this->process();

#ifdef DEBUG
			// Check if the item is processed now
			if(this->is_due_for_processing()) {
				throw gemfony_exception(
					g_error_streamer(DO_LOG,  time_and_place)
						<< "In GParameterSet::fitness(...):" << std::endl
						<< "Individual is unprocessed in a location where it shouldn't be" << std::endl
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

	// Return the desired result
	if(useTransformedFitness) {
		return this->transformed_fitness(id);
	} else {
		return this->raw_fitness(id);
	}
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
 * throw, when it is called on a dirty individual.
 *
 * @param id The id of the fitness criterion
 * @param reevaluationAllowed Explicit permission to re-evaluate the individual
 * @param useTransformedFitness Whether the transformed or the raw fitness should be returned
 * @return The fitness of this individual
 */
double GParameterSet::fitness(
	std::size_t id
	, bool reevaluationAllowed
	, bool useTransformedFitness
) const {
#ifdef DEBUG
	// This function should only be called for clean (i.e. evaluated) individuals
	if(this->is_due_for_processing()) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GParameterSet::fitness(...) const:" << std::endl
				<< "Individual is still unprocessed in a location where it shouldn't be" << std::endl
		);
	}
#endif /* DEBUG */

	// Return the desired result
	if(useTransformedFitness) {
		return this->transformed_fitness(id);
	} else {
		return this->raw_fitness(id);
	}
}

/******************************************************************************/
/**
 * Returns the transformed result of the fitness function with id 0
 */
double GParameterSet::transformedFitness(std::size_t id) const {
	return this->transformed_fitness(id);
}

/******************************************************************************/
/**
 * Returns all raw fitness results in a std::vector
 */
std::vector<double> GParameterSet::fitnessVec() const {
	return this->raw_fitness_vec();
}

/******************************************************************************/
/**
 * Returns all raw or transformed fitness results in a std::vector
 */
std::vector<double> GParameterSet::fitnessVec(bool useRawFitness) const {
	if(useRawFitness) {
		return raw_fitness_vec();
	} else {
		return transformed_fitness_vec();
	}
}

/******************************************************************************/
/**
 * Returns all transformed fitness results in a std::vector
 */
std::vector<double> GParameterSet::transformedFitnessVec() const {
	return this->transformed_fitness_vec();
}

/******************************************************************************/
/**
 * Retrieve the current (not necessarily up-to-date) fitness
 */
double GParameterSet::getCachedFitness(
	std::size_t id
	, bool useTransformedFitness
) const {
	if(useTransformedFitness) {
		return this->transformed_fitness(id);
	} else { // raw fitness
		return this->raw_fitness(id);
	}
}

/******************************************************************************/
/**
 * Register another result value of the fitness calculation. Multiple fitness
 * criteria are used in multi-criterion optimization. fitnessCalculation() returns
 * the main fitness value, but may also add further, secondary results. Note that,
 * whether these are actually used, depends on the optimization algorithm being
 * used. Transformation for the second fitness value will be done in the process_()
 * function. You may store the primary fitness value with this function as well.
 * As the primary (raw) value is however also returned by fitnessCalculation() and
 * integrated into the list of results, this is redundant.
 *
 * @param id The position of the fitness criterion (must be >= 0 !)
 * @param value The fitness value to be registered
 */
void GParameterSet::setResult(
	std::size_t id
	, double value
) {
#ifdef DEBUG
	if(id >= this->getNStoredResults()) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG, time_and_place)
				<< "In GParameterSet::setResult(...): Error!" << std::endl
				<< "Invalid position in vector: " << id << " (expected min 0 and max " <<  this->getNStoredResults()-1 << ")" << std::endl
		);
	}
#endif /* DEBUG */

	this->getStoredResult(id).reset(value);
}

/******************************************************************************/
/**
 * Registers a new, "raw"  result value of the custom fitness calculation.
 * NOTE THAT THIS FUNCTION IS DEPRECATED. USE setResult() instead!
 *
 * @param id The position of the fitness criterion (must be >= 0 !)
 * @param value The value fitness value to be registered
 */
void GParameterSet::registerSecondaryResult(
	std::size_t id
	, double value
) {
	this->setResult(id, value);
}

/******************************************************************************/
/**
 * Determines the number of fitness criteria present for the individual.
 *
 * @return The number of fitness criteria registered with this individual
 */
std::size_t GParameterSet::getNumberOfFitnessCriteria() const {
	return this->getNStoredResults();
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
	parameterset_processing_result worstVal(this->getWorstCase());
	worstVal.setTransformedFitnessTo(this->getWorstCase());

	this->setNStoredResults(nFitnessCriteria, worstVal);
}

/******************************************************************************/
/**
 * Determines whether more than one fitness criterion is present for this individual
 *
 * @return A boolean indicating whether more than one target function is present
 */
bool GParameterSet::hasMultipleFitnessCriteria() const {
	return (this->getNStoredResults() > 1 ? true : false);
}

/******************************************************************************/
/**
 * Retrieve the fitness tuple at a given evaluation position.
 */
std::tuple<double, double> GParameterSet::getFitnessTuple(std::uint32_t id) const {
	return std::make_tuple<double, double>(
		this->raw_fitness(id)
		, this->transformed_fitness(id)
	);
}

/******************************************************************************/
/**
 * Checks whether the work item has the DO_PROCESS flag set or has errors.
 * NOTE THAT THIS FUNCTION IS DEPRECATED.
 *
 * @return A boolean indicating whether the individual is due for processing
 */
bool GParameterSet::isDirty() const {
	return (this->is_due_for_processing() || this->has_errors());
}

/* ----------------------------------------------------------------------------------
 * Tested in GTestIndividual1::specificTestsNoFailureExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Checks whether this individual is "clean", i.e. has been processed.
 * NOTE THAT THIS FUNCTION IS DEPRECATED.
 */
bool GParameterSet::isClean() const {
	return this->is_processed();
}

/******************************************************************************/
/**
 * Marks this object as due for processing. NOTE THAT THIS FUNCTION IS DEPRECATED.
 */
void GParameterSet::setDirtyFlag() {
	this->mark_as_due_for_processing();
}

/* ----------------------------------------------------------------------------------
 * Tested in GTestIndividual1::specificTestsNoFailureExpected_GUnitTests()
 * Tested in GParameterSet::specificTestsNoFailureExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Allows to retrieve the m_maxmode parameter
 *
 * @return The current value of the m_maxmode parameter
 */
maxMode GParameterSet::getMaxMode() const {
	return m_maxmode;
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
	return ((maxMode::MAXIMIZE == this->getMaxMode()) ? boost::numeric::bounds<double>::lowest() : boost::numeric::bounds<double>::highest());
}

/******************************************************************************/
/**
 * Retrieves the best possible evaluation result, depending on whether we are in
 * maximization or minimization mode
 */
double GParameterSet::getBestCase() const {
	return ((maxMode::MAXIMIZE == this->getMaxMode()) ? boost::numeric::bounds<double>::highest() : boost::numeric::bounds<double>::lowest());
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
 * Sets the maximum number of adaption attempts that may pass without
 * actual modifications. Setting this to 0 disables this check. You should only
 * do this if you are sure that an adaption will eventually happen. Otherwise
 * you would get an endless loop.
 */
void GParameterSet::setMaxUnsuccessfulAdaptions(std::size_t maxUnsuccessfulAdaptions) {
	m_max_unsuccessful_adaptions = maxUnsuccessfulAdaptions;
}

/******************************************************************************/
/**
 * Retrieves the maximum number of adaption attempts that may pass without
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
		<< "2 (a.k.a. USESIGMOID): Assign a multiple of m_validity_level and sigmoid barrier to invalid solutions, apply a sigmoid function to valid evaluations" << std::endl;

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
	gpb.registerFileParameter<maxMode>(
		"maxmode" // The name of the variable
		, maxMode::MINIMIZE // The default value
		, [this](maxMode mm) { this->setMaxMode(mm); }
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
 * Emits a name for this class / object
 *
 * @return The name of this class / object
 */
std::string GParameterSet::name() const {
	return std::string("GParameterSet");
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
 * for "clean" individuals only and will throw when called for unprocessed or
 * erroneous individuals.
 */
bool GParameterSet::isValid() const {
#ifdef DEBUG
	if (this->is_due_for_processing() || this->has_errors()) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GParameterSet::isValid():" << std::endl
				<< "Function was called for unprocessed or erroneous individual" << std::endl
		);
	}
#endif

	if (m_validity_level <= 1.) {
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
	return !this->isValid();
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
 * Performs all necessary (remote-)processing steps for this object.
 */
void GParameterSet::process_() {
	// Find out, whether this is a valid solution
	if (
		this->parameterSetFulfillsConstraints(m_validity_level) // Needs to be called first, or else the m_validity_level will not be filled
		|| evaluationPolicy::USESIMPLEEVALUATION == m_eval_policy
	) {
		// Trigger actual fitness calculation using the user-supplied function. This will
		// also register any secondary "raw" fitness values used in multi-criterion optimization.
		// Transformation of values is taken care of below.
		double main_raw_result = 0.;

		try {
			main_raw_result = this->fitnessCalculation();
		} catch(...) {
			// Make sure we invalidate all fitness values, if an exception was thrown
			this->setAllFitnessTo(this->getWorstCase());

			// Rethrow the exception
			throw;
		}

		// Make sure the main result is stored
		this->setResult(0, main_raw_result);
		this->getStoredResult(0).setTransformedFitnessToRaw();

		// Take care of erroneous calculations, flagged by the user. It is assumed here that marking
		// entire solutions as invalid after the evaluation happens relatively rarely so that a flat
		// "worst" quality surface for such solutions does not hinder progress of the optimization
		// procedure too much
		if (this->error_flagged_by_user()) { // has the user indicated a problem without throwing an error ?
			// Fill the raw and transformed vectors with the worst case scenario.
			this->setAllFitnessTo(this->getWorstCase());
		} else { // So this is a valid solution!
			for (std::size_t i = 0; i < this->getNStoredResults(); i++) {
				if (evaluationPolicy::USESIGMOID == m_eval_policy) { // Update the fitness value to use sigmoidal values
					this->getStoredResult(i).setTransformedFitnessWith(
						[this](double rawValue) {
							return Gem::Common::gsigmoid(
								rawValue
								, this->m_sigmoid_extremes
								, this->m_sigmoid_steepness
							);
						}
					);
				} else { // All other transformation policies use the same value for the transformed fitness as a (valid) raw fitness
					this->getStoredResult(i).setTransformedFitnessToRaw();
				}
			}
		}
	} else { // Some constraints were violated. Act on the chosen policy
		if (evaluationPolicy::USEWORSTCASEFORINVALID == m_eval_policy) {
			this->setAllFitnessTo(this->getWorstCase());
		} else if (evaluationPolicy::USESIGMOID == m_eval_policy) {
			double uniformFitnessValue = 0.;
			if (maxMode::MAXIMIZE == this->getMaxMode()) { // maximize
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

			this->setAllFitnessTo(this->getWorstCase(), uniformFitnessValue);
		}
	}
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
	Gem::Courtier::GProcessingContainerT<GParameterSet, parameterset_processing_result>::load_pc(p_load);

	// and then our local data
	m_perItemCrossOverProbability = p_load->m_perItemCrossOverProbability;
	m_best_past_primary_fitness = p_load->m_best_past_primary_fitness;
	m_n_stalls = p_load->m_n_stalls;
	m_maxmode = p_load->m_maxmode;
	m_assigned_iteration = p_load->m_assigned_iteration;
	m_validity_level = p_load->m_validity_level;
	m_eval_policy = p_load->m_eval_policy;
	m_sigmoid_steepness = p_load->m_sigmoid_steepness;
	m_sigmoid_extremes = p_load->m_sigmoid_extremes;
	m_max_unsuccessful_adaptions = p_load->m_max_unsuccessful_adaptions;
	m_max_retries_until_valid = p_load->m_max_retries_until_valid;
	m_n_adaptions = p_load->m_n_adaptions;

	Gem::Common::copyCloneableSmartPointer(p_load->m_pt_ptr, m_pt_ptr);
	Gem::Common::copyCloneableSmartPointer(p_load->m_individual_constraint_ptr, m_individual_constraint_ptr);
}

/******************************************************************************/
/**
 * Allows to randomly initialize parameter members. This function may be overloaded
 * by derived classes, but should be called by them. This function recursively initializes
 * parameters randomly.
 *
 * @return A boolean indicating whether modifications where made
 */
bool GParameterSet::randomInit_(const activityMode &am) {
	bool modifications_made = false;

	// Trigger random initialization of all our parameter objects
	for(auto& parm_ptr: *this) {
		if(parm_ptr->randomInit(am, m_gr)) {
			modifications_made=true;
		}
	}

	// This also takes care of empty parameter sets, as modifications_made
	// will remain false in this case.
	return modifications_made;
}

/* ----------------------------------------------------------------------------------
 * Tested in GParameterSet::specificTestsNoFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/**********************************************************************************/
/**
 * The actual adaption operations. Easy, as we know that all objects
 * in this collection must implement the adapt() function, as they are
 * derived from the GMutableI class / interface.
 */
std::size_t GParameterSet::customAdaptions() {
	std::size_t nAdaptions = 0;
	for (const auto& par_ptr: *this) {
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
 * Sets the fitness to a given set of values and clears the dirty flag. This is meant
 * to be used by external methods of performing the actual evaluation, such as the
 * OpenCL-Consumer. The fitness vector is interpreted as raw fitness values, and
 * transformed fitness values are calculated as needed.
 *
 * @param f_vec A vector of raw fitness values
 */
void GParameterSet::setFitness_(const std::vector<double> &f_vec) {
#ifdef DEBUG
	if(f_vec.size() != this->getNStoredResults()) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG, time_and_place)
				<< "In GParameterSet::setFitness_(...): Error!" << std::endl
				<< "Invalid size of fitness vector: " << std::endl
				<< f_vec.size() << ", expected: " << this->getNStoredResults() << std::endl
		);
	}
#endif /* DEBUG */

	// Find out, whether this is a valid solution
	if (
		this->parameterSetFulfillsConstraints(m_validity_level) // Needs to be called first, or else the m_validity_level will not be filled
		|| evaluationPolicy::USESIMPLEEVALUATION == m_eval_policy
	) {
	   // Create a vector of parameterset_processing_result objects
		std::vector<parameterset_processing_result> processing_results(f_vec.size(), parameterset_processing_result());

		// Take care of the transformed fitness
		std::size_t pos = 0;
		for(auto& p: processing_results) {
			// Set the raw fitness
			p.reset(f_vec.at(pos));

			if (evaluationPolicy::USESIGMOID == m_eval_policy) { // Update the fitness value to use sigmoidal values
				p.setTransformedFitnessWith(
					[this](double rawValue) {
						return Gem::Common::gsigmoid(
							rawValue
							, this->m_sigmoid_extremes
							, this->m_sigmoid_steepness
						);
					}
				);
			} else { // All other transformation policies use the same value for the transformed fitness as a (valid) raw fitness
				p.setTransformedFitnessToRaw();
			}

			pos++;
		}

		// Transfer the data into the individual
		this->markAsProcessedWith(processing_results);
	} else { // Some constraints were violated. Act on the chosen policy
		if (evaluationPolicy::USEWORSTCASEFORINVALID == m_eval_policy) {
			this->setAllFitnessTo(this->getWorstCase());
		} else if (evaluationPolicy::USESIGMOID == m_eval_policy) {
			double uniformFitnessValue = 0.;
			if (maxMode::MAXIMIZE == this->getMaxMode()) { // maximize
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

			this->setAllFitnessTo(this->getWorstCase(), uniformFitnessValue);
		}
	}
}

/******************************************************************************/
/**
 * Combines evaluation results by adding the individual results
 *
 *  @return The result of the combination
 */
double GParameterSet::sumCombiner() const {
	double result = 0.;

	for(std::size_t id=0; id < this->getNStoredResults(); id++) {
		result += this->transformed_fitness(id);
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

	for(std::size_t id=0; id < this->getNStoredResults(); id++) {
		result += Gem::Common::gfabs(this->transformed_fitness(id));
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

	for(std::size_t id=0; id < this->getNStoredResults(); id++) {
		result += GSQUARED(this->transformed_fitness(id));
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
	if (this->getNStoredResults() != weights.size()) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GParameterSet::weighedSquaredSumCombine(): Error!" << std::endl
				<< "Sizes of transformedCurrentFitnessVec_ and the weights vector don't match: " << this->getNStoredResults() <<
				" / " << weights.size() << std::endl
		);
	}

	double result = 0.;
	auto cit_weights = weights.begin();

	for(std::size_t id=0; id < this->getNStoredResults(); id++, ++cit_weights) {
		result += GSQUARED((*cit_weights) * this->transformed_fitness(id));
	}

	return sqrt(result);
}

/******************************************************************************/
/**
 * Allows users to mark this solution as invalid in derived classes (usually
 * from within the evaluation function). NOTE THAT THIS FUNCTION IS DEPRECATED.
 * USE force_set_error("some message") instead.
 */
void GParameterSet::markAsInvalid() {
	this->force_set_error("Error flagged via GParameterSet::markAsInvalid()");
}

/******************************************************************************/
/**
 * Allows to check whether this solution was marked as invalid. NOTE THAT THIS
 * FUNCTION IS DEPRECATED. USE has_errors() instead.
 */
bool GParameterSet::markedAsInvalidByUser() const {
	return this->has_errors();
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

/***************************************************************************/
/**
 * Allows to set all fitnesses to the same value (raw and transformed values seperately)
 */
void GParameterSet::setAllFitnessTo(
	double rawValue
	, double transformedValue
) {
	for (std::size_t i = 0; i < this->getNStoredResults(); i++) {
		this->getStoredResult(i).reset(rawValue);
		this->getStoredResult(i).setTransformedFitnessTo(transformedValue);
	}
}

/***************************************************************************/
/**
 * Allows to set all fitnesses to the same value (both raw and transformed values)
 */
void GParameterSet::setAllFitnessTo(double val) {
	this->setAllFitnessTo(val,val);
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

	for(const auto& o_ptr: *this) {
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

		BOOST_CHECK_NO_THROW(p_test->setMaxMode(maxMode::MAXIMIZE));
		BOOST_CHECK(p_test->getMaxMode() == maxMode::MAXIMIZE);
		BOOST_CHECK_NO_THROW(p_test->setMaxMode(maxMode::MINIMIZE));
		BOOST_CHECK(p_test->getMaxMode() == maxMode::MINIMIZE);
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

		double d = 0.;
		while(true) {
			BOOST_CHECK_NO_THROW(p_test->setBestKnownPrimaryFitness(std::make_tuple(d, d)));
			BOOST_CHECK_MESSAGE(
				p_test->getBestKnownPrimaryFitness() == std::make_tuple(d, d), "\n"
				<< "p_test->getBestKnownPrimaryFitness() = " << Gem::Common::g_to_string(p_test->getBestKnownPrimaryFitness()) << "\n"
				<< "d = " << d << "\n"
			);

			if((d+=0.1) >= 1.) break;
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
			std::shared_ptr<GParameterSet> p_test = p_test_0->clone<GParameterSet>();

			BOOST_CHECK_NO_THROW(p_test->randomInit(activityMode::ALLPARAMETERS));

			bool objects_equal = false;
			try {
				p_test->compare(
					*p_test_0
					, Gem::Common::expectation::CE_INEQUALITY
					, Gem::Common::CE_DEF_SIMILARITY_DIFFERENCE
				);
			} catch(const g_expectation_violation& e) {
				objects_equal = true;
			}

			BOOST_CHECK(false==objects_equal);
		}

		//-----------------------------------------------------------------
		{ // Test initialization of all fp parameters with a fixed value
			double d = FPFIXEDVALINITMIN;
			while(true) {
				// Create a GParameterSet object as a clone of p_test_0 for further usage
				std::shared_ptr <GParameterSet> p_test = p_test_0->clone<GParameterSet>();

				// Initialize all fp-values with 0.
				p_test->fixedValueInit<double>(d, activityMode::ALLPARAMETERS);

				// Make sure the dirty flag is set
				BOOST_CHECK(p_test->is_due_for_processing() == true);

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

				if((d+=1.) >= FPFIXEDVALINITMAX) break;
			}
		}

		//-----------------------------------------------------------------

		{ // Test multiplication of all fp parameters with a fixed value
			double d = -3.;
			while(true) {
				// Create a GParameterSet object as a clone of p_test_0 for further usage
				std::shared_ptr <GParameterSet> p_test = p_test_0->clone<GParameterSet>();

				// Initialize all fp-values with FPFIXEDVALINITMAX
				BOOST_CHECK_NO_THROW(p_test->fixedValueInit<double>(FPFIXEDVALINITMAX, activityMode::ALLPARAMETERS));

				// Multiply this fixed value by d
				BOOST_CHECK_NO_THROW(p_test->multiplyBy<double>(d, activityMode::ALLPARAMETERS));

				// Make sure the dirty flag is set
				BOOST_CHECK(p_test->is_due_for_processing() == true);

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

				if((d+=1.) >= 3.) break;
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
			BOOST_CHECK(p_test->is_due_for_processing() == true);

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
			BOOST_CHECK(p_test->is_due_for_processing() == true);

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
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

