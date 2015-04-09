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
	, perItemCrossOverProbability_(DEFAULTPERITEMEXCHANGELIKELIHOOD)
{ /* nothing */ }

/******************************************************************************/
/**
 * Initialization with the number of fitness criteria
 */
GParameterSet::GParameterSet(const std::size_t& nFitnessCriteria)
   : GMutableSetT<Gem::Geneva::GParameterBase>(nFitnessCriteria)
   , Gem::Courtier::GSubmissionContainerT<GParameterSet>()
   , perItemCrossOverProbability_(DEFAULTPERITEMEXCHANGELIKELIHOOD)
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
	, perItemCrossOverProbability_(cp.perItemCrossOverProbability_)
{ /* nothing */ }

/******************************************************************************/
/**
 * The destructor
 */
GParameterSet::~GParameterSet()
{ /* nothing */ }

/***************************************************************************/
/**
 * The standard assignment operator
 */
const GParameterSet& GParameterSet::operator=(const GParameterSet& cp) {
   this->load_(&cp);
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
   try {
      this->compare(cp, CE_EQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
      return true;
   } catch(g_expectation_violation&) {
      return false;
   }
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
   try {
      this->compare(cp, CE_INEQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
      return true;
   } catch(g_expectation_violation&) {
      return false;
   }
}

/******************************************************************************/
/**
 * Searches for compliance with expectations with respect to another object
 * of the same type
 *
 * @param cp A constant reference to another GObject object
 * @param e The expected outcome of the comparison
 * @param limit The maximum deviation for floating point values (important for similarity checks)
 */
void GParameterSet::compare(
   const GObject& cp
   , const Gem::Common::expectation& e
   , const double& limit
) const {
   using namespace Gem::Common;

   // Check that we are indeed dealing with a GBaseEA reference
   const GParameterSet *p_load = GObject::gobject_conversion<GParameterSet>(&cp);

   try {
      BEGIN_COMPARE;

      // Check our parent class'es data ...
      COMPARE_PARENT(GMutableSetT<Gem::Geneva::GParameterBase>, cp, e, limit);

      // ... and then our local data
      COMPARE(perItemCrossOverProbability_, p_load->perItemCrossOverProbability_, e, limit);

      END_COMPARE;

   } catch(g_expectation_violation& g) { // Create a suitable stack-trace
      throw g("g_expectation_violation caught by GParameterSet");
   }
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
 * Checks whether this object is better than a given set of evaluations. This
 * function compares "real" boundaries with evaluations, hence we use "raw"
 * measurements here instead of transformed measurements.
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
         if(this->fitness(i, PREVENTREEVALUATION, USERAWFITNESS) < boundaries.at(i)) {
            return false;
         }
      }
   } else { // Minimization
      for(std::size_t i=0; i<boundaries.size(); i++) {
         if(this->fitness(i, PREVENTREEVALUATION, USERAWFITNESS) > boundaries.at(i)) {
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
boost::shared_ptr<GParameterSet> GParameterSet::amalgamate(const boost::shared_ptr<GParameterSet> cp) const {
   // Create a copy of this object
   boost::shared_ptr<GParameterSet> this_cp = this->GObject::clone<GParameterSet>();

   this_cp->perItemCrossOver(*cp, perItemCrossOverProbability_);

   return this_cp;
}

/******************************************************************************/
/**
 * Allows to set the "per item" cross-over probability
 */
void GParameterSet::setPerItemCrossOverProbability(double perItemCrossOverProbability) {
   if(perItemCrossOverProbability < 0. || perItemCrossOverProbability > 1.) {
      glogger
      << "In GParameterSet::setPerItemCrossOverProbability(" << perItemCrossOverProbability << "): Error!" << std::endl
      << "Variable outside of allowed ranged [0:1]" << std::endl
      << GEXCEPTION;
   }

   perItemCrossOverProbability_ = perItemCrossOverProbability;
}

/******************************************************************************/
/**
 * Allows to retrieve the "per item" cross-over probability
 */
double GParameterSet::getPerItemCrossOverProbability() const {
   return perItemCrossOverProbability_;
}

/******************************************************************************/
/**
 * Triggers updates of adaptors contained in this object.
 */
void GParameterSet::updateAdaptorsOnStall(const boost::uint32_t& nStalls) {
   GParameterSet::iterator it;
   for(it=this->begin(); it!=this->end(); ++it) {
      (*it)->updateAdaptorsOnStall(nStalls);
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
   const std::string& adaptorName
   , const std::string& property
   , std::vector<boost::any>& data
) const {
   GParameterSet::const_iterator cit;
   for(cit=this->begin(); cit!=this->end(); ++cit) {
      (*cit)->queryAdaptor(adaptorName, property, data);
   }
}

/******************************************************************************/
/**
 * Loads the data of another GParameterSet object, camouflaged as a GObject.
 *
 * @param cp A copy of another GParameterSet object, camouflaged as a GObject
 */
void GParameterSet::load_(const GObject* cp) {
	// Convert to local format
	const GParameterSet *p_load = this->gobject_conversion<GParameterSet>(cp);

	// Load the parent class'es data
	GMutableSetT<Gem::Geneva::GParameterBase>::load_(cp);

	// and then our local data
	perItemCrossOverProbability_ = p_load->perItemCrossOverProbability_;
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
bool GParameterSet::randomInit(const activityMode& am) {
	// Trigger random initialization of all our parameter objects
	GParameterSet::iterator it;
	for(it=this->begin(); it!=this->end(); ++it) {
	   (*it)->randomInit(am);
	}

	// As we have modified our internal data sets, make sure the dirty flag is set
	GOptimizableEntity::setDirtyFlag();

	return true;
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
 * these. The whole procedure happens on an "per item" basis, i.e., each item is swapped with the
 * corresponding "foreign" item with a given likelihood. The procedure requires both objects to have
 * the same "architecture" and will throw, if this is not the case.
 */
void GParameterSet::perItemCrossOver(const GParameterSet& cp, const double& likelihood) {
#ifdef DEBUG
	// Do some error checking
	if(likelihood < 0. || likelihood > 1.) {
	   glogger
	   << "In GParameterSet::crossOver(): Error!" << std::endl
	   << "Received invalid likelihood: " << likelihood << std::endl
	   << GEXCEPTION;
	}
#endif /* DEBUG */

	// Extract all data items
	std::vector<double>         this_double_vec, cp_double_vec;
	std::vector<float>          this_float_vec , cp_float_vec;
	std::vector<bool>           this_bool_vec  , cp_bool_vec;
	std::vector<boost::int32_t> this_int_vec   , cp_int_vec;

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
      glogger
      << "In GParameterSet::perItemCrossOver(): Error!" << std::endl
      << "Got invalid sizes (double): " << this_double_vec.size() << " / " << cp_double_vec.size() << std::endl
      << GEXCEPTION;
    }
   if(this_float_vec.size() != cp_float_vec.size()) {
      glogger
      << "In GParameterSet::perItemCrossOver(): Error!" << std::endl
      << "Got invalid sizes (float): " << this_float_vec.size() << " / " << cp_float_vec.size() << std::endl
      << GEXCEPTION;
    }
   if(this_bool_vec.size() != cp_bool_vec.size()) {
      glogger
      << "In GParameterSet::perItemCrossOver(): Error!" << std::endl
      << "Got invalid sizes (bool): " << this_bool_vec.size() << " / " << cp_bool_vec.size() << std::endl
      << GEXCEPTION;
    }
   if(this_int_vec.size() != cp_int_vec.size()) {
      glogger
      << "In GParameterSet::perItemCrossOver(): Error!" << std::endl
      << "Got invalid sizes (boost::int32_t): " << this_int_vec.size() << " / " << cp_int_vec.size() << std::endl
      << GEXCEPTION;
    }
#endif /* DEBUG */

   // Do the actual cross-over
   if(!this_double_vec.empty()) {
      // Calculate a suitable position for the cross-over
      std::size_t pos = gr.uniform_int(std::size_t(0), this_double_vec.size() - std::size_t(1));

      // Perform the actual cross-over operation
      for(std::size_t i=pos; i<this_double_vec.size(); i++) {
         this_double_vec[i] = cp_double_vec[i];
      }
   }

   if(!this_float_vec.empty()) {
      // Calculate a suitable position for the cross-over
      std::size_t pos = gr.uniform_int(std::size_t(0), this_float_vec.size() - std::size_t(1));

      // Perform the actual cross-over operation
      for(std::size_t i=pos; i<this_float_vec.size(); i++) {
         this_float_vec[i] = cp_float_vec[i];
      }
   }

   if(!this_bool_vec.empty()) {
      // Calculate a suitable position for the cross-over
      std::size_t pos = gr.uniform_int(std::size_t(0), this_bool_vec.size() - std::size_t(1));

      // Perform the actual cross-over operation
      for(std::size_t i=pos; i<this_bool_vec.size(); i++) {
         this_bool_vec[i] = cp_bool_vec[i];
      }
   }

   if(!this_int_vec.empty()) {
      // Calculate a suitable position for the cross-over
      std::size_t pos = gr.uniform_int(std::size_t(0), this_int_vec.size() - std::size_t(1));

      // Perform the actual cross-over operation
      for(std::size_t i=pos; i<this_int_vec.size(); i++) {
         this_int_vec[i] = cp_int_vec[i];
      }
   }

   // Load the data vectors back into this object
   this->assignValueVector(this_double_vec);
   this->assignValueVector(this_float_vec);
   this->assignValueVector(this_bool_vec);
   this->assignValueVector(this_int_vec);

   // Makr this individual as "dirty"
   this->setDirtyFlag(true);
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
 * @return A boolean which indicates whether processing was done
 */
bool GParameterSet::process(){
   this->fitness(0, Gem::Geneva::ALLOWREEVALUATION, Gem::Geneva::USETRANSFORMEDFITNESS);

   // Let the audience know that we were successful
   return true;
}

/******************************************************************************/
/**
 * Prevent shadowing of std::vector<GParameterBase>::at()
 *
 * @param pos The position of the item we aim to retrieve from the std::vector<GParameterBase>
 * @return The item we aim to retrieve from the std::vector<GParameterBase>
 */
GMutableSetT<Gem::Geneva::GParameterBase>::reference GParameterSet::at(const std::size_t& pos) {
	return GMutableSetT<Gem::Geneva::GParameterBase>::at(pos);
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
std::size_t GParameterSet::customAdaptions() {
   std::size_t nAdaptions = 0;

	GParameterSet::iterator it;
	for(it=this->begin(); it!=this->end(); ++it) {
	   nAdaptions += (*it)->adapt();
	}

	return nAdaptions;
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
 */
void GParameterSet::addConfigurationOptions (
	Gem::Common::GParserBuilder& gpb
) {
	// Call our parent class'es function
	GMutableSetT<Gem::Geneva::GParameterBase>::addConfigurationOptions(gpb);

	// Add local data
	gpb.registerFileParameter<bool>(
		"maximize" // The name of the variable
		, false // The default value
		, boost::bind(
			&GParameterSet::setMaxMode
			, this
			, _1
		  )
	)
	<< "Specifies whether the individual should be maximized (1) or minimized (0)" << std::endl
	<< "Note that minimization is the by far most common option.";

   gpb.registerFileParameter<double>(
      "perItemCrossOverProbability" // The name of the variable
      , DEFAULTPERITEMEXCHANGELIKELIHOOD // The default value
      , boost::bind(
         &GParameterSet::setPerItemCrossOverProbability
         , this
         , _1
        )
   )
   << "The likelihood for two data items to be exchanged";
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

   bool dirtyFlag = this->isDirty();
   double rawFitness = 0., transformedFitness = 0.;

   ptr.put(baseName + ".iteration", this->getAssignedIteration());
   ptr.put(baseName + ".id"       , this->getCurrentEvaluationID());
   ptr.put(baseName + ".isDirty"  , dirtyFlag);
   ptr.put(baseName + ".isValid"  , dirtyFlag?false:this->isValid());
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
      ptr.put(baseName + ".transformationPolicy", "USESIMPLEEVALUATION");
      break;

      case USESIGMOID:
      ptr.put(baseName + ".transformationPolicy", "USESIGMOID");
      break;

      case USEWORSTKNOWNVALIDFORINVALID:
      ptr.put(baseName + ".transformationPolicy", "USEWORSTKNOWNVALIDFORINVALID");
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

   // Output all fitness criteria. We do not enforce re-calculation of the fitness here,
   // as the property is meant to capture the current state of the individual.
   // Check the "isDirty" tag, if you need to know whether the results are current.
   ptr.put(baseName + ".nResults", this->getNumberOfFitnessCriteria());
   for(std::size_t f=0; f<this->getNumberOfFitnessCriteria(); f++) {
      rawFitness         = dirtyFlag?this->getWorstCase():this->fitness(f, PREVENTREEVALUATION, USERAWFITNESS);
      transformedFitness = dirtyFlag?this->getWorstCase():this->transformedFitness(f);

      base = baseName + ".results.result" + boost::lexical_cast<std::string>(f);
      ptr.put(base, transformedFitness);
      base = baseName + ".results.rawResult"  + boost::lexical_cast<std::string>(f);
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
std::string GParameterSet::toCSV(bool withNameAndType, bool withCommas, bool useRawFitness, bool showValidity) const {
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

   // Note: The following will throw if this individual is in a "dirty" state
   for(std::size_t f=0; f<this->getNumberOfFitnessCriteria(); f++) {
      if(withNameAndType) {
         varNames.push_back(std::string("Fitness_") + boost::lexical_cast<std::string>(f));
         varTypes.push_back("double");
      }
      if(useRawFitness) {
         varValues.push_back(boost::lexical_cast<std::string>(this->fitness(f, PREVENTREEVALUATION, USERAWFITNESS)));
      } else { // Output potentially transformed fitness
         varValues.push_back(boost::lexical_cast<std::string>(this->transformedFitness(f)));
      }
   }

   if(showValidity) {
      if(withNameAndType) {
         varNames.push_back(std::string("validity"));
         varTypes.push_back("bool");
      }

      varValues.push_back(boost::lexical_cast<std::string>(this->isValid()));
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

		{ // Test random initialization
			// Create a GParameterSet object as a clone of p_test_0 for further usage
			boost::shared_ptr<GParameterSet> p_test = p_test_0->clone<GParameterSet>();

			BOOST_CHECK_NO_THROW(p_test->randomInit(ALLPARAMETERS));
			BOOST_CHECK(*p_test != *p_test_0);
		}

		//-----------------------------------------------------------------
		{ // Test initialization of all fp parameters with a fixed value
			for(double d=FPFIXEDVALINITMIN; d<FPFIXEDVALINITMAX; d+=1.) {
				// Create a GParameterSet object as a clone of p_test_0 for further usage
				boost::shared_ptr<GParameterSet> p_test = p_test_0->clone<GParameterSet>();

				// Initialize all fp-values with 0.
				p_test->fixedValueInit<double>(d, ALLPARAMETERS);

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
				BOOST_CHECK_NO_THROW(p_test->fixedValueInit<double>(FPFIXEDVALINITMAX, ALLPARAMETERS));

				// Multiply this fixed value by d
				BOOST_CHECK_NO_THROW(p_test->multiplyBy<double>(d, ALLPARAMETERS));

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
			BOOST_CHECK_NO_THROW(p_test->multiplyByRandom<double>(FPMULTIPLYBYRANDMIN, FPMULTIPLYBYRANDMAX, ALLPARAMETERS));

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
			BOOST_CHECK_NO_THROW(p_test->multiplyByRandom<double>(ALLPARAMETERS));

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

			// Initialize all fp-values of the "add" individual with a fixed value
			BOOST_CHECK_NO_THROW(p_test_fixed->fixedValueInit<double>(FPADD, ALLPARAMETERS));

			// Add p_test_fixed to p_test
			BOOST_CHECK_NO_THROW(p_test->add<double>(p_test_fixed, ALLPARAMETERS));

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
			BOOST_CHECK_NO_THROW(p_test_fixed->fixedValueInit<double>(FPSUBTRACT, ALLPARAMETERS));

			// Add p_test_fixed to p_test
			BOOST_CHECK_NO_THROW(p_test->subtract<double>(p_test_fixed, ALLPARAMETERS));

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

	{ // Check counting of active and inactive parameters
      // Some settings for the collection of tests below
      const double MINGCONSTRDOUBLE    =  -4.;
      const double MAXGCONSTRDOUBLE    =   4.;
      const double MINGDOUBLE          =  -5.;
      const double MAXGDOUBLE          =   5.;
      const double MINGDOUBLECOLL      =  -3.;
      const double MAXGDOUBLECOLL      =   3.;
      const std::size_t NGDOUBLECOLL   =  10 ;
      const std::size_t NINTCOLL       =  10 ;
      const std::size_t NINTBOOLOBJ    =  10 ;
      const boost::int32_t MINGINT      = -100;
      const boost::int32_t MAXGINT      =  100;
      const std::size_t FPLOOPCOUNT    =   5 ;
      const double FPFIXEDVALINITMIN   =  -3.;
      const double FPFIXEDVALINITMAX   =   3.;
      const double FPMULTIPLYBYRANDMIN =  -5.;
      const double FPMULTIPLYBYRANDMAX =   5.;
      const double FPADD               =   2.;
      const double FPSUBTRACT          =   2.;

      // Create a GParameterSet object as a clone of this object for further usage
      boost::shared_ptr<GParameterSet> p_test_0 = this->clone<GParameterSet>();
      // Clear the collection
      p_test_0->clear();
      // Make sure it is really empty
      BOOST_CHECK(p_test_0->empty());

      // Add some floating point parameters
      for(std::size_t i=0; i<FPLOOPCOUNT; i++) {
         boost::shared_ptr<GConstrainedDoubleObject> gcdo_ptr = boost::shared_ptr<GConstrainedDoubleObject>(new GConstrainedDoubleObject(gr.uniform_real<double>(MINGCONSTRDOUBLE, MAXGCONSTRDOUBLE), MINGCONSTRDOUBLE, MAXGCONSTRDOUBLE));
         boost::shared_ptr<GDoubleObject> gdo_ptr = boost::shared_ptr<GDoubleObject>(new GDoubleObject(gr.uniform_real<double>(MINGDOUBLE,MAXGDOUBLE)));
         boost::shared_ptr<GDoubleCollection> gdc_ptr = boost::shared_ptr<GDoubleCollection>(new GDoubleCollection(NGDOUBLECOLL, MINGDOUBLECOLL, MAXGDOUBLECOLL));

         // Mark the last parameter type as inactive
         gdc_ptr->setAdaptionsInactive();

         // Add the parameter objects to the parameter set
         p_test_0->push_back(gcdo_ptr);
         p_test_0->push_back(gdo_ptr);
         p_test_0->push_back(gdc_ptr);
      }

      // Attach a few other parameter types
      for(std::size_t i=0; i<NINTBOOLOBJ; i++) {
         p_test_0->push_back(boost::shared_ptr<GConstrainedInt32Object>(new GConstrainedInt32Object(7, MINGINT, MAXGINT)));
         p_test_0->push_back(boost::shared_ptr<GBooleanObject>(new GBooleanObject(true)));
      }

      // Finally we add a tree structure
      boost::shared_ptr<GParameterObjectCollection> poc_ptr =
            boost::shared_ptr<GParameterObjectCollection>(new GParameterObjectCollection());

      for(std::size_t i=0; i<FPLOOPCOUNT; i++) {
         boost::shared_ptr<GConstrainedDoubleObject> gcdo_ptr = boost::shared_ptr<GConstrainedDoubleObject>(new GConstrainedDoubleObject(gr.uniform_real<double>(MINGCONSTRDOUBLE, MAXGCONSTRDOUBLE), MINGCONSTRDOUBLE, MAXGCONSTRDOUBLE));
         boost::shared_ptr<GDoubleObject> gdo_ptr = boost::shared_ptr<GDoubleObject>(new GDoubleObject(gr.uniform_real<double>(MINGDOUBLE,MAXGDOUBLE)));
         boost::shared_ptr<GConstrainedInt32ObjectCollection> gcioc_ptr
            = boost::shared_ptr<GConstrainedInt32ObjectCollection>(
                  new GConstrainedInt32ObjectCollection()
            );

         boost::shared_ptr<GParameterObjectCollection> sub_poc_ptr =
               boost::shared_ptr<GParameterObjectCollection>(new GParameterObjectCollection());

         for(std::size_t ip=0; ip<NINTCOLL; ip++) {
            boost::shared_ptr<GConstrainedInt32Object> gci32o_ptr
               = boost::shared_ptr<GConstrainedInt32Object>(new GConstrainedInt32Object(MINGINT, MAXGINT));
            gci32o_ptr->setAdaptionsInactive(); // The parameter should not be modifiable now

            sub_poc_ptr->push_back(gci32o_ptr);
         }

         boost::shared_ptr<GDoubleObject> gdo2_ptr
            = boost::shared_ptr<GDoubleObject>(new GDoubleObject(gr.uniform_real<double>(MINGDOUBLE,MAXGDOUBLE)));
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
      std::size_t NDOUBLEACTIVE = 2*FPLOOPCOUNT + 2*FPLOOPCOUNT;
      std::size_t NDOUBLEINACTIVE = NGDOUBLECOLL*FPLOOPCOUNT + FPLOOPCOUNT;
      std::size_t NDOUBLEALL = NDOUBLEINACTIVE + NDOUBLEACTIVE;
      std::size_t NINTACTIVE = NINTBOOLOBJ;
      std::size_t NINTINACTIVE = NINTCOLL*FPLOOPCOUNT;
      std::size_t NINTALL = NINTINACTIVE + NINTACTIVE;
      std::size_t NBOOLACTIVE = NINTBOOLOBJ;
      std::size_t NBOOLINACTIVE = 0;
      std::size_t NBOOLALL = NBOOLINACTIVE + NBOOLACTIVE;

      //-----------------------------------------------------------------

      { // Test counting of parameters
         // Create a GParameterSet object as a clone of p_test_0 for further usage
         boost::shared_ptr<GParameterSet> p_test = p_test_0->clone<GParameterSet>();

         // Count the number of parameters and compare with the expected number
         BOOST_CHECK(p_test->countParameters<double>(ACTIVEONLY) == NDOUBLEACTIVE);
         BOOST_CHECK(p_test->countParameters<double>(INACTIVEONLY) == NDOUBLEINACTIVE);
         BOOST_CHECK(p_test->countParameters<double>(ALLPARAMETERS) == NDOUBLEALL);
         BOOST_CHECK(p_test->countParameters<boost::int32_t>(ACTIVEONLY) == NINTACTIVE);
         BOOST_CHECK(p_test->countParameters<boost::int32_t>(INACTIVEONLY) == NINTINACTIVE);
         BOOST_CHECK(p_test->countParameters<boost::int32_t>(ALLPARAMETERS) == NINTALL);
         BOOST_CHECK(p_test->countParameters<bool>(ACTIVEONLY) == NBOOLACTIVE);
         BOOST_CHECK(p_test->countParameters<bool>(INACTIVEONLY) == NBOOLINACTIVE);
         BOOST_CHECK(p_test->countParameters<bool>(ALLPARAMETERS) == NBOOLALL);
      }

      //-----------------------------------------------------------------

      { // Check that streamline(INACTIVEONLY) yields unchanged results before and after randomInit(ACTIVEONLY)
         // Create two GParameterSet objects as clones of p_test_0 for further usage
         boost::shared_ptr<GParameterSet> p_test_orig = p_test_0->clone<GParameterSet>();
         boost::shared_ptr<GParameterSet> p_test_rand = p_test_0->clone<GParameterSet>();

         // Randomly initialize active components of p_test2
         BOOST_CHECK_NO_THROW(p_test_rand->randomInit(ACTIVEONLY));

         std::vector<double> orig_d_inactive;
         std::vector<double> rand_d_inactive;

         std::vector<boost::int32_t> orig_i_inactive;
         std::vector<boost::int32_t> rand_i_inactive;

         std::vector<bool> orig_b_inactive;
         std::vector<bool> rand_b_inactive;

         // Extract the parameters
         BOOST_CHECK_NO_THROW(p_test_orig->streamline<double>(orig_d_inactive, INACTIVEONLY));
         BOOST_CHECK_NO_THROW(p_test_rand->streamline<double>(rand_d_inactive, INACTIVEONLY));
         BOOST_CHECK_NO_THROW(p_test_orig->streamline<boost::int32_t>(orig_i_inactive, INACTIVEONLY));
         BOOST_CHECK_NO_THROW(p_test_rand->streamline<boost::int32_t>(rand_i_inactive, INACTIVEONLY));
         BOOST_CHECK_NO_THROW(p_test_orig->streamline<bool>(orig_b_inactive, INACTIVEONLY));
         BOOST_CHECK_NO_THROW(p_test_rand->streamline<bool>(rand_b_inactive, INACTIVEONLY));

         // Check that the "inactive" vectors have the expected characteristics
         BOOST_CHECK(orig_d_inactive.size() == NDOUBLEINACTIVE);
         BOOST_CHECK(orig_d_inactive == rand_d_inactive);
         BOOST_CHECK(orig_i_inactive.size() == NINTINACTIVE);
         BOOST_CHECK(orig_i_inactive == rand_i_inactive);
         BOOST_CHECK(orig_b_inactive.size() == NBOOLINACTIVE);
         BOOST_CHECK(orig_b_inactive == rand_b_inactive);
      }

      //-----------------------------------------------------------------

      { // Check that streamline(ACTIVEONLY) yields changed results after randomInit(ACTIVEONLY)
         // Create a GParameterSet object as a clone of p_test_0 for further usage
         // Create two GParameterSet objects as clones of p_test_0 for further usage
         boost::shared_ptr<GParameterSet> p_test_orig = p_test_0->clone<GParameterSet>();
         boost::shared_ptr<GParameterSet> p_test_rand = p_test_0->clone<GParameterSet>();

         // Randomly initialize active components of p_test2
         BOOST_CHECK_NO_THROW(p_test_rand->randomInit(ACTIVEONLY));

         std::vector<double> orig_d_active;
         std::vector<double> rand_d_active;

         std::vector<boost::int32_t> orig_i_active;
         std::vector<boost::int32_t> rand_i_active;

         std::vector<bool> orig_b_active;
         std::vector<bool> rand_b_active;

         // Extract the parameters
         BOOST_CHECK_NO_THROW(p_test_orig->streamline<double>(orig_d_active, ACTIVEONLY));
         BOOST_CHECK_NO_THROW(p_test_rand->streamline<double>(rand_d_active, ACTIVEONLY));
         BOOST_CHECK_NO_THROW(p_test_orig->streamline<boost::int32_t>(orig_i_active, ACTIVEONLY));
         BOOST_CHECK_NO_THROW(p_test_rand->streamline<boost::int32_t>(rand_i_active, ACTIVEONLY));
         BOOST_CHECK_NO_THROW(p_test_orig->streamline<bool>(orig_b_active, ACTIVEONLY));
         BOOST_CHECK_NO_THROW(p_test_rand->streamline<bool>(rand_b_active, ACTIVEONLY));

         // Check that the "active" vectors' contents indeed differ
         BOOST_CHECK(orig_d_active.size() == NDOUBLEACTIVE);
         BOOST_CHECK(rand_d_active.size() == NDOUBLEACTIVE);
         BOOST_CHECK(orig_d_active != rand_d_active);
         BOOST_CHECK(orig_i_active.size() == NINTACTIVE);
         BOOST_CHECK(rand_i_active.size() == NINTACTIVE);
         BOOST_CHECK_MESSAGE(
               orig_i_active != rand_i_active
               , "orig_i_active: " << Gem::Common::vecToString(orig_i_active) << "\n" << "rand_i_active: " << Gem::Common::vecToString(rand_i_active) << "\n"
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

