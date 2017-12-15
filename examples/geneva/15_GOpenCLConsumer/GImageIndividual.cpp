/**
 * @file GImageIndividual.cpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) com
 *
 * This file is part of the Geneva/P library collection. No part of
 * this code may be distributed without prior, written consent by
 * Gemfony scientific.
 */

#include "GImageIndividual.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GImageIndividual)

namespace Gem {
namespace Geneva {

/******************************************************************************/
//////////////////////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * The default constructor. All real work is done in the init() function
 */
GImageIndividual::GImageIndividual()
	: Gem::Geneva::GParameterSet()
	  , nTriangles_(GII_DEF_NTRIANGLES)
	  , alphaSort_(GII_DEF_ALPHASORT)
{ /* nothing */ }

/******************************************************************************/
/**
 * The copy constructor
 */
GImageIndividual::GImageIndividual(const GImageIndividual& cp)
	: Gem::Geneva::GParameterSet(cp)
	  , nTriangles_(cp.nTriangles_)
	  , alphaSort_(cp.alphaSort_)
{
	// All copying of GConstrainedDoubleObjectCollection objects is
	// done by the GParameterSet
}

/******************************************************************************/
/**
 * The standard destructor
 */
GImageIndividual::~GImageIndividual()
{ /* nothing */ }

/******************************************************************************/
/**
 * Fills the individual with parameters. Our parameter set consists of nTriangles GParameterOjectCollection
 * objects. Each holds 10 parameters:
 * - a GConstrainedDoubleCollection holding the middle of a circle
 * - a single GConstrainedDoubleObject for the radius
 * - a GConstrainedDoubleCollection holding three angles which point to the corners of the triangle (on the circles edge).
 * - a GConstrainedDoubleCollection for the three colors
 * - a single GConstrainedDoubleObject for the alpha channel
 *
 * @param startSize The initial size of triangles
 * @param minSize The minimum size of triangles
 * @param maxSize The maximum size of triangles
 * @param sigma The step-width used for Gauss-Adaptions
 * @param sigmaSigma Indicates the level of adaption of sigma
 * @param minSigma The minimum allowed value for sigma
 * @param maxSigma The maximum allowed value for sigma
 * @param minOpaqueness The minimum opaqueness allowed for objects
 * @param maxOpaqueness The maximum opaqueness allowed for objects
 * @param adProb Specifies the adaption probability for adaptors
 * @param nTriangles The number of triangles constituting a candidate image
 * @param alphaSort Whether triangles should be sorted according to their alpha channel
 */
void GImageIndividual::init(
	const std::size_t& nTriangles
	, const double& bgRed
	, const double& bgGreen
	, const double& bgBlue
	, const double& startSize
	, const double& minSize
	, const double& maxSize
	, const double& minOpaqueness
	, const double& maxOpaqueness
	, const bool&   alphaSort
	, const double& sigma
	, const double& sigmaSigma
	, const double& minSigma
	, const double& maxSigma
	, const double& adProb
	, const double& adaptAdProb
	, const double& minAdProb
	, const double& maxAdProb
	, const double& loc_sigma
	, const double& loc_sigmaSigma
	, const double& loc_minSigma
	, const double& loc_maxSigma
	, const double& loc_adProb
	, const double& loc_adaptAdProb
	, const double& loc_minAdProb
	, const double& loc_maxAdProb
) {
	if(startSize>=0. && startSize < minSize) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GImageIndividual::init() : Error!" << std::endl
				<< "Invalid values for minSize and startSize provided: " << minSize << " / " << startSize << std::endl
		);
	}

	if(startSize>=0. && startSize > maxSize) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GImageIndividual::init() : Error!" << std::endl
				<< "Invalid values for maxSize and startSize provided: " << maxSize << " / " << startSize << std::endl
		);
	}

	if(minSize >= maxSize || minSize < 0. || maxSize > 1.) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GImageIndividual::init() : Error!" << std::endl
				<< "Invalid values for minSize and maxSize provided: " << minSize << " / " << maxSize << std::endl
		);
	}

	if(adaptAdProb < 0. || adaptAdProb > 1.) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GImageIndividual::init() : Error!" << std::endl
				<< "Invalid value for adaptAdProb provided: " << adaptAdProb << std::endl
		);
	}

	if(loc_adaptAdProb < 0. || loc_adaptAdProb > 1.) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GImageIndividual::init() : Error!" << std::endl
				<< "Invalid value for loc_adaptAdProb provided: " << loc_adaptAdProb << std::endl
		);
	}

	if(minAdProb >= maxAdProb || minAdProb < 0 || maxAdProb > 1 || adProb < minAdProb || adProb > maxAdProb) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GImageIndividual::init() : Error!" << std::endl
				<< "Invalid values for minAdprob, maxAdProb or adProb provided: " << minAdProb << " / " << maxAdProb << " / " << adProb << std::endl
		);
	}

	if(loc_minAdProb >= loc_maxAdProb || loc_minAdProb < 0 || loc_maxAdProb > 1 || loc_adProb < loc_minAdProb || loc_adProb > loc_maxAdProb) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GImageIndividual::init() : Error!" << std::endl
				<< "Invalid values for loc_minAdprob, loc_maxAdProb or loc_adProb provided: " << loc_minAdProb << " / " << loc_maxAdProb << " / " << loc_adProb << std::endl
		);
	}

	nTriangles_ = nTriangles;
	alphaSort_ = alphaSort;

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Create suitable adaptors

	// Gaussian distributed random numbers
	std::shared_ptr<GDoubleGaussAdaptor> gdga_ptr_tmpl(
		new GDoubleGaussAdaptor(
			sigma
			, sigmaSigma
			, minSigma
			, maxSigma
		)
	);
	gdga_ptr_tmpl->setAdaptionProbability(adProb);
	gdga_ptr_tmpl->setAdaptAdProb(adaptAdProb);
	gdga_ptr_tmpl->setAdProbRange(minAdProb, maxAdProb);

	// Gaussian distributed random numbers for location parameters
	std::shared_ptr<GDoubleGaussAdaptor> loc_gdga_ptr_tmpl(
		new GDoubleGaussAdaptor(
			loc_sigma
			, loc_sigmaSigma
			, loc_minSigma
			, loc_maxSigma
		)
	);
	loc_gdga_ptr_tmpl->setAdaptionProbability(loc_adProb);
	loc_gdga_ptr_tmpl->setAdaptAdProb(loc_adaptAdProb);
	loc_gdga_ptr_tmpl->setAdProbRange(loc_minAdProb, loc_maxAdProb);

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Set up a hierarchical data structure holding the triangle information (compare the description of this function)

	// Create one GParameterObjectCollection for each triangle
	for(std::size_t t_cnt=0; t_cnt<nTriangles_; t_cnt++) {
		//--------------------------------------------------------------------------------------------
		// Add objects for the middle-x and -y
		std::shared_ptr<GConstrainedDoubleObject> middle_x_ptr(new GConstrainedDoubleObject(0., 1.));
		std::shared_ptr<GConstrainedDoubleObject> middle_y_ptr(new GConstrainedDoubleObject(0., 1.));
		// ... and equip them with an adaptor. This will clone the adaptor ...
		middle_x_ptr->addAdaptor(loc_gdga_ptr_tmpl);
		middle_y_ptr->addAdaptor(loc_gdga_ptr_tmpl);
		// ... finally add them to the GParameterObjectCollection representing the triangle
		this->push_back(middle_x_ptr);
		this->push_back(middle_y_ptr);

		//--------------------------------------------------------------------------------------------
		// Add an object for the radius ...
		std::shared_ptr<GConstrainedDoubleObject> radius_ptr;

		if(startSize < 0.) { // Random initialization of radius
			radius_ptr = std::shared_ptr<GConstrainedDoubleObject>(new GConstrainedDoubleObject(minSize, maxSize));
		} else { // Radius will be set to startSize
			radius_ptr = std::shared_ptr<GConstrainedDoubleObject>(new GConstrainedDoubleObject(startSize, minSize, maxSize));
		}

		// ... equip it with an adaptor ...
		radius_ptr->addAdaptor(gdga_ptr_tmpl);
		// ... and add it to the GParameterObjectCollection representing the triangle
		this->push_back(radius_ptr);

		//--------------------------------------------------------------------------------------------
		// Create GConstrainedDoubleObjects holding three angles ...
		std::shared_ptr<GConstrainedDoubleObject> angle1_ptr(new GConstrainedDoubleObject(0., 1.));
		std::shared_ptr<GConstrainedDoubleObject> angle2_ptr(new GConstrainedDoubleObject(0., 1.));
		std::shared_ptr<GConstrainedDoubleObject> angle3_ptr(new GConstrainedDoubleObject(0., 1.));

		// ... equip them with an adaptor
		angle1_ptr->addAdaptor(gdga_ptr_tmpl);
		angle2_ptr->addAdaptor(gdga_ptr_tmpl);
		angle3_ptr->addAdaptor(gdga_ptr_tmpl);

		// ... and add them to the GParameterObjectCollection representing the triangle
		this->push_back(angle1_ptr);
		this->push_back(angle2_ptr);
		this->push_back(angle3_ptr);

		//--------------------------------------------------------------------------------------------
		// Create GConstrainedDoubleObjects for the three colors and the alpha channel
		std::shared_ptr<GConstrainedDoubleObject> color_r_ptr(new GConstrainedDoubleObject(0., 1.));
		std::shared_ptr<GConstrainedDoubleObject> color_g_ptr(new GConstrainedDoubleObject(0., 1.));
		std::shared_ptr<GConstrainedDoubleObject> color_b_ptr(new GConstrainedDoubleObject(0., 1.));
		std::shared_ptr<GConstrainedDoubleObject> color_a_ptr(new GConstrainedDoubleObject(minOpaqueness, maxOpaqueness));

		// ... equip them with an adaptor
		color_r_ptr->addAdaptor(gdga_ptr_tmpl);
		color_g_ptr->addAdaptor(gdga_ptr_tmpl);
		color_b_ptr->addAdaptor(gdga_ptr_tmpl);
		color_a_ptr->addAdaptor(gdga_ptr_tmpl);

		// ... and add them to the GParameterObjectCollection representing the triangle
		this->push_back(color_r_ptr);
		this->push_back(color_g_ptr);
		this->push_back(color_b_ptr);
		this->push_back(color_a_ptr);
	}

	//---------------------------------------------------------------------------
	// Add three parameters for the background color ...
	std::shared_ptr<GConstrainedDoubleObject> bg_color_r_ptr;
	std::shared_ptr<GConstrainedDoubleObject> bg_color_g_ptr;
	std::shared_ptr<GConstrainedDoubleObject> bg_color_b_ptr;

	if(bgRed < 0) {
		bg_color_r_ptr = std::shared_ptr<GConstrainedDoubleObject>(new GConstrainedDoubleObject(0., 1.));
	} else {
		bg_color_r_ptr = std::shared_ptr<GConstrainedDoubleObject>(new GConstrainedDoubleObject(bgRed, 0., 1.));
	}

	if(bgGreen < 0) {
		bg_color_g_ptr = std::shared_ptr<GConstrainedDoubleObject>(new GConstrainedDoubleObject(0., 1.));
	} else {
		bg_color_g_ptr = std::shared_ptr<GConstrainedDoubleObject>(new GConstrainedDoubleObject(bgGreen, 0., 1.));
	}

	if(bgBlue < 0) {
		bg_color_b_ptr = std::shared_ptr<GConstrainedDoubleObject>(new GConstrainedDoubleObject(0., 1.));
	} else {
		bg_color_b_ptr = std::shared_ptr<GConstrainedDoubleObject>(new GConstrainedDoubleObject(bgBlue, 0., 1.));
	}

	// ... and equip them with an adaptor
	bg_color_r_ptr->addAdaptor(gdga_ptr_tmpl);
	bg_color_g_ptr->addAdaptor(gdga_ptr_tmpl);
	bg_color_b_ptr->addAdaptor(gdga_ptr_tmpl);

	// ... and add them to the object
	this->push_back(bg_color_r_ptr);
	this->push_back(bg_color_g_ptr);
	this->push_back(bg_color_b_ptr);
}

/******************************************************************************/
/**
 * A standard assignment operator. Target file name and the target itself are not copied
 */
const GImageIndividual& GImageIndividual::operator=(const GImageIndividual& cp) {
	GImageIndividual::load_(&cp);
	return *this;
}

/***************************************************************************/
/**
 * Searches for compliance with expectations with respect to another object
 * of the same type
 *
 * @param cp A constant reference to another GObject object
 * @param e The expected outcome of the comparison
 * @param limit The maximum deviation for floating point values (important for similarity checks)
 */
void GImageIndividual::compare(
	const GObject& cp
	, const Gem::Common::expectation& e
	, const double& limit
) const {
	using namespace Gem::Common;

	// Check that we are dealing with a GImageIndividual reference independent of this object and convert the pointer
	const GImageIndividual *p_load = Gem::Common::g_convert_and_compare<GObject, GImageIndividual>(cp, this);

	GToken token("GImageIndividual", e);

	// Compare our parent data ...
	Gem::Common::compare_base<GParameterSet>(IDENTITY(*this, *p_load), token);

	// ... and then the local data
	Gem::Common::compare_t(IDENTITY(nTriangles_, p_load->nTriangles_), token);
	Gem::Common::compare_t(IDENTITY(alphaSort_, p_load->alphaSort_), token);

	// React on deviations from the expectation
	token.evaluate();
}

/*******************************************************************************************/
/**
 * Retrieves the number of triangles
 */
std::size_t GImageIndividual::getNTriangles() const {
	return nTriangles_;
}

/*******************************************************************************************/
/**
 * Retrieve an array with the triangles' data, using the circular triangle definition.
 * Note that this array is sorted in ascending order of opacity and is thus not identical
 * to the order in which triangles are sorted in this individual.
 *
 * @param nT The number of triangles (return parameter)
 * @return An array with the triangle data
 */
std::vector<Gem::Common::t_circle> GImageIndividual::getTriangleData() const {
#ifdef DEBUG
	if(this->size() != 10*nTriangles_+3) { // including background color
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GImageIndividual::getTriangleData(): Error!" << std::endl
				<< "Invalid number of entries in this class " << this->size() << " / " << nTriangles_ << std::endl
		);
	}
#endif /* DEBUG */

	std::size_t offset = 0;
	double angle1 = 0., angle2 = 0., angle3 = 0.;
	std::vector<Gem::Common::t_circle> circle_vec(nTriangles_);
	for(std::size_t i=0; i<nTriangles_; i++) {
		offset = i*10;

		circle_vec[i].middle.x = static_cast<float>(this->at<GConstrainedDoubleObject>(offset+0)->value());
		circle_vec[i].middle.y = static_cast<float>(this->at<GConstrainedDoubleObject>(offset+1)->value());
		circle_vec[i].radius   = static_cast<float>(this->at<GConstrainedDoubleObject>(offset+2)->value());

		angle1 = this->at<GConstrainedDoubleObject>(offset+3)->value();
		angle2 = this->at<GConstrainedDoubleObject>(offset+4)->value();
		angle3 = this->at<GConstrainedDoubleObject>(offset+5)->value();

		// Adjust the angles
		angle1 /= 3.;
		angle2 /= 3.; angle2 += angle1;
		angle3 /= 3.; angle3 += angle2;

		angle3 = Gem::Common::gmin(angle3, 0.99999999);

		circle_vec[i].angle1   = static_cast<float>(angle1);
		circle_vec[i].angle2   = static_cast<float>(angle2);
		circle_vec[i].angle3   = static_cast<float>(angle3);

		circle_vec[i].r        = static_cast<float>(this->at<GConstrainedDoubleObject>(offset+6)->value());
		circle_vec[i].g        = static_cast<float>(this->at<GConstrainedDoubleObject>(offset+7)->value());
		circle_vec[i].b        = static_cast<float>(this->at<GConstrainedDoubleObject>(offset+8)->value());
		circle_vec[i].a        = static_cast<float>(this->at<GConstrainedDoubleObject>(offset+9)->value());
	}

	if(alphaSort_) {
		// Sort circle_vec so that items with higher opacity are in the front position
		std::sort(
			circle_vec.begin()
			, circle_vec.end()
			, [](Gem::Common::t_circle& x, Gem::Common::t_circle& y) {
				return x.getAlphaValue() > y.getAlphaValue();
			}
		);
	}

	return circle_vec;
}

/*******************************************************************************************/
/**
 * Retrieves the background colors
 *
 * @return The background color used for the candidate image
 */
std::tuple<float,float,float> GImageIndividual::getBackGroundColor() const {
	float local_bg_red, local_bg_green, local_bg_blue;
	std::size_t offset = 10*nTriangles_;
	local_bg_red   = static_cast<float>(this->at<GConstrainedDoubleObject>(offset+0)->value());
	local_bg_green = static_cast<float>(this->at<GConstrainedDoubleObject>(offset+1)->value());
	local_bg_blue  = static_cast<float>(this->at<GConstrainedDoubleObject>(offset+2)->value());
	return std::tuple<float,float,float>(local_bg_red, local_bg_green, local_bg_blue);
}

/*******************************************************************************************/
/**
 * Converts the triangle data into a GCanvas object
 *
 * @param dimensions The desired dimensions of the canvas to be created
 * @return A canvas constructed from the triangle data held in this individual
 */
std::shared_ptr<Gem::Common::GCanvas<GII_DEF_COLORDEPTH> > GImageIndividual::toCanvas(
	const std::tuple<std::size_t, std::size_t>& dimensions
) const {
	// Retrieve the background color
	std::tuple<float,float,float> bgcols = this->getBackGroundColor();

	// Retrieve the canvas
	std::shared_ptr<Gem::Common::GCanvas<GII_DEF_COLORDEPTH> >
		canvas_ptr(new Gem::Common::GCanvas<GII_DEF_COLORDEPTH>(dimensions, bgcols));

	// Add the triangles to the canvas
	canvas_ptr->addTriangles(this->getTriangleData());

	// Let the audience know
	return canvas_ptr;
}

/*******************************************************************************************/
/**
 * Writes an image with the current setup to disc. The name is assembled from the current
 * generation, its position in the population, the fitness of this individual and the name
 * of the image the generated picture should resemble.
 *
 * @param path The path where the image should be stored
 * @param filename The name of the file the result should be written to
 */
void GImageIndividual::writeImage(
	const std::string& prefix
	, const std::string& path
	, const std::tuple<std::size_t, std::size_t>& dimensions
) {
	// Cross-check that we can safely access the fitness
	if(this->isDirty()) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GImageIndividual::writeImage():" << std::endl
				<< "Individual has dirty flag set when it shouldn't"
		);
	}

	// Write the generated image out
	std::ostringstream gen_filename_stream;
	gen_filename_stream
		<< prefix << "-result-"
		<< this->getAssignedIteration()
		<< "-" << this->fitness()
		<< ".ppm";

	std::ofstream result((path+gen_filename_stream.str()).c_str());
	result << this->toCanvas(dimensions)->toPPM();
	result.close();
}

/******************************************************************************/
/**
 * Loads the data of another GImageIndividual, camouflaged as a GObject.
 *
 * @param cp A copy of another GImageIndividual, camouflaged as a GObject
 */
void GImageIndividual::load_(const GObject* cp)
{
	// Check that we are indeed dealing with a GImageIndividual reference
	const GImageIndividual *p_load = Gem::Common::g_convert_and_compare<GObject, GImageIndividual>(cp, this);

	// Load our parent's data
	GParameterSet::load_(cp);

	// Load local data
	nTriangles_ = p_load->nTriangles_;
	alphaSort_ = p_load->alphaSort_;
}

/******************************************************************************/
/**
 * Creates a deep clone of this object
 *
 * @return A deep clone of this object, camouflaged as a GObject
 */
GObject* GImageIndividual::clone_() const {
	return new GImageIndividual(*this);
}

/******************************************************************************/
/**
 * The actual fitness calculation takes place here.
 *
 * @param id The id of the target function (ignored here)
 * @return The value of this object
 */
double GImageIndividual::fitnessCalculation() {
	throw gemfony_exception(
		g_error_streamer(DO_LOG,  time_and_place)
			<< "In GImageIndividual::fitnessCalculation(): Error!" << std::endl
			<< "This function is not supposed to be called for this individual." << std::endl
	);

	// Make the exception
	return 0.;
}

/******************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GImageIndividual::modify_GUnitTests() {
#ifdef GEM_TESTING
	using boost::unit_test_framework::test_suite;
	using boost::unit_test_framework::test_case;

	// Call the parent classes' functions
	Gem::Geneva::GParameterSet::modify_GUnitTests();

	// Change the parameter settings
	this->adapt();

	return true;
#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
	condnotset("GImageIndividual::modify_GUnitTests", "GEM_TESTING");
   return false;
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GImageIndividual::specificTestsNoFailureExpected_GUnitTests() {
#ifdef GEM_TESTING
	using namespace Gem::Geneva;

	using boost::unit_test_framework::test_suite;
	using boost::unit_test_framework::test_case;

	// Call the parent classes' functions
	Gem::Geneva::GParameterSet::specificTestsNoFailureExpected_GUnitTests();

	const std::size_t NTESTS = 100;

	//------------------------------------------------------------------------------

	{ // Test that repeated extraction of an object's data results in the same output
		std::shared_ptr<GImageIndividual> p_test = this->clone<GImageIndividual>();

		std::vector<Gem::Common::t_circle> circles = p_test->getTriangleData();

		for(std::size_t i=0; i<NTESTS; i++) {
			std::vector<Gem::Common::t_circle> circles_new = p_test->getTriangleData();
			BOOST_REQUIRE(circles_new == circles);
		}
	}

	//------------------------------------------------------------------------------
#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
	condnotset("GImageIndividual::modify_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GImageIndividual::specificTestsFailuresExpected_GUnitTests() {
#ifdef GEM_TESTING
	using namespace Gem::Geneva;

	using boost::unit_test_framework::test_suite;
	using boost::unit_test_framework::test_case;

	// Call the parent classes' functions
	Gem::Geneva::GParameterSet::specificTestsFailuresExpected_GUnitTests();

	//------------------------------------------------------------------------------
	//------------------------------------------------------------------------------

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
	condnotset("GImageIndividual::modify_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */

}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A constructor with the ability to switch the parallelization mode. It initializes a
 * target item as needed.
 *
 * @param configFile The name of the configuration file
 */
GImageIndividualFactory::GImageIndividualFactory(const std::string& configFile)
	: Gem::Common::GFactoryT<GImageIndividual>(configFile)
	  , adProb_(GII_DEF_ADPROB)
	  , adaptAdProb_(GII_DEF_ADAPTADPROB)
	  , minAdProb_(GII_DEF_MINADPROB)
	  , maxAdProb_(GII_DEF_MAXADPROB)
	  , sigma_(GII_DEF_SIGMA)
	  , sigmaSigma_(GII_DEF_SIGMASIGMA)
	  , minSigma_(GII_DEF_MINSIGMA)
	  , maxSigma_(GII_DEF_MAXSIGMA)
	  , loc_adProb_(GII_DEF_LOC_ADPROB)
	  , loc_adaptAdProb_(GII_DEF_LOC_ADAPTADPROB)
	  , loc_minAdProb_(GII_DEF_LOC_MINADPROB)
	  , loc_maxAdProb_(GII_DEF_LOC_MAXADPROB)
	  , loc_sigma_(GII_DEF_LOC_SIGMA)
	  , loc_sigmaSigma_(GII_DEF_LOC_SIGMASIGMA)
	  , loc_minSigma_(GII_DEF_LOC_MINSIGMA)
	  , loc_maxSigma_(GII_DEF_LOC_MAXSIGMA)
	  , minOpaqueness_(GII_DEF_MINOPAQUENESS)
	  , maxOpaqueness_(GII_DEF_MAXOPAQUENESS)
	  , alphaSort_(GII_DEF_ALPHASORT)
	  , startSize_(GII_DEF_STARTSIZE)
	  , minSize_(GII_DEF_MINSIZE)
	  , maxSize_(GII_DEF_MAXSIZE)
	  , bgRed_(GII_DEF_BGRED)
	  , bgGreen_(GII_DEF_BGGREEN)
	  , bgBlue_(GII_DEF_BGBLUE)
	  , nTriangles_(GII_DEF_NTRIANGLES)
{ /* nothing */ }

/******************************************************************************/
/**
 * The destructor
 */
GImageIndividualFactory::~GImageIndividualFactory()
{ /* nothing */ }

/******************************************************************************/
/**
 * Creates items of this type
 *
 * @return Items of the desired type
 */
std::shared_ptr<GImageIndividual> GImageIndividualFactory::getObject_(
	Gem::Common::GParserBuilder& gpb
	, const std::size_t& id
) {
	// Will hold the result
	std::shared_ptr<GImageIndividual> target(new GImageIndividual());

	// Make the object's local configuration options known
	target->addConfigurationOptions(gpb);

	return target;
}

/******************************************************************************/
/**
 * Allows to describe local configuration options for gradient descents
 */
void GImageIndividualFactory::describeLocalOptions_(Gem::Common::GParserBuilder& gpb) {
	// Describe our own options
	using namespace Gem::Courtier;
	using namespace Gem::Common;

	std::string comment;

	comment = "";
	comment += "The minimum size of the triangle in percent of the canvas;";
	comment += "The allowed value range is [0,maxSize[;";
	gpb.registerFileParameter<double>(
		"minSize"
		, minSize_.reference()
		, GII_DEF_MINSIZE
		, Gem::Common::VAR_IS_ESSENTIAL
		, comment
	);
	checkValueRange(minSize_.value(), 0., 1., GFPLOWERCLOSED, GFPUPPEROPEN, GFNOWARNING, "minSize");

	comment = "";
	comment += "The maximum size of the triangle in percent of the canvas;";
	comment += "The allowed value range is ]minSize,1];";
	gpb.registerFileParameter<double>(
		"maxSize"
		, maxSize_.reference()
		, GII_DEF_MAXSIZE
		, Gem::Common::VAR_IS_ESSENTIAL
		, comment
	);
	checkValueRange(maxSize_.value(), minSize_.value(), 1., GFPLOWEROPEN, GFPUPPEROPEN, GFNOWARNING, "maxSize");

	comment = "";
	comment += "The start size of the triangle in percent of the canvas;";
	comment += "The allowed value range is [minSize,maxSize];";
	gpb.registerFileParameter<double>(
		"startSize"
		, startSize_.reference()
		, GII_DEF_MINSIZE
		, Gem::Common::VAR_IS_ESSENTIAL
		, comment
	);
	checkValueRange(startSize_.value(), minSize_.value(), maxSize_.value(), GFPLOWERCLOSED, GFPUPPEROPEN, GFNOWARNING, "startSize");

	comment = "";
	comment += "The minimum allowed opaqueness of triangles;";
	comment += "The allowed value range is [0,maxOpaqueness];";
	gpb.registerFileParameter<double>(
		"minOpaqueness"
		, minOpaqueness_.reference()
		, GII_DEF_MINOPAQUENESS
		, Gem::Common::VAR_IS_ESSENTIAL
		, comment
	);
	checkValueRange(minOpaqueness_.value(), 0., 1., GFPLOWERCLOSED, GFPUPPEROPEN, GFNOWARNING, "minOpaqueness");

	comment = "";
	comment += "The maximum allowed opaqueness of triangles;";
	comment += "The allowed value range is [minOpaqueness,1];";
	gpb.registerFileParameter<double>(
		"maxOpaqueness"
		, maxOpaqueness_.reference()
		, GII_DEF_MAXOPAQUENESS
		, Gem::Common::VAR_IS_ESSENTIAL
		, comment
	);
	checkValueRange(maxOpaqueness_.value(), minOpaqueness_.value(), 1., GFPLOWEROPEN, GFPUPPEROPEN, GFNOWARNING, "maxOpaqueness");

	comment = "";
	comment += "Determines the rate of adaption of adProb. Set to 0, if you do not need this feature;";
	gpb.registerFileParameter<double>(
		"adaptAdProb"
		, adaptAdProb_.reference()
		, GII_DEF_ADAPTADPROB
		, Gem::Common::VAR_IS_ESSENTIAL
		, comment
	);

	comment = "";
	comment += "Determines the rate of adaption of location-adProb. Set to 0, if you do not need this feature;";
	gpb.registerFileParameter<double>(
		"loc_adaptAdProb"
		, loc_adaptAdProb_.reference()
		, GII_DEF_LOC_ADAPTADPROB
		, Gem::Common::VAR_IS_ESSENTIAL
		, comment
	);

	comment = "";
	comment += "The lower allowed boundary for adProb-variation;";
	gpb.registerFileParameter<double>(
		"minAdProb"
		, minAdProb_.reference()
		, GII_DEF_MINADPROB
		, Gem::Common::VAR_IS_ESSENTIAL
		, comment
	);
	checkValueRange(minAdProb_.value(), 0., 1., GFPLOWERCLOSED, GFPUPPERCLOSED, GFNOWARNING, "minAdProb");

	comment = "";
	comment += "The upper allowed boundary for adProb-variation;";
	gpb.registerFileParameter<double>(
		"maxAdProb"
		, maxAdProb_.reference()
		, GII_DEF_MAXADPROB
		, Gem::Common::VAR_IS_ESSENTIAL
		, comment
	);
	checkValueRange(maxAdProb_.value(), minAdProb_.value(), 1., GFPLOWERCLOSED, GFPUPPERCLOSED, GFNOWARNING, "maxAdProb");

	comment = "";
	comment += "The lower allowed boundary for loc_adProb-variation;";
	gpb.registerFileParameter<double>(
		"loc_minAdProb"
		, loc_minAdProb_.reference()
		, GII_DEF_LOC_MINADPROB
		, Gem::Common::VAR_IS_ESSENTIAL
		, comment
	);
	checkValueRange(loc_minAdProb_.value(), 0., 1., GFPLOWERCLOSED, GFPUPPERCLOSED, GFNOWARNING, "loc_minAdProb");

	comment = "";
	comment += "The upper allowed boundary for loc_adProb-variation;";
	gpb.registerFileParameter<double>(
		"loc_maxAdProb"
		, loc_maxAdProb_.reference()
		, GII_DEF_LOC_MAXADPROB
		, Gem::Common::VAR_IS_ESSENTIAL
		, comment
	);
	checkValueRange(loc_maxAdProb_.value(), loc_minAdProb_.value(), 1., GFPLOWERCLOSED, GFPUPPERCLOSED, GFNOWARNING, "loc_maxAdProb");

	comment = "";
	comment += "The probability for random adaptions of values in evolutionary algorithms;";
	comment += "The allowed value range is [0,1];";
	gpb.registerFileParameter<double>(
		"adProb"
		, adProb_.reference()
		, GII_DEF_ADPROB
		, Gem::Common::VAR_IS_ESSENTIAL
		, comment
	);
	checkValueRange(adProb_.value(), minAdProb_.value(), maxAdProb_.value(), GFPLOWERCLOSED, GFPUPPERCLOSED, GFNOWARNING, "adProb");

	comment = "";
	comment += "The probability for random adaptions of location parameters of values in evolutionary algorithms;";
	comment += "The allowed value range is [0,1];";
	gpb.registerFileParameter<double>(
		"loc_adProb"
		, loc_adProb_.reference()
		, GII_DEF_LOC_ADPROB
		, Gem::Common::VAR_IS_ESSENTIAL
		, comment
	);
	checkValueRange(loc_adProb_.value(), loc_minAdProb_.value(), loc_maxAdProb_.value(), GFPLOWERCLOSED, GFPUPPERCLOSED, GFNOWARNING, "loc_adProb");

	comment = "";
	comment += "The sigma for gauss-adaption in ES;";
	comment += "sigma must be positive;";
	comment += "Recommended value range [0,1];";
	gpb.registerFileParameter<double>(
		"sigma"
		, sigma_.reference()
		, GII_DEF_SIGMA
		, Gem::Common::VAR_IS_ESSENTIAL
		, comment
	);
	checkValueRange(sigma_.value(), 0., 1., GFPLOWERCLOSED, GFPUPPEROPEN, GFNOWARNING, "sigma");

	comment = "";
	comment += "The minimum value of sigma;";
	comment += "minSigma must be positive and smaller than maxSigma;";
	gpb.registerFileParameter<double>(
		"minSigma"
		, minSigma_.reference()
		, GII_DEF_MINSIGMA
		, Gem::Common::VAR_IS_ESSENTIAL
		, comment
	);
	checkValueRange(minSigma_.value(), 0., 1., GFPLOWERCLOSED, GFPUPPEROPEN, GFNOWARNING, "minSigma");

	comment = "";
	comment += "The maximum value of sigma;";
	comment += "maxSigma must be positive and larger than minSigma;";
	gpb.registerFileParameter<double>(
		"maxSigma"
		, maxSigma_.reference()
		, GII_DEF_MAXSIGMA
		, Gem::Common::VAR_IS_ESSENTIAL
		, comment
	);
	checkValueRange(maxSigma_.value(), minSigma_.value(), 1., GFPLOWEROPEN, GFPUPPEROPEN, GFNOWARNING, "maxSigma");

	comment = "";
	comment += "Influences the self-adaption of gauss-mutation in ES;";
	comment += "sigmaSigma must be positive;";
	comment += "The allowed value range is [0,1];";
	gpb.registerFileParameter<double>(
		"sigmaSigma"
		, sigmaSigma_.reference()
		, GII_DEF_SIGMASIGMA
		, Gem::Common::VAR_IS_ESSENTIAL
		, comment
	);
	checkValueRange(sigmaSigma_.value(), 0., 1., GFPLOWERCLOSED, GFPUPPERCLOSED, GFNOWARNING, "sigmaSigma");

	comment = "";
	comment += "The sigma for gauss-adaption of location parameters in ES;";
	comment += "loc_sigma must be positive;";
	comment += "Recommended value range [0,1];";
	gpb.registerFileParameter<double>(
		"loc_sigma"
		, loc_sigma_.reference()
		, GII_DEF_LOC_SIGMA
		, Gem::Common::VAR_IS_ESSENTIAL
		, comment
	);
	checkValueRange(loc_sigma_.value(), 0., 1., GFPLOWERCLOSED, GFPUPPEROPEN, GFNOWARNING, "loc_sigma");

	comment = "";
	comment += "The minimum value of sigma for location parameters;";
	comment += "loc_minSigma must be positive and smaller than loc_maxSigma;";
	gpb.registerFileParameter<double>(
		"loc_minSigma"
		, loc_minSigma_.reference()
		, GII_DEF_LOC_MINSIGMA
		, Gem::Common::VAR_IS_ESSENTIAL
		, comment
	);
	checkValueRange(loc_minSigma_.value(), 0., 1., GFPLOWERCLOSED, GFPUPPEROPEN, GFNOWARNING, "loc_minSigma");

	comment = "";
	comment += "The maximum value of sigma for location parameters;";
	comment += "loc_maxSigma must be positive and larger than loc_minSigma;";
	gpb.registerFileParameter<double>(
		"loc_maxSigma"
		, loc_maxSigma_.reference()
		, GII_DEF_LOC_MAXSIGMA
		, Gem::Common::VAR_IS_ESSENTIAL
		, comment
	);
	checkValueRange(loc_maxSigma_.value(), loc_minSigma_.value(), 1., GFPLOWEROPEN, GFPUPPEROPEN, GFNOWARNING, "loc_maxSigma");

	comment = "";
	comment += "Influences the self-adaption of gauss-mutation in ES for location parameters;";
	comment += "loc_sigmaSigma must be positive;";
	comment += "The allowed value range is [0,1];";
	gpb.registerFileParameter<double>(
		"loc_sigmaSigma"
		, loc_sigmaSigma_.reference()
		, GII_DEF_LOC_SIGMASIGMA
		, Gem::Common::VAR_IS_ESSENTIAL
		, comment
	);
	checkValueRange(loc_sigmaSigma_.value(), 0., 1., GFPLOWERCLOSED, GFPUPPERCLOSED, GFNOWARNING, "loc_sigmaSigma");

	comment = "";
	comment += "The initial background color (red channel);";
	comment += "Negative values mean random initialization;";
	comment += "Otherwise the allowed value range is [0.,1.];";
	gpb.registerFileParameter<double>(
		"bgRed"
		, bgRed_.reference()
		, GII_DEF_BGRED
		, Gem::Common::VAR_IS_ESSENTIAL
		, comment
	);
	checkValueRange(bgRed_.value(), 0., 1., GFPLOWERCLOSED, GFPUPPEROPEN, GFNOWARNING, "bgRed");

	comment = "";
	comment += "The initial background color (green channel);";
	comment += "Negative values mean random initialization;";
	comment += "Otherwise the allowed value range is [0.,1.];";
	gpb.registerFileParameter<double>(
		"bgGreen"
		, bgGreen_.reference()
		, GII_DEF_BGGREEN
		, Gem::Common::VAR_IS_ESSENTIAL
		, comment
	);
	checkValueRange(bgGreen_.value(), 0., 1., GFPLOWERCLOSED, GFPUPPEROPEN, GFNOWARNING, "bgGreen");

	comment = "";
	comment += "The initial background color (blue channel);";
	comment += "Negative values mean random initialization;";
	comment += "Otherwise the allowed value range is [0.,1.];";
	gpb.registerFileParameter<double>(
		"bgBlue"
		, bgBlue_.reference()
		, GII_DEF_BGBLUE
		, Gem::Common::VAR_IS_ESSENTIAL
		, comment
	);
	checkValueRange(bgBlue_.value(), 0., 1., GFPLOWERCLOSED, GFPUPPEROPEN, GFNOWARNING, "bgBlue");

	comment = "";
	comment += "The number of triangles that will constitute;";
	comment += "each candidate image;";
	comment += "Allowed value range [1,1000]";
	gpb.registerFileParameter<std::size_t>(
		"nTriangles"
		, nTriangles_.reference()
		, GII_DEF_NTRIANGLES
		, Gem::Common::VAR_IS_ESSENTIAL
		, comment
	);
	// checkValueRange<std::size_t>(std::size_t(nTriangles_.value()), std::size_t(1), std::size_t(1000), GINTLOWERCLOSED, GINTUPPERCLOSED, GFNOWARNING, "nTriangles");

	comment = "";
	comment += "Whether triangles should be sorted according;";
	comment += "to their alpha channel;";
	gpb.registerFileParameter<bool>(
		"alphaSort"
		, alphaSort_.reference()
		, GII_DEF_ALPHASORT
		, Gem::Common::VAR_IS_ESSENTIAL
		, comment
	);

	// Allow our parent class to describe its options
	Gem::Common::GFactoryT<GImageIndividual>::describeLocalOptions_(gpb);
}

/******************************************************************************/
/**
 * Allows to act on the configuration options received from the configuration file. Here
 * we can add the options described in describeLocalOptions to the object.
 *
 * @param p A smart-pointer to be acted on during post-processing
 */
void GImageIndividualFactory::postProcess_(std::shared_ptr<GImageIndividual>& p) {
	// The image must already have been loaded for this function to work properly
	p->init(
		nTriangles_
		, bgRed_
		, bgGreen_
		, bgBlue_
		, startSize_
		, minSize_
		, maxSize_
		, minOpaqueness_
		, maxOpaqueness_
		, alphaSort_
		, sigma_
		, sigmaSigma_
		, minSigma_
		, maxSigma_
		, adProb_
		, adaptAdProb_
		, minAdProb_
		, maxAdProb_
		, loc_sigma_
		, loc_sigmaSigma_
		, loc_minSigma_
		, loc_maxSigma_
		, loc_adProb_
		, loc_adaptAdProb_
		, loc_minAdProb_
		, loc_maxAdProb_
	);
}

/******************************************************************************/
/**
 * Returns the value of the startSize_ variable
 */
double GImageIndividualFactory::getStartSize() const {
	return startSize_;
}

/******************************************************************************/
/**
 * Returns the value of the adProb_ variable
 */
double GImageIndividualFactory::getAdProb() const
{
	return adProb_;
}

/******************************************************************************/
/**
 * Returns the value of the loc_adaptAdProb_ variable
 */
double GImageIndividualFactory::getLocAdaptAdProb() const
{
	return loc_adaptAdProb_;
}

/******************************************************************************/
/**
 * Returns the value of the adaptAdProb variable
 */
double GImageIndividualFactory::getAdaptAdProb() const
{
	return adaptAdProb_;
}

/******************************************************************************/
/**
 * Returns the value of the loc_adProb variable
 */
double GImageIndividualFactory::getLocAdProb() const
{
	return loc_adProb_;
}

/******************************************************************************/
/**
 * Allows to specify an adaption factor for adProb_ (or 0, if you do not want this feature)
 */
void GImageIndividualFactory::setAdaptAdProb(double adaptAdProb) {
#ifdef DEBUG
	if(adaptAdProb < 0. || adaptAdProb > 1.) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GImageIndividualFactory::setAdaptAdProb(): Error!" << std::endl
				<< "Invalid value for adaptAdProb given: " << adaptAdProb << std::endl
				<< "Expected range of [0:1]" << std::endl
		);
	}
#endif /* DEBUG */

	adaptAdProb_ = adaptAdProb;
}

/******************************************************************************/
/**
 * Allows to specify an adaption factor for loc_adProb_ (or 0, if you do not want this feature)
 */
void GImageIndividualFactory::setLocAdaptAdProb(double loc_adaptAdProb) {
#ifdef DEBUG
	if(loc_adaptAdProb < 0. || loc_adaptAdProb > 1.) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GImageIndividualFactory::setLocAdaptAdProb(): Error!" << std::endl
				<< "Invalid value for loc_adaptAdProb given: " << loc_adaptAdProb << std::endl
				<< "Expected range of [0:1]" << std::endl
		);
	}
#endif /* DEBUG */

	loc_adaptAdProb_ = loc_adaptAdProb;
}

/******************************************************************************/
/**
 * Allows to retrieve the allowed range for adProb_ variation
 */
std::tuple<double,double> GImageIndividualFactory::getAdProbRange() const {
	return std::tuple<double, double>(minAdProb_.value(), maxAdProb_.value());
}

/******************************************************************************/
/**
 * Allows to retrieve the allowed range for loc_adProb_ variation
 */
std::tuple<double,double> GImageIndividualFactory::getLocAdProbRange() const {
	return std::tuple<double, double>(loc_minAdProb_.value(), loc_maxAdProb_.value());
}

/******************************************************************************/
/**
 * Allows to set the allowed range for adaption probability variation
 */
void GImageIndividualFactory::setAdProbRange(
	double minAdProb
	, double maxAdProb
) {
#ifdef DEBUG
	if(minAdProb < 0.) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GImageIndividualFactory::setAdProbRange(): Error!" << std::endl
				<< "minAdProb < 0: " << minAdProb << std::endl
		);
	}

	if(minAdProb > maxAdProb) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GImageIndividualFactory::setAdProbRange(): Error!" << std::endl
				<< "Invalid minAdProb and/or maxAdProb: " << minAdProb << " / " << maxAdProb << std::endl
		);
	}

	if(maxAdProb > 1.) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GImageIndividualFactory::setAdProbRange(): Error!" << std::endl
				<< "maxAdProb > 1: " << maxAdProb << std::endl
		);
	}
#endif /* DEBUG */

	minAdProb_ = minAdProb;
	maxAdProb_ = maxAdProb;
}

/******************************************************************************/
/**
 * Allows to set the allowed range for location adaption probability variation
 */
void GImageIndividualFactory::setLocAdProbRange(
	double minLocAdProb
	, double maxLocAdProb
) {
#ifdef DEBUG
	if(minLocAdProb < 0.) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GImageIndividualFactory::setLocAdProbRange(): Error!" << std::endl
				<< "minLocAdProb < 0: " << minLocAdProb << std::endl
		);
	}

	if(minLocAdProb > maxLocAdProb) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GImageIndividualFactory::setLocAdProbRange(): Error!" << std::endl
				<< "Invalid minLocAdProb and/or maxLocAdProb: " << minLocAdProb << " / " << maxLocAdProb << std::endl
		);
	}

	if(maxLocAdProb > 1.) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GImageIndividualFactory::setLocAdProbRange(): Error!" << std::endl
				<< "maxLocAdProb > 1: " << maxLocAdProb << std::endl
		);
	}
#endif /* DEBUG */

	loc_minAdProb_ = minLocAdProb;
	loc_maxAdProb_ = maxLocAdProb;
}

/******************************************************************************/
/**
 * Returns the value of the maxOpaqueness variable
 */
double GImageIndividualFactory::getMaxOpaqueness() const
{
	return maxOpaqueness_;
}

/******************************************************************************/
/**
 * Returns the value of the maxSigma variable
 */
double GImageIndividualFactory::getMaxSigma() const
{
	return maxSigma_;
}

/******************************************************************************/
/**
 * Returns the value of the loc_maxSigma variable
 */
double GImageIndividualFactory::getLocMaxSigma() const
{
	return loc_maxSigma_;
}

/******************************************************************************/
/**
 * Returns the value of the maxSize_ variable
 */
double GImageIndividualFactory::getMaxSize() const
{
	return maxSize_;
}

/******************************************************************************/
/**
 * Returns the value of the minOpaqueness variable
 */
double GImageIndividualFactory::getMinOpaqueness() const
{
	return minOpaqueness_;
}

/******************************************************************************/
/**
 * Returns the value of the minSigma variable
 */
double GImageIndividualFactory::getMinSigma() const
{
	return minSigma_;
}

/******************************************************************************/
/**
 * Returns the value of the loc_minSigma variable
 */
double GImageIndividualFactory::getLocMinSigma() const
{
	return loc_minSigma_;
}

/******************************************************************************/
/**
 * Returns the value of the minSize_ variable
 */
double GImageIndividualFactory::getMinSize() const
{
	return minSize_;
}

/******************************************************************************/
/**
 * Returns the value of the sigma variable
 */
double GImageIndividualFactory::getSigma() const
{
	return sigma_;
}

/******************************************************************************/
/**
 * Returns the value of the loc_sigma variable
 */
double GImageIndividualFactory::getLocSigma() const
{
	return loc_sigma_;
}

/******************************************************************************/
/**
 * Returns the value of the loc_sigmaSigma_ variable
 */
double GImageIndividualFactory::getLocSigmaSigma() const {
	return loc_sigmaSigma_;
}

/******************************************************************************/
/**
 * Returns the value of the sigmaSigma variable
 */
double GImageIndividualFactory::getSigmaSigma() const {
	return sigmaSigma_;
}

/******************************************************************************/
/**
 * Returns the value of the bgRed variable
 */
double GImageIndividualFactory::getBGRed() const {
	return bgRed_;
}

/******************************************************************************/
/**
 * Returns the value of the bgGreen variable
 */
double GImageIndividualFactory::getBGGreen() const {
	return bgGreen_;
}

/*******************************************************************************/
/**
 * Returns the value of the bgBlue variable
 */
double GImageIndividualFactory::getBGBlue() const {
	return bgBlue_;
}

/******************************************************************************/
/**
 * Returns the value of the nTriangles variable
 */
std::size_t GImageIndividualFactory::getNTriangles() const {
	return nTriangles_;
}

/******************************************************************************/
/**
 * Returns the value of the alphaSort variable
 */
bool GImageIndividualFactory::getAlphaSort() const {
	return alphaSort_;
}

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

#ifdef GEM_TESTING

template <> std::shared_ptr<Gem::Geneva::GImageIndividual> TFactory_GUnitTests<Gem::Geneva::GImageIndividual>() {
	// Create an image individual factory and create the first individual
	Gem::Geneva::GImageIndividualFactory f("../../config/GImageIndividual.json");
	return f();
}

#endif /* GEM_TESTING */
