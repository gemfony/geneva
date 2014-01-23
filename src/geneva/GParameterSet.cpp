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
 * The default constructor.
 */
GParameterSet::GParameterSet()
	: GMutableSetT<Gem::Geneva::GParameterBase>()
	, Gem::Courtier::GSubmissionContainerT<GParameterSet>()
{ /* nothing */ }

/******************************************************************************/
/**
 * The copy constructor. Note that we cannot rely on the operator=() of the vector
 * here, as we do not know the actual type of T objects.
 *
 * @param cp A copy of another GParameterSet object
 */
GParameterSet::GParameterSet(const GParameterSet& cp)
	: GMutableSetT<Gem::Geneva::GParameterBase>(cp)
	, Gem::Courtier::GSubmissionContainerT<GParameterSet>() // The data is intentionally not copied, as this class only stores a temporary parameter
{ /* nothing */ }

/******************************************************************************/
/**
 * The destructor
 */
GParameterSet::~GParameterSet()
{ /* nothing */ }

/******************************************************************************/
/**
 * A Standard assignment operator
 *
 * @param cp A copy of another GParameterSet object
 * @return A constant reference to this object
 */
const GParameterSet& GParameterSet::operator=(const GParameterSet& cp){
	GParameterSet::load_(&cp);
	return *this;
}

/******************************************************************************/
/**
 * Checks for equality with another GParameterSet object
 *
 * @param  cp A constant reference to another GParameterSet object
 * @return A boolean indicating whether both objects are equal
 */
bool GParameterSet::operator==(const GParameterSet& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GParameterSet::operator==","cp", CE_SILENT);
}

/******************************************************************************/
/**
 * Checks for inequality with another GParameterSet object
 *
 * @param  cp A constant reference to another GParameterSet object
 * @return A boolean indicating whether both objects are inequal
 */
bool GParameterSet::operator!=(const GParameterSet& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GParameterSet::operator!=","cp", CE_SILENT);
}

/******************************************************************************/
/**
 * Checks whether a given expectation for the relationship between this object and another object
 * is fulfilled.
 *
 * @param cp A constant reference to another object, camouflaged as a GObject
 * @param e The expected outcome of the comparison
 * @param limit The maximum deviation for floating point values (important for similarity checks)
 * @param caller An identifier for the calling entity
 * @param y_name An identifier for the object that should be compared to this one
 * @param withMessages Whether or not information should be emitted in case of deviations from the expected outcome
 * @return A boost::optional<std::string> object that holds a descriptive string if expectations were not met
 */
boost::optional<std::string> GParameterSet::checkRelationshipWith(
   const GObject& cp
   , const Gem::Common::expectation& e
   , const double& limit
   , const std::string& caller
   , const std::string& y_name
   , const bool& withMessages
) const {
	using namespace Gem::Common;

	// Check that we are not accidently assigning this object to itself
	GObject::selfAssignmentCheck<GParameterSet>(&cp);

	// Will hold possible deviations from the expectation, including explanations
	std::vector<boost::optional<std::string> > deviations;

	// Check our parent class'es data ...
	deviations.push_back(GMutableSetT<Gem::Geneva::GParameterBase>::checkRelationshipWith(cp, e, limit, "GParameterSet", y_name, withMessages));

	// ... and there is no local data

	return evaluateDiscrepancies("GParameterSet", caller, deviations, e);
}

/******************************************************************************/
/**
 * Emits a name for this class / object
 */
std::string GParameterSet::name() const {
   return std::string("GParameterSet");
}

/******************************************************************************/
/**
 * Retrieves a parameter of a given type at the specified position
 */
boost::any GParameterSet::getVarVal(
   const std::string& descr
   , const boost::tuple<std::size_t, std::string, std::size_t>& target
) {
   boost::any result;

   if(descr == "d") {
      result = GParameterSet::getVarItem<double>(target);
   } else if(descr == "f") {
      result = GParameterSet::getVarItem<float>(target);
   } else if(descr == "i") {
      result = GParameterSet::getVarItem<boost::int32_t>(target);
   } else if(descr == "b") {
      result = GParameterSet::getVarItem<bool>(target);
   } else {
      glogger
      << "In GParameterSet::getVarVal(): Error!" << std::endl
      << "Received invalid type description" << std::endl
      << GEXCEPTION;
   }

   return result;
}

/******************************************************************************/
/**
 * Checks whether this object is better than a given set of evaluations.
 */
bool GParameterSet::isGoodEnough(const std::vector<double>& boundaries) {
#ifdef DEBUG
   // Does the number of fitness criteria match the number of boundaries ?
   if(boundaries.size() != this->getNumberOfFitnessCriteria()) {
      glogger
      << "In GParameterSet::isGoodEnough(): Error!" << std::endl
      << "Number of boundaries does not match number of fitness criteria" << std::endl
      << GEXCEPTION;
   }

   // Is the dirty flag set ?
   if(this->isDirty()) {
      glogger
      << "In GParameterSet::isGoodEnough(): Error!" << std::endl
      << "Trying to compare fitness values although dirty flag is set" << std::endl
      << GEXCEPTION;
   }
#endif /* DEBUG */

   // Check the fitness values. If we find at least one
   // which is worse than the one supplied by the boundaries
   // vector, then this individual fails the test
   if(true == this->getMaxMode()) { // Maximization
      for(std::size_t i=0; i<boundaries.size(); i++) {
         if(this->fitness(i) < boundaries.at(i)) {
            return false;
         }
      }
   } else { // Minimization
      for(std::size_t i=0; i<boundaries.size(); i++) {
         if(this->fitness(i) > boundaries.at(i)) {
            return false;
         }
      }
   }

   // All fitness values are better than those supplied by boundaries
   return true;
}

/******************************************************************************/
/**
 * Loads the data of another GParameterSet object, camouflaged as a GObject.
 *
 * @param cp A copy of another GParameterSet object, camouflaged as a GObject
 */
void GParameterSet::load_(const GObject* cp){
	// Convert to local format
	// const GParameterSet *p_load = this->gobject_conversion<GParameterSet>(cp);
	// Uncomment the previous line and comment the following line if you wish to use local data
	GObject::selfAssignmentCheck<GParameterSet>(cp);

	// Load the parent class'es data
	GMutableSetT<Gem::Geneva::GParameterBase>::load_(cp);

	// No local data
}

/******************************************************************************/
/**
 * Creates a deep clone of this object
 *
 * @return A deep clone of this object
 */
GObject* GParameterSet::clone_() const {
	return new GParameterSet(*this);
}

/******************************************************************************/
/**
 * The actual fitness calculation takes place here. Note: This function is a trap. You need to overload
 * this function in derived classes.
 *
 * @return The fitness of this object
 */
double GParameterSet::fitnessCalculation() {
   glogger
   << "In GParameterSet::fitnessCalculation()" << std::endl
   << "Function called directly which should not happen" << std::endl
   << GEXCEPTION;

	// Make the compiler happy
	return 0.;
}

/******************************************************************************/
/**
 * Allows to randomly initialize parameter members
 */
void GParameterSet::randomInit() {
	// Trigger random initialization of all our parameter objects
	GParameterSet::iterator it;
	for(it=this->begin(); it!=this->end(); ++it) {
		(*it)->randomInit();
	}

	// As we have modified our internal data sets, make sure the dirty flag is set
	GOptimizableEntity::setDirtyFlag();
}

/* ----------------------------------------------------------------------------------
 * Tested in GParameterSet::specificTestsNoFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * This function performs a cross-over with another GParameterSet object with a given likelihood.
 * Items subject to cross-over may either be located in the GParameterSet-root or in one of the
 * GParmeterBase-derivatives stored in this object. Hence the cross-over operation is propagated to
 * these. A likelihood specifies how likely a cross-over will actually be performed. Note that thus more
 * than one cross-over may occur if more than one parameter collection is stored in this object.
 * Cross-Over will be skipped if a different composition of the other object is detected.
 *
 * TODO: Not yet ready
 */
void GParameterSet::crossOver(GParameterSet& cp, const double& likelihood) {
#ifdef DEBUG
	// Do some error checking
	if(likelihood < 0. || likelihood > 1.) {
	   glogger
	   << "In GParameterSet::crossOver(): Error!" << std::endl
	   << "Received invalid likelihood: " << likelihood << std::endl
	   << GEXCEPTION;
	}
#endif /* DEBUG */

	// Check the architecture and leave, if it is different. Cross-over does not make sense
	// if there are differences. These are only tolerated inside of parameter collections of
	// the same type.
	// a) We require both objects to have the same size
	if(this->size() != cp.size()) {
		return;
	}
	// b) We require both objects to have individual parameters in the same locations.
	// We count the number of individual parameters along the way
	std::size_t nIndividualParameters = 0;
	for(std::size_t i=0; i<this->size(); i++) {
		if(this->at(i)->isIndividualParameter()) {
			if((cp.at(i))->isParameterCollection()) {
				// This will terminate all cross-over for the entire object
				return;
			} else {
				nIndividualParameters++;
			}
		}
	}

	// Calculate a random number in the range [0,1[ . We only cross over at the root
	// level if this probe is below the "likelihood" threshold
	double probe = gr.uniform_01<double>();
	if(probe <= likelihood) {
		// Cross over at a random position inside of the root-level's individual objects
		// a) Determine a suitable swap-over position. swap_pos indicates the first position
		// where a swap should be carried out. Note that swapping from position 0 onwards
		// does not make sense. Hence we only act if there is more than one individual parameter.
		if(nIndividualParameters > 1) {
			std::size_t swap_pos = nIndividualParameters==2?1:gr.uniform_int<std::size_t>(1,nIndividualParameters-1);
			std::size_t actualIndPos = 0;

			for(std::size_t i=0; i<this->size(); i++) {
				if(this->at(i)->isIndividualParameter()) {
					if(actualIndPos >= swap_pos) {
						std::swap((this->data).at(i), cp.data.at(i));
					}
					actualIndPos++;
				}
			}
		}
	}

	// Now propagate the cross-over command to all contained parameter collections
	for(std::size_t i=0; i<this->size(); i++) {
		if(this->at(i)->isParameterCollection()) {
			// this->at(i)->crossOver(cp.at(i));
		}
	}
}

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
void GParameterSet::setMaxMode(const bool& mode) {
	this->setMaxMode_(mode);
}

/* ----------------------------------------------------------------------------------
 * So far untested
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Recursively initializes floating-point-based parameters with a given value. Allows e.g. to set all
 * floating point parameters to 0. "float" is used as the largest common denominator of float and double
 * types.
 *
 * @param val The value to be assigned to the parameters
 */
void GParameterSet::fpFixedValueInit(const float& val) {
	// Loop over all GParameterBase objects
	GParameterSet::iterator it;
	for(it=this->begin(); it!=this->end(); ++it) {
		(*it)->fpFixedValueInit(val);
	}

	// As we have modified our internal data sets, make sure the dirty flag is set
	GOptimizableEntity::setDirtyFlag();
}

/* ----------------------------------------------------------------------------------
 * Tested in GParameterSet::specificTestsNoFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Multiplies floating-point-based parameters with a given value.
 *
 * @param val The value to be multiplied with parameters
 * @param dummy A dummy parameter needed to ensure that fp_type is indeed a floating point value
 */
void GParameterSet::fpMultiplyBy(const float& val) {
	// Loop over all GParameterBase objects
	GParameterSet::iterator it;
	for(it=this->begin(); it!=this->end(); ++it) {
		(*it)->fpMultiplyBy(val);
	}

	// As we have modified our internal data sets, make sure the dirty flag is set
	GOptimizableEntity::setDirtyFlag();
}

/* ----------------------------------------------------------------------------------
 * Tested in GParameterSet::specificTestsNoFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Triggers multiplication of floating point parameters with a random floating point number in a given range
 *
 * @param min The lower boundary for random number generation
 * @param max The upper boundary for random number generation
 */
void GParameterSet::fpMultiplyByRandom(const float& min, const float& max) {
	// Loop over all GParameterBase objects
	GParameterSet::iterator it;
	for(it=this->begin(); it!=this->end(); ++it) {
		(*it)->fpMultiplyByRandom(min, max);
	}

	// As we have modified our internal data sets, make sure the dirty flag is set
	GOptimizableEntity::setDirtyFlag();
}

/* ----------------------------------------------------------------------------------
 * Tested in GParameterSet::specificTestsNoFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Triggers multiplication of floating point parameters with a random floating point number in the range [0,1[
 */
void GParameterSet::fpMultiplyByRandom() {
	// Loop over all GParameterBase objects
	GParameterSet::iterator it;
	for(it=this->begin(); it!=this->end(); ++it) {
		(*it)->fpMultiplyByRandom();
	}

	// As we have modified our internal data sets, make sure the dirty flag is set
	GOptimizableEntity::setDirtyFlag();
}

/* ----------------------------------------------------------------------------------
 * Tested in GParameterSet::specificTestsNoFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Adds the floating point parameters of another GParameterSet object to this one
 *
 * @param cp A boost::shared_ptr to another GParameterSet object whose floating point parameters should be added to this one
 */
void GParameterSet::fpAdd(boost::shared_ptr<GParameterSet> p) {
	// Some error checking
	if(p->size() != this->size()) {
	   glogger
	   << "In GParameterSet::fpAdd():" << std::endl
      << "Sizes do not match: " << this->size() << " " << p->size() << std::endl
      << GEXCEPTION;
	}

	// Loop over all GParameterBase objects in this and the other object
	GParameterSet::iterator it;
	GParameterSet::const_iterator cit;
	// Note that the GParameterBase objects need to accept a
	// boost::shared_ptr<GParameterBase>, contrary to the calling conventions
	// of this function.
	for(it=this->begin(), cit=p->begin(); it!=this->end(); ++it, ++cit) {
		(*it)->fpAdd(*cit);
	}

	// As we have modified our internal data sets, make sure the dirty flag is set
	GOptimizableEntity::setDirtyFlag();
}

/* ----------------------------------------------------------------------------------
 * Tested in GParameterSet::specificTestsNoFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Subtract the floating point parameters of another GParameterSet object from this one
 *
 * @param cp A boost::shared_ptr to another GParameterSet object whose floating point parameters should be subtracted from this one
 */
void GParameterSet::fpSubtract(boost::shared_ptr<GParameterSet> p) {
	// Some error checking
	if(p->size() != this->size()) {
	   glogger
	   << "In GParameterSet::fpAdd():" << std::endl
      << "Sizes do not match: " << this->size() << " " << p->size() << std::endl
      << GEXCEPTION;
	}

	// Loop over all GParameterBase objects in this and the other object
	GParameterSet::iterator it;
	GParameterSet::iterator it_p;
	// Note that the GParameterBase objects need to accept a
	// boost::shared_ptr<GParameterBase>, contrary to the calling conventions
	// of this function.
	for(it=this->begin(), it_p=p->begin(); it!=this->end(); ++it, ++it_p) {
		(*it)->fpSubtract(*it_p);
	}

	// As we have modified our internal data sets, make sure the dirty flag is set
	GOptimizableEntity::setDirtyFlag();
}

/* ----------------------------------------------------------------------------------
 * Tested in GParameterSet::specificTestsNoFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Emits a GParameterSet object that only has clones of our GParameterBase objects attached to it
 *
 * @return A GParameterSet object that only has clones of our GParameterBase objects attached to it
 */
boost::shared_ptr<GParameterSet> GParameterSet::parameter_clone() const {
	boost::shared_ptr<GParameterSet> result(new GParameterSet());
	GParameterSet::const_iterator cit;
	for(cit=this->begin(); cit!=this->end(); ++cit) {
		result->push_back((*cit)->clone<GParameterBase>());
	}
	result->setDirtyFlag(false); // Make sure the parameter set is clean.
	return result;
}

/* ----------------------------------------------------------------------------------
 * So far untested
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Performs all necessary (remote-)processing steps for this object.
 *
 * @return A boolean which indicates whether processing has led to a useful result
 */
bool GParameterSet::process(){
   // Make sure GParameterBase objects are updated with our local random number generator
   this->updateRNGs();

   // Record the previous setting of the serverMode_ flag and make
   // sure that re-evaluation is possible
   bool previousServerMode=setServerMode(false);

   this->fitness();

   // Restore the serverMode_ flag
   setServerMode(previousServerMode);

   // Restore the local random number generators in the individuals
   this->restoreRNGs();

   // Let the audience know that we were successful
   return true;
}

/******************************************************************************/
/**
 * Updates the random number generators contained in this object's GParameterBase-derivatives
 */
void GParameterSet::updateRNGs() {
	GParameterSet::iterator it;
	for(it=this->begin(); it!=this->end(); ++it) {
		(*it)->assignGRandomPointer(&(GMutableSetT<Gem::Geneva::GParameterBase>::gr));
	}
}

/* ----------------------------------------------------------------------------------
 * - Assigning a random number generator is tested in GParameterSet::specificTestsNoFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Restores the local random number generators contained in this object's GParameterBase-derivatives
 */
void GParameterSet::restoreRNGs() {
	GParameterSet::iterator it;
	for(it=this->begin(); it!=this->end(); ++it) {
		(*it)->resetGRandomPointer();
	}
}

/* ----------------------------------------------------------------------------------
 * - Restoring the random number generators is tested in GParameterSet::specificTestsNoFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Checks whether all GParameterBase derivatives use local random number generators. The function will return
 * false if at least one object is found in this collection that does not use a local RNG.
 *
 * @return A boolean which indicates whether all objects in this collection use local random number generators
 */
bool GParameterSet::localRNGsUsed() const {
	bool result = true;

	GParameterSet::const_iterator it;
	for(it=this->begin(); it!=this->end(); ++it) {
		if(!(*it)->usesLocalRNG()) result = false;
	}

	return result;
}

/* ----------------------------------------------------------------------------------
 * Tested in GParameterSet::specificTestsNoFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Checks whether all GParameterBase derivatives use the assigned random number generator. The function will return
 * false if at least one object is found in this collection that uses a local RNG.
 *
 * @return A boolean which indicates whether all objects in this collection use the assigned random number generator
 */
bool GParameterSet::assignedRNGUsed() const {
	bool result = true;

	GParameterSet::const_iterator it;
	for(it=this->begin(); it!=this->end(); ++it) {
		if((*it)->usesLocalRNG()) result = false;
	}

	return result;
}

/* ----------------------------------------------------------------------------------
 * Tested in GParameterSet::specificTestsNoFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Prevent shadowing of std::vector<GParameterBase>::at()
 *
 * @param pos The position of the item we aim to retrieve from the std::vector<GParameterBase>
 * @return The item we aim to retrieve from the std::vector<GParameterBase>
 */
boost::shared_ptr<Gem::Geneva::GParameterBase> GParameterSet::at(const std::size_t& pos) {
	return data.at(pos);
}

/* ----------------------------------------------------------------------------------
 * So far untested
 * ----------------------------------------------------------------------------------
 */

/**********************************************************************************/
/**
 * The actual adaption operations. Easy, as we know that all objects
 * in this collection must implement the adapt() function, as they are
 * derived from the GMutableI class / interface.
 */
void GParameterSet::customAdaptions() {
	GParameterSet::iterator it;
	for(it=this->begin(); it!=this->end(); ++it) {
		(*it)->adapt();
	}
}

/* ----------------------------------------------------------------------------------
 * Tested in GTestIndividual1::specificTestsNoFailureExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Adds local configuration options to a GParserBuilder object
 *
 * @param gpb The GParserBuilder object to which configuration options should be added
 * @param showOrigin Makes the function indicate the origin of parameters in comments
 */
void GParameterSet::addConfigurationOptions (
	Gem::Common::GParserBuilder& gpb
	, const bool& showOrigin
) {
	std::string comment;

	// Call our parent class'es function
	GMutableSetT<Gem::Geneva::GParameterBase>::addConfigurationOptions(gpb, showOrigin);

	// Add local data
	comment = ""; // Reset the comment string
	comment += "Specifies whether the individual should be maximized (1) or minimized (0);";
	comment += "Note that minimization is the by far most common option.;";
	if(showOrigin) comment += "[GParameterset]";
	gpb.registerFileParameter<boost::uint32_t>(
		"maximize" // The name of the variable
		, false // The default value
		, boost::bind(
			&GParameterSet::setMaxMode
			, this
			, _1
		  )
		, Gem::Common::VAR_IS_SECONDARY // Alternative: VAR_IS_ESSENTIAL
		, comment
	);
}

/******************************************************************************/
/**
 * Allows to assign a name to the role of this individual(-derivative). This is mostly important for the
 * GBrokerEA class which should prevent objects of its type from being stored as an individual in its population.
 * All other objects do not need to re-implement this function (unless they rely on the name for some reason).
 */
std::string GParameterSet::getIndividualCharacteristic() const {
	return std::string("GENEVA_PARAMETERSET");
}

/******************************************************************************/
/**
 * Provides access to all data stored in the individual in a user defined selection
 *
 * @param var_vec A std::vector of user-defined types
 */
void GParameterSet::custom_streamline(std::vector<boost::any>& var_vec)
{ /* nothing -- override in user-code */ }

/******************************************************************************/
/**
 * Transformation of the individual's parameter objects into a boost::property_tree object
 */
void GParameterSet::toPropertyTree(
   pt::ptree& ptr
   , const std::string& baseName
) const {
#ifdef DEBUG
   // Check if the object is empty. If so, complain
   if(this->empty()) {
      glogger
      << "In GParameterSet::toPropertyTree(): Error!" << std::endl
      << "Object is empty." << std::endl
      << GEXCEPTION;
   }
#endif

   ptr.put(baseName + ".iteration", this->getAssignedIteration());
   ptr.put(baseName + ".isDirty"  , this->isDirty());
   ptr.put(baseName + ".isValid"  , this->isDirty()?false:this->isValid());
   ptr.put(baseName + ".type"     , std::string("GParameterSet"));

   // Loop over all parameter objects and ask them to add their data to our ptree object
   ptr.put(baseName + ".nVars"     , this->size());
   std::string base;
   std::size_t pos;
   GParameterSet::const_iterator cit;
   for(cit=this->begin(); cit!=this->end(); ++cit) {
      pos = std::distance(this->begin(), cit);
      base = baseName + ".vars.var" + boost::lexical_cast<std::string>(pos);
      (*cit)->toPropertyTree(ptr, base);
   }

   // Output the transformation policy
   switch(this->getEvaluationPolicy()) {
      case USESIMPLEEVALUATION:
      ptr.put(baseName + "transformationPolicy", "USESIMPLEEVALUATION");
      break;

      case USESIGMOID:
      ptr.put(baseName + "transformationPolicy", "USESIGMOID");
      break;

      case USEWORSTKNOWNVALIDFORINVALID:
      ptr.put(baseName + "transformationPolicy", "USEWORSTKNOWNVALIDFORINVALID");
      break;

      default:
      {
         glogger
         << "In GParameterSet::toPropertyTree(): Error!" << std::endl
         << "Got invalid evaluation policy: " << this->getEvaluationPolicy() << std::endl
         << GEXCEPTION;
      }
      break;
   }

   // Output all fitness criteria. We do not enforce re-calculation of the fitness here.
   // Check the "isDirty" tag, if you need to know whether the results are current.
   ptr.put(baseName + ".nResults", this->getNumberOfFitnessCriteria());
   bool dirtyFlag;
   for(std::size_t f=0; f<this->getNumberOfFitnessCriteria(); f++) {
      base = baseName + ".results.result" + boost::lexical_cast<std::string>(f);
      ptr.put(base, this->getCachedFitness(dirtyFlag, f));
      base = baseName + ".results.trueResult"  + boost::lexical_cast<std::string>(f);
      ptr.put(base, this->getTrueCachedFitness(dirtyFlag, f));
   }
}

/******************************************************************************/
/**
 * Transformation of the individual's parameter objects into a list of
 * comma-separated values and fitness
 *
 * @param  withNameAndType Indicates whether a list of names and types should be prepended
 * @param  withCommas Indicates, whether commas should be printed
 * @return A string holding the parameter values and possibly the types
 */
std::string GParameterSet::toCSV(bool withNameAndType, bool withCommas) const {
   std::map<std::string, std::vector<double> > dData;
   std::map<std::string, std::vector<float> > fData;
   std::map<std::string, std::vector<boost::int32_t> > iData;
   std::map<std::string, std::vector<bool> > bData;

   std::map<std::string, std::vector<double> >::const_iterator d_it;
   std::map<std::string, std::vector<float> >::const_iterator f_it;
   std::map<std::string, std::vector<boost::int32_t> >::const_iterator i_it;
   std::map<std::string, std::vector<bool> >::const_iterator b_it;

   // Retrieve the parameter maps
   this->streamline<double>(dData);
   this->streamline<float>(fData);
   this->streamline<boost::int32_t>(iData);
   this->streamline<bool>(bData);

   std::vector<std::string> varNames;
   std::vector<std::string> varTypes;
   std::vector<std::string> varValues;

   std::vector<std::string>::const_iterator s_it;

   // Extract the data
   for(d_it=dData.begin(); d_it!=dData.end(); ++d_it) {
      for(std::size_t pos=0; pos<(d_it->second).size(); pos++) {
         if(withNameAndType) {
            varNames.push_back(d_it->first + "_" + boost::lexical_cast<std::string>(pos));
            varTypes.push_back("double");
         }
         varValues.push_back(boost::lexical_cast<std::string>((d_it->second).at(pos)));
      }
   }

   for(f_it=fData.begin(); f_it!=fData.end(); ++f_it) {
      for(std::size_t pos=0; pos<(f_it->second).size(); pos++) {
         if(withNameAndType) {
            varNames.push_back(f_it->first + "_" + boost::lexical_cast<std::string>(pos));
            varTypes.push_back("float");
         }
         varValues.push_back(boost::lexical_cast<std::string>((f_it->second).at(pos)));
      }
   }

   for(i_it=iData.begin(); i_it!=iData.end(); ++i_it) {
      for(std::size_t pos=0; pos<(i_it->second).size(); pos++) {
         if(withNameAndType) {
            varNames.push_back(i_it->first + "_" + boost::lexical_cast<std::string>(pos));
            varTypes.push_back("int32");
         }
         varValues.push_back(boost::lexical_cast<std::string>((i_it->second).at(pos)));
      }
   }

   for(b_it=bData.begin(); b_it!=bData.end(); ++b_it) {
      for(std::size_t pos=0; pos<(b_it->second).size(); pos++) {
         if(withNameAndType) {
            varNames.push_back(b_it->first + "_" + boost::lexical_cast<std::string>(pos));
            varTypes.push_back("bool");
         }
         varValues.push_back(boost::lexical_cast<std::string>((b_it->second).at(pos)));
      }
   }

   // Add fitness name, type and value
   for(std::size_t f=0; f<this->getNumberOfFitnessCriteria(); f++) {
      if(withNameAndType) {
         varNames.push_back(std::string("Fitness_") + boost::lexical_cast<std::string>(f));
         varTypes.push_back("double");
      }
      bool isDirty;
      varValues.push_back(boost::lexical_cast<std::string>(this->getCachedFitness(isDirty, f)));

#ifdef DEBUG
      if(isDirty) {
         glogger
         << "In GParameterSet::toCSV(bool withNameAndType) const: Error!" << std::endl
         << "Got dirty individual when clean individual was expected" << std::endl
         << GEXCEPTION;
      }
#endif
   }


   // Transfer the data into the result string
   std::ostringstream result;

   if(withNameAndType) {
      for(s_it=varNames.begin(); s_it!=varNames.end(); ++s_it) {
         result << *s_it;
         if(s_it+1 != varNames.end()) {
            result << (withCommas?",\t":"\t");
         }
      }
      result << std::endl;

      for(s_it=varTypes.begin(); s_it!=varTypes.end(); ++s_it) {
         result << *s_it;
         if(s_it+1 != varTypes.end()) {
            result << (withCommas?",\t":"\t");
         }
      }
      result << std::endl;
   }

   for(s_it=varValues.begin(); s_it!=varValues.end(); ++s_it) {
      result << *s_it;
      if(s_it+1 != varValues.end()) {
         result << (withCommas?",\t":"\t");
      }
   }
   result << std::endl;

   return result.str();
}

/******************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GParameterSet::modify_GUnitTests() {
#ifdef GEM_TESTING
	bool result = false;

	// Call the parent class'es function
	if(GMutableSetT<Gem::Geneva::GParameterBase>::modify_GUnitTests()) result = true;

	return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GParameterSet::modify_GUnitTests", "GEM_TESTING");
   return false;
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GParameterSet::specificTestsNoFailureExpected_GUnitTests() {
#ifdef GEM_TESTING

	// Call the parent class'es function
	GMutableSetT<Gem::Geneva::GParameterBase>::specificTestsNoFailureExpected_GUnitTests();

	//---------------------------------------------------------------------

	{ // All tests below use the same, cloned collection
		// Some settings for the collection of tests below
		const double MINGCONSTRDOUBLE    = -4.;
		const double MAXGCONSTRDOUBLE    =  4.;
		const double MINGDOUBLE          = -5.;
		const double MAXGDOUBLE          =  5.;
		const double MINGDOUBLECOLL      = -3.;
		const double MAXGDOUBLECOLL      =  3.;
		const std::size_t NGDOUBLECOLL    = 10 ;
		const std::size_t FPLOOPCOUNT     =  5 ;
		const double FPFIXEDVALINITMIN   = -3.;
		const double FPFIXEDVALINITMAX   =  3.;
		const double FPMULTIPLYBYRANDMIN = -5.;
		const double FPMULTIPLYBYRANDMAX =  5.;
		const double FPADD               =  2.;
		const double FPSUBTRACT          =  2.;

		// Create a GParameterSet object as a clone of this object for further usage
		boost::shared_ptr<GParameterSet> p_test_0 = this->clone<GParameterSet>();
		// Clear the collection
		p_test_0->clear();
		// Make sure it is really empty
		BOOST_CHECK(p_test_0->empty());
		// Add some floating pount parameters
		for(std::size_t i=0; i<FPLOOPCOUNT; i++) {
			p_test_0->push_back(boost::shared_ptr<GConstrainedDoubleObject>(new GConstrainedDoubleObject(gr.uniform_real<double>(MINGCONSTRDOUBLE, MAXGCONSTRDOUBLE), MINGCONSTRDOUBLE, MAXGCONSTRDOUBLE)));
			p_test_0->push_back(boost::shared_ptr<GDoubleObject>(new GDoubleObject(gr.uniform_real<double>(MINGDOUBLE,MAXGDOUBLE))));
			p_test_0->push_back(boost::shared_ptr<GDoubleCollection>(new GDoubleCollection(NGDOUBLECOLL, MINGDOUBLECOLL, MAXGDOUBLECOLL)));
		}

		// Attach a few other parameter types
		p_test_0->push_back(boost::shared_ptr<GConstrainedInt32Object>(new GConstrainedInt32Object(7, -10, 10)));
		p_test_0->push_back(boost::shared_ptr<GBooleanObject>(new GBooleanObject(true)));

		//-----------------------------------------------------------------

		{ // Test setting and resetting of the random number generator
			// Create a GParameterSet object as a clone of p_test_0 for further usage
			boost::shared_ptr<GParameterSet> p_test = p_test_0->clone<GParameterSet>();

			// Distribute our own random number generator
			BOOST_CHECK_NO_THROW(p_test->updateRNGs());
			BOOST_CHECK(p_test->assignedRNGUsed() == true);

			// Restore the original generators in all objects in the container
			BOOST_CHECK_NO_THROW(p_test->restoreRNGs());
			BOOST_CHECK(p_test->localRNGsUsed() == true);
		}

		//-----------------------------------------------------------------

		{ // Test random initialization
			// Create a GParameterSet object as a clone of p_test_0 for further usage
			boost::shared_ptr<GParameterSet> p_test = p_test_0->clone<GParameterSet>();

			BOOST_CHECK_NO_THROW(p_test->randomInit());
			BOOST_CHECK(*p_test != *p_test_0);
		}

		//-----------------------------------------------------------------
		{ // Test initialization of all fp parameters with a fixed value
			for(double d=FPFIXEDVALINITMIN; d<FPFIXEDVALINITMAX; d+=1.) {
				// Create a GParameterSet object as a clone of p_test_0 for further usage
				boost::shared_ptr<GParameterSet> p_test = p_test_0->clone<GParameterSet>();

				// Initialize all fp-values with 0.
				p_test->fpFixedValueInit(d);

				// Make sure the dirty flag is set
				BOOST_CHECK(p_test->isDirty() == true);

				// Cross-check
				std::size_t counter = 0;
				for(std::size_t i=0; i<FPLOOPCOUNT; i++) {
					BOOST_CHECK(p_test->at<GConstrainedDoubleObject>(counter)->value() == d);
					counter++;
					BOOST_CHECK(p_test->at<GDoubleObject>(counter)->value() == d);
					counter++;
					boost::shared_ptr<GDoubleCollection> p_gdc;
					BOOST_CHECK_NO_THROW(p_gdc = p_test->at<GDoubleCollection>(counter));
					for(std::size_t gdc_cnt=0; gdc_cnt<NGDOUBLECOLL; gdc_cnt++) {
						BOOST_CHECK_MESSAGE (
								p_gdc->at(gdc_cnt) == d
								,  "\n"
								<< "p_gdc->at(gdc_cnt) = " << p_gdc->at(gdc_cnt) << "\n"
								<< "expected " << d << "\n"
								<< "iteration = " << gdc_cnt << "\n"
						);
					}
					counter++;
				}

				// The int32 parameter should have stayed the same
				boost::shared_ptr<GConstrainedInt32Object> p_int32_0;
				boost::shared_ptr<GConstrainedInt32Object> p_int32;
				BOOST_CHECK_NO_THROW(p_int32_0   = p_test_0->at<GConstrainedInt32Object>(counter));
				BOOST_CHECK_NO_THROW(p_int32 =   p_test->at<GConstrainedInt32Object>(counter));
				BOOST_CHECK(*p_int32_0 == *p_int32);
				counter++;

				// Likewise, the boolean parameter should have stayed the same
				boost::shared_ptr<GBooleanObject> p_boolean_orig;
				boost::shared_ptr<GBooleanObject> p_boolean_cloned;
				BOOST_CHECK_NO_THROW(p_boolean_orig   = p_test_0->at<GBooleanObject>(counter));
				BOOST_CHECK_NO_THROW(p_boolean_cloned =   p_test->at<GBooleanObject>(counter));
				BOOST_CHECK(*p_boolean_orig == *p_boolean_cloned);
				counter++;
			}
		}

		//-----------------------------------------------------------------

		{ // Test setting and retrieval of the maximization mode flag
			boost::shared_ptr<GParameterSet> p_test = p_test_0->clone<GParameterSet>();

			BOOST_CHECK_NO_THROW(p_test->setMaxMode(true));
			BOOST_CHECK(p_test->getMaxMode() == true);
			BOOST_CHECK_NO_THROW(p_test->setMaxMode(false));
			BOOST_CHECK(p_test->getMaxMode() == false);
		}

		//-----------------------------------------------------------------

		{ // Test multiplication of all fp parameters with a fixed value
			for(double d=-3; d<3; d+=1.) {
				// Create a GParameterSet object as a clone of p_test_0 for further usage
				boost::shared_ptr<GParameterSet> p_test = p_test_0->clone<GParameterSet>();

				// Initialize all fp-values with FPFIXEDVALINITMAX
				BOOST_CHECK_NO_THROW(p_test->fpFixedValueInit(FPFIXEDVALINITMAX));

				// Multiply this fixed value by d
				BOOST_CHECK_NO_THROW(p_test->fpMultiplyBy(d));

				// Make sure the dirty flag is set
				BOOST_CHECK(p_test->isDirty() == true);

				// Cross-check
				std::size_t counter = 0;
				for(std::size_t i=0; i<FPLOOPCOUNT; i++) {
					// A constrained value does not have to assume the value d*FPFIXEDVALINITMAX,
					// but needs to stay within its boundaries
					BOOST_CHECK(p_test->at<GConstrainedDoubleObject>(counter)->value() >= MINGCONSTRDOUBLE);
					BOOST_CHECK(p_test->at<GConstrainedDoubleObject>(counter)->value() <= MAXGCONSTRDOUBLE);
					counter++;
					BOOST_CHECK(p_test->at<GDoubleObject>(counter)->value() == d*FPFIXEDVALINITMAX);
					counter++;
					boost::shared_ptr<GDoubleCollection> p_gdc;
					BOOST_CHECK_NO_THROW(p_gdc = p_test->at<GDoubleCollection>(counter));
					for(std::size_t gdc_cnt=0; gdc_cnt<NGDOUBLECOLL; gdc_cnt++) {
						BOOST_CHECK_MESSAGE (
								p_gdc->at(gdc_cnt) == d*FPFIXEDVALINITMAX
								,  "\n"
								<< "p_gdc->at(gdc_cnt) = " << p_gdc->at(gdc_cnt) << "\n"
								<< "expected " << d*FPFIXEDVALINITMAX << "\n"
								<< "iteration = " << gdc_cnt << "\n"
						);
					}
					counter++;
				}

				// The int32 parameter should have stayed the same
				boost::shared_ptr<GConstrainedInt32Object> p_int32_0;
				boost::shared_ptr<GConstrainedInt32Object> p_int32;
				BOOST_CHECK_NO_THROW(p_int32_0   = p_test_0->at<GConstrainedInt32Object>(counter));
				BOOST_CHECK_NO_THROW(p_int32 =   p_test->at<GConstrainedInt32Object>(counter));
				BOOST_CHECK(*p_int32_0 == *p_int32);
				counter++;

				// Likewise, the boolean parameter should have stayed the same
				boost::shared_ptr<GBooleanObject> p_boolean_orig;
				boost::shared_ptr<GBooleanObject> p_boolean_cloned;
				BOOST_CHECK_NO_THROW(p_boolean_orig   = p_test_0->at<GBooleanObject>(counter));
				BOOST_CHECK_NO_THROW(p_boolean_cloned =   p_test->at<GBooleanObject>(counter));
				BOOST_CHECK(*p_boolean_orig == *p_boolean_cloned);
				counter++;
			}
		}

		//-----------------------------------------------------------------

		{ // Test that fpMultiplyByRandom(min,max) changes every single parameter
			// Create a GParameterSet object as a clone of p_test_0 for further usage
			boost::shared_ptr<GParameterSet> p_test       = p_test_0->clone<GParameterSet>();

			// Multiply each floating point value with a constrained random value
			BOOST_CHECK_NO_THROW(p_test->fpMultiplyByRandom(FPMULTIPLYBYRANDMIN, FPMULTIPLYBYRANDMAX));

			// Make sure the dirty flag is set
			BOOST_CHECK(p_test->isDirty() == true);

			// Cross-check
			std::size_t counter = 0;
			for(std::size_t i=0; i<FPLOOPCOUNT; i++) {
				BOOST_CHECK(p_test->at<GConstrainedDoubleObject>(counter)->value() != p_test_0->at<GConstrainedDoubleObject>(counter)->value());
				counter++;
				BOOST_CHECK(p_test->at<GDoubleObject>(counter)->value() != p_test_0->at<GDoubleObject>(counter)->value());
				counter++;
				boost::shared_ptr<GDoubleCollection> p_gdc;
				boost::shared_ptr<GDoubleCollection> p_gdc_0;
				BOOST_CHECK_NO_THROW(p_gdc   = p_test->at<GDoubleCollection>(counter));
				BOOST_CHECK_NO_THROW(p_gdc_0 = p_test_0->at<GDoubleCollection>(counter));
				for(std::size_t gdc_cnt=0; gdc_cnt<NGDOUBLECOLL; gdc_cnt++) {
					BOOST_CHECK_MESSAGE (
							p_gdc->at(gdc_cnt) != p_gdc_0->at(gdc_cnt)
							,  "\n"
							<< "p_gdc->at(gdc_cnt) = " << p_gdc->at(gdc_cnt) << "\n"
							<< "p_gdc_0->at(gdc_cnt) = " << p_gdc_0->at(gdc_cnt) << "\n"
							<< "iteration = " << gdc_cnt << "\n"
					);
				}
				counter++;
			}

			// The int32 parameter should have stayed the same
			boost::shared_ptr<GConstrainedInt32Object> p_int32_0;
			boost::shared_ptr<GConstrainedInt32Object> p_int32;
			BOOST_CHECK_NO_THROW(p_int32_0   = p_test_0->at<GConstrainedInt32Object>(counter));
			BOOST_CHECK_NO_THROW(p_int32 =   p_test->at<GConstrainedInt32Object>(counter));
			BOOST_CHECK(*p_int32_0 == *p_int32);
			counter++;

			// Likewise, the boolean parameter should have stayed the same
			boost::shared_ptr<GBooleanObject> p_boolean_orig;
			boost::shared_ptr<GBooleanObject> p_boolean_cloned;
			BOOST_CHECK_NO_THROW(p_boolean_orig   = p_test_0->at<GBooleanObject>(counter));
			BOOST_CHECK_NO_THROW(p_boolean_cloned =   p_test->at<GBooleanObject>(counter));
			BOOST_CHECK(*p_boolean_orig == *p_boolean_cloned);
			counter++;
		}

		//-----------------------------------------------------------------

		{ // Test that fpMultiplyByRandom() changes every single parameter
			// Create a GParameterSet object as a clone of p_test_0 for further usage
			boost::shared_ptr<GParameterSet> p_test       = p_test_0->clone<GParameterSet>();

			// Multiply each floating point value with a constrained random value
			BOOST_CHECK_NO_THROW(p_test->fpMultiplyByRandom());

			// Make sure the dirty flag is set
			BOOST_CHECK(p_test->isDirty() == true);

			// Cross-check
			std::size_t counter = 0;
			for(std::size_t i=0; i<FPLOOPCOUNT; i++) {
				BOOST_CHECK(p_test->at<GConstrainedDoubleObject>(counter)->value() != p_test_0->at<GConstrainedDoubleObject>(counter)->value());
				counter++;
				BOOST_CHECK(p_test->at<GDoubleObject>(counter)->value() != p_test_0->at<GDoubleObject>(counter)->value());
				counter++;
				boost::shared_ptr<GDoubleCollection> p_gdc;
				boost::shared_ptr<GDoubleCollection> p_gdc_0;
				BOOST_CHECK_NO_THROW(p_gdc   = p_test->at<GDoubleCollection>(counter));
				BOOST_CHECK_NO_THROW(p_gdc_0 = p_test_0->at<GDoubleCollection>(counter));
				for(std::size_t gdc_cnt=0; gdc_cnt<NGDOUBLECOLL; gdc_cnt++) {
					BOOST_CHECK_MESSAGE (
							p_gdc->at(gdc_cnt) != p_gdc_0->at(gdc_cnt)
							,  "\n"
							<< "p_gdc->at(gdc_cnt) = " << p_gdc->at(gdc_cnt) << "\n"
							<< "p_gdc_0->at(gdc_cnt) = " << p_gdc_0->at(gdc_cnt) << "\n"
							<< "iteration = " << gdc_cnt << "\n"
					);
				}
				counter++;
			}

			// The int32 parameter should have stayed the same
			boost::shared_ptr<GConstrainedInt32Object> p_int32_0;
			boost::shared_ptr<GConstrainedInt32Object> p_int32;
			BOOST_CHECK_NO_THROW(p_int32_0   = p_test_0->at<GConstrainedInt32Object>(counter));
			BOOST_CHECK_NO_THROW(p_int32 =   p_test->at<GConstrainedInt32Object>(counter));
			BOOST_CHECK(*p_int32_0 == *p_int32);
			counter++;

			// Likewise, the boolean parameter should have stayed the same
			boost::shared_ptr<GBooleanObject> p_boolean_orig;
			boost::shared_ptr<GBooleanObject> p_boolean_cloned;
			BOOST_CHECK_NO_THROW(p_boolean_orig   = p_test_0->at<GBooleanObject>(counter));
			BOOST_CHECK_NO_THROW(p_boolean_cloned =   p_test->at<GBooleanObject>(counter));
			BOOST_CHECK(*p_boolean_orig == *p_boolean_cloned);
			counter++;
		}

		//-----------------------------------------------------------------

		{ // Check adding of individuals
			// Create two GParameterSet objects as a clone of p_test_0 for further usage
			boost::shared_ptr<GParameterSet> p_test       = p_test_0->clone<GParameterSet>();
			boost::shared_ptr<GParameterSet> p_test_fixed = p_test_0->clone<GParameterSet>();

			// Initialize all fp-values of the "add" individual with a fixed valie
			BOOST_CHECK_NO_THROW(p_test_fixed->fpFixedValueInit(FPADD));

			// Add p_test_fixed to p_test
			BOOST_CHECK_NO_THROW(p_test->fpAdd(p_test_fixed));

			// Check the results
			std::size_t counter = 0;
			for(std::size_t i=0; i<FPLOOPCOUNT; i++) {
				// A constrained value does not have to assume the value value()+FPADD
				// but needs to stay within its boundaries
				BOOST_CHECK(p_test->at<GConstrainedDoubleObject>(counter)->value() >= MINGCONSTRDOUBLE);
				BOOST_CHECK(p_test->at<GConstrainedDoubleObject>(counter)->value() <= MAXGCONSTRDOUBLE);
				counter++;
				BOOST_CHECK(p_test->at<GDoubleObject>(counter)->value() == p_test_0->at<GDoubleObject>(counter)->value() + FPADD);
				counter++;
				boost::shared_ptr<GDoubleCollection> p_gdc;
				boost::shared_ptr<GDoubleCollection> p_gdc_0;
				BOOST_CHECK_NO_THROW(p_gdc   = p_test->at<GDoubleCollection>(counter));
				BOOST_CHECK_NO_THROW(p_gdc_0 = p_test_0->at<GDoubleCollection>(counter));
				for(std::size_t gdc_cnt=0; gdc_cnt<NGDOUBLECOLL; gdc_cnt++) {
					BOOST_CHECK_MESSAGE (
							p_gdc->at(gdc_cnt) == p_gdc_0->at(gdc_cnt) + FPADD
							,  "\n"
							<< "p_gdc->at(gdc_cnt) = " << p_gdc->at(gdc_cnt) << "\n"
							<< "p_gdc_0->at(gdc_cnt) = " << p_gdc_0->at(gdc_cnt) << "\n"
							<< "FPADD = " << FPADD
							<< "p_gdc_0->at(gdc_cnt) + FPADD = " << p_gdc_0->at(gdc_cnt) + FPADD << "\n"
							<< "iteration = " << gdc_cnt << "\n"
					);
				}
				counter++;
			}

			// The int32 parameter should have stayed the same
			boost::shared_ptr<GConstrainedInt32Object> p_int32_0;
			boost::shared_ptr<GConstrainedInt32Object> p_int32;
			BOOST_CHECK_NO_THROW(p_int32_0   = p_test_0->at<GConstrainedInt32Object>(counter));
			BOOST_CHECK_NO_THROW(p_int32 =   p_test->at<GConstrainedInt32Object>(counter));
			BOOST_CHECK(*p_int32_0 == *p_int32);
			counter++;

			// Likewise, the boolean parameter should have stayed the same
			boost::shared_ptr<GBooleanObject> p_boolean_orig;
			boost::shared_ptr<GBooleanObject> p_boolean_cloned;
			BOOST_CHECK_NO_THROW(p_boolean_orig   = p_test_0->at<GBooleanObject>(counter));
			BOOST_CHECK_NO_THROW(p_boolean_cloned =   p_test->at<GBooleanObject>(counter));
			BOOST_CHECK(*p_boolean_orig == *p_boolean_cloned);
			counter++;
		}

		//-----------------------------------------------------------------

		{ // Check subtraction of individuals
			// Create two GParameterSet objects as a clone of p_test_0 for further usage
			boost::shared_ptr<GParameterSet> p_test       = p_test_0->clone<GParameterSet>();
			boost::shared_ptr<GParameterSet> p_test_fixed = p_test_0->clone<GParameterSet>();

			// Initialize all fp-values of the "add" individual with a fixed valie
			BOOST_CHECK_NO_THROW(p_test_fixed->fpFixedValueInit(FPSUBTRACT));

			// Add p_test_fixed to p_test
			BOOST_CHECK_NO_THROW(p_test->fpSubtract(p_test_fixed));

			// Check the results
			std::size_t counter = 0;
			for(std::size_t i=0; i<FPLOOPCOUNT; i++) {
				// A constrained value does not have to assume the value value()-FPSUBTRACT
				// but needs to stay within its boundaries
				BOOST_CHECK(p_test->at<GConstrainedDoubleObject>(counter)->value() >= MINGCONSTRDOUBLE);
				BOOST_CHECK(p_test->at<GConstrainedDoubleObject>(counter)->value() <= MAXGCONSTRDOUBLE);
				counter++;
				BOOST_CHECK(p_test->at<GDoubleObject>(counter)->value() == p_test_0->at<GDoubleObject>(counter)->value() - FPSUBTRACT);
				counter++;
				boost::shared_ptr<GDoubleCollection> p_gdc;
				boost::shared_ptr<GDoubleCollection> p_gdc_0;
				BOOST_CHECK_NO_THROW(p_gdc   = p_test->at<GDoubleCollection>(counter));
				BOOST_CHECK_NO_THROW(p_gdc_0 = p_test_0->at<GDoubleCollection>(counter));
				for(std::size_t gdc_cnt=0; gdc_cnt<NGDOUBLECOLL; gdc_cnt++) {
					BOOST_CHECK_MESSAGE (
							p_gdc->at(gdc_cnt) == p_gdc_0->at(gdc_cnt) - FPSUBTRACT
							,  "\n"
							<< "p_gdc->at(gdc_cnt) = " << p_gdc->at(gdc_cnt) << "\n"
							<< "p_gdc_0->at(gdc_cnt) = " << p_gdc_0->at(gdc_cnt) << "\n"
							<< "FPSUBTRACT = " << FPSUBTRACT
							<< "p_gdc_0->at(gdc_cnt) - FPSUBTRACT = " << p_gdc_0->at(gdc_cnt) - FPSUBTRACT << "\n"
							<< "iteration = " << gdc_cnt << "\n"
					);
				}
				counter++;
			}

			// The int32 parameter should have stayed the same
			boost::shared_ptr<GConstrainedInt32Object> p_int32_0;
			boost::shared_ptr<GConstrainedInt32Object> p_int32;
			BOOST_CHECK_NO_THROW(p_int32_0   = p_test_0->at<GConstrainedInt32Object>(counter));
			BOOST_CHECK_NO_THROW(p_int32 =   p_test->at<GConstrainedInt32Object>(counter));
			BOOST_CHECK(*p_int32_0 == *p_int32);
			counter++;

			// Likewise, the boolean parameter should have stayed the same
			boost::shared_ptr<GBooleanObject> p_boolean_orig;
			boost::shared_ptr<GBooleanObject> p_boolean_cloned;
			BOOST_CHECK_NO_THROW(p_boolean_orig   = p_test_0->at<GBooleanObject>(counter));
			BOOST_CHECK_NO_THROW(p_boolean_cloned =   p_test->at<GBooleanObject>(counter));
			BOOST_CHECK(*p_boolean_orig == *p_boolean_cloned);
			counter++;
		}

		//-----------------------------------------------------------------
	}

	//---------------------------------------------------------------------

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GParameterSet::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GParameterSet::specificTestsFailuresExpected_GUnitTests() {
#ifdef GEM_TESTING
	// Call the parent class'es function
	GMutableSetT<Gem::Geneva::GParameterBase>::specificTestsFailuresExpected_GUnitTests();

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GParameterSet::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

