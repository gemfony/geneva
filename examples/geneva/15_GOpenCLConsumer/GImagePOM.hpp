/**
 * @file GImagePOM.hpp
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
#include <type_traits>
#include <future>
#include <atomic>

// Geneva headers go here
#include "geneva/GPluggableOptimizationMonitors.hpp"
#include "GImageIndividual.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * The default dimension of the canvas in x-direction
 */
const boost::uint16_t DEFAULTXDIMPROGRESS=166;

/******************************************************************************/
/**
 * The default dimension of the canvas in y-direction
 */
const boost::uint16_t DEFAULTYDIMPROGRESS=192;

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class saves the best image of each iteration to disk
 */
class GImagePOM
	: public GBasePluggableOM
{
	 ///////////////////////////////////////////////////////////////////////
	 friend class boost::serialization::access;

	 template<typename Archive>
	 void serialize(Archive & ar, const unsigned int){
		 using boost::serialization::make_nvp;

		 ar
		 & make_nvp("GBasePluggableOM",	boost::serialization::base_object<GBasePluggableOM>(*this))
		 & BOOST_SERIALIZATION_NVP(resultImageDirectory_)
		 & BOOST_SERIALIZATION_NVP(dimX_)
		 & BOOST_SERIALIZATION_NVP(dimY_)
		 & BOOST_SERIALIZATION_NVP(emitBestOnly_);
	 }
	 ///////////////////////////////////////////////////////////////////////

public:
	 /*
	  * The standard constructor.
	  *
	  * @param resultDirectory The directory to which result information should be written
	  * @param emitBestOnly Whether only the best individuals should be emitted
	  */
	 GImagePOM(
		 const std::string& resultDirectory
		 , const bool& emitBestOnly
	 )
		 : resultImageDirectory_(GImagePOM::trailingSlash(resultDirectory))
			, dimX_(DEFAULTXDIMPROGRESS)
			, dimY_(DEFAULTYDIMPROGRESS)
			, emitBestOnly_(emitBestOnly)
	 { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * The copy constructor
	  *
	  * @param cp A copy of another GImagePOM object
	  */
	 GImagePOM(const GImagePOM& cp)
		 : GBasePluggableOM(cp)
			, resultImageDirectory_(cp.resultImageDirectory_)
			, dimX_(cp.dimX_)
			, dimY_(cp.dimY_)
			, emitBestOnly_(cp.emitBestOnly_)
	 { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * The destructor
	  */
	 virtual ~GImagePOM()
	 { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * Searches for compliance with expectations with respect to another object
	  * of the same type
	  *
	  * @param cp A constant reference to another GObject object
	  * @param e The expected outcome of the comparison
	  * @param limit The maximum deviation for floating point values (important for similarity checks)
	  */
	 virtual void compare(
		 const GObject& cp
		 , const Gem::Common::expectation& e
		 , const double& limit
	 ) const override {
		 using namespace Gem::Common;

		 // Check that we are dealing with a GImagePOM reference independent of this object and convert the pointer
		 const GImagePOM *p_load = Gem::Common::g_convert_and_compare(cp, this);

		 GToken token("GImagePOM", e);

		 // Compare our parent data ...
		 Gem::Common::compare_base<GBasePluggableOM>(IDENTITY(*this, *p_load), token);

		 // ... and then our local data
		 compare_t(IDENTITY(resultImageDirectory_, p_load->resultImageDirectory_), token);
		 compare_t(IDENTITY(dimX_, p_load->dimX_), token);
		 compare_t(IDENTITY(dimY_, p_load->dimY_), token);
		 compare_t(IDENTITY(emitBestOnly_, p_load->emitBestOnly_), token);

		 // React on deviations from the expectation
		 token.evaluate();
	 }

	 /***************************************************************************/
	 /**
	  * Allows to set the image dimensions of the image written to disk
	  */
	 void setImageDimensions(
		 const std::tuple<std::size_t, std::size_t>& dim
		 , const std::size_t& factor
	 ) {
		 dimX_ = factor*std::get<0>(dim);
		 dimY_ = factor*std::get<1>(dim);
	 }

	 /***************************************************************************/
	 /**
	  * Returns the dimensions used to store result images
	  */
	 auto getImageDimensions() const {
		 return std::tuple<std::size_t, std::size_t>{dimX_, dimY_};
	 }

	 /***************************************************************************/
	 /**
	  * Allows to specify whether only images for improved iterations should be emitted
	  */
	 void setEmitBestOnly(const bool& emitBestOnly) {
		 emitBestOnly_ = emitBestOnly;
	 }

	 /***************************************************************************/
	 /**
	  * Allows to check whether only images for improved iterations should be emitted
	  */
	 bool getEmitBestOnly() const {
		 return emitBestOnly_;
	 }

	 /***************************************************************************/
	 /**
	  * Allows to emit information in different stages of the information cycle
	  * (initialization, during each cycle and during finalization)
	  */
	 virtual void informationFunction(
		 const infoMode& im
		 , G_OptimizationAlgorithm_Base * const goa
	 ) override {
		 switch(im) {
			 case Gem::Geneva::infoMode::INFOINIT:
			 {
				 // Check that the target directory for result files exists. If not, try to create it.
				 if(!boost::filesystem::exists(boost::filesystem::path(resultImageDirectory_))) {
					 if(!boost::filesystem::create_directory(boost::filesystem::path(resultImageDirectory_))) {
						 throw gemfony_exception(
							 g_error_streamer(DO_LOG,  time_and_place)
								 << "Error: could not create directory " << resultImageDirectory_ << std::endl
						 );
					 }
				 } else { // Check that resultImageDirectory_ is indeed a directory and not a file
					 if(!boost::filesystem::is_directory(boost::filesystem::path(resultImageDirectory_))) {
						 throw gemfony_exception(
							 g_error_streamer(DO_LOG,  time_and_place)
								 << "Error: " << resultImageDirectory_ << " is not a directory" << std::endl
						 );
					 }
				 }
			 }
				 break;

			 case Gem::Geneva::infoMode::INFOPROCESSING:
			 {
				 // Trigger output of a result picture
				 if(!emitBestOnly_ || (emitBestOnly_ && goa->progress())) {
					 // TODO: We assume that access to dimX_/dimY_ and resultImageDirectory_ is read-only
					 // (i.e. no entity writes to these quantities during an optimization run). Hence we
					 // do not currently protect these resouces. This should be changed.
					 std::async(
						 [&]() {
							 goa->G_Interface_OptimizerT::getBestIterationIndividual<GImageIndividual>()->writeImage(
								 "image"
								 , resultImageDirectory_
								 , std::tuple<std::size_t, std::size_t>(
									 dimX_
									 , dimY_
								 )
							 );
						 }
					 );
				 }
			 }
				 break;

			 case Gem::Geneva::infoMode::INFOEND:
				 // nothing
				 break;
		 };
	 }

protected:
	 /************************************************************************/
	 /**
	  * Loads the data of another object
	  *
	  * cp A pointer to another GCollectiveMonitorT<ind_type> object, camouflaged as a GObject
	  */
	 virtual void load_(const GObject* cp) override {
		 // Check that we are dealing with a GImagePOM reference independent of this object and convert the pointer
		 const GImagePOM *p_load = Gem::Common::g_convert_and_compare(cp, this);

		 // Load the parent classes' data ...
		 GBasePluggableOM::load_(cp);

		 // ... and then our local data
		 this->resultImageDirectory_ = p_load->resultImageDirectory_;
		 this->dimX_ = p_load->dimX_;
		 this->dimY_ = p_load->dimY_;
		 this->emitBestOnly_ = p_load->emitBestOnly_;
	 }

private:
	 /***************************************************************************/
	 /**
	  * Emits a name for this class / object
	  */
	 virtual std::string name_() const override {
		 return std::string("GImagePOM");
	 }

	 /************************************************************************/
	 /**
	  * Creates a deep clone of this object
	  */
	 virtual GObject* clone_() const override {
		 return new GImagePOM(*this);
	 }

	 /***************************************************************************/
	 /**
	  * The default constructor. It is intentionally private, as it is only needed
	  * for (de-)serialization purposes.
	  */
	 GImagePOM()
	 { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * Adds a slash to the end of the path if necessary
	  */
	 static std::string trailingSlash(const std::string& path) {
		 if(path[path.size() - 1] != '/') return path + '/';
		 else return path;
	 }

	 /***************************************************************************/
	 // Class data

	 std::string resultImageDirectory_ = "./results/"; ///< The target directory for results
	 std::size_t dimX_ = DEFAULTXDIMPROGRESS;
	 std::size_t dimY_ = DEFAULTYDIMPROGRESS; ///< The dimensions of the candidate image written to disk
	 bool emitBestOnly_ = true; ///< Indicates whether images should only be written for improved iterations

public:
	 /***************************************************************************/
	 /**
	  * Applies modifications to this object. This is needed for testing purposes
	  *
	  * @return A boolean which indicates whether modifications were made
	  */
	 virtual bool modify_GUnitTests() override {
#ifdef GEM_TESTING
		 using boost::unit_test_framework::test_suite;
		 using boost::unit_test_framework::test_case;

		 bool result = false;

		 // Call the parent classes' functions
		 if(GBasePluggableOM::modify_GUnitTests()) {
			 result = true;
		 }

		 // no local data -- nothing to change

		 return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		 condnotset("GImagePOM::modify_GUnitTests", "GEM_TESTING");
		return false;
#endif /* GEM_TESTING */
	 }

	 /***************************************************************************/
	 /**
	  * Performs self tests that are expected to succeed. This is needed for testing purposes
	  */
	 virtual void specificTestsNoFailureExpected_GUnitTests() override {
#ifdef GEM_TESTING
		 using boost::unit_test_framework::test_suite;
		 using boost::unit_test_framework::test_case;

		 // Call the parent classes' functions
		 GBasePluggableOM::specificTestsNoFailureExpected_GUnitTests();
#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		 condnotset("GImagePOM::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
	 }

	 /***************************************************************************/
	 /**
	  * Performs self tests that are expected to fail. This is needed for testing purposes
	  */
	 virtual void specificTestsFailuresExpected_GUnitTests() override {
#ifdef GEM_TESTING
		 using boost::unit_test_framework::test_suite;
		 using boost::unit_test_framework::test_case;

		 // Call the parent classes' functions
		 GBasePluggableOM::specificTestsFailuresExpected_GUnitTests();

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		 condnotset("GImagePOM::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
	 }
	 /***************************************************************************/
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

