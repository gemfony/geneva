/**
 * @file GImageIndividual.cpp
 */

/********************************************************************************
 *
 * This file is part of the Geneva library collection. The following license
 * applies to this file:
 *
 * ------------------------------------------------------------------------------
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * ------------------------------------------------------------------------------
 *
 * Note that other files in the Geneva library collection may use a different
 * license. Please see the licensing information in each file.
 *
 ********************************************************************************
 *
 * Geneva was started by Dr. Rüdiger Berlich and was later maintained together
 * with Dr. Ariel Garcia under the auspices of Gemfony scientific. For further
 * information on Gemfony scientific, see http://www.gemfomy.eu .
 *
 * The majority of files in Geneva was released under the Apache license v2.0
 * in February 2020.
 *
 * See the NOTICE file in the top-level directory of the Geneva library
 * collection for a list of contributors and copyright information.
 *
 ********************************************************************************/

#include "GImageIndividual.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GImageIndividual)

namespace Gem::Geneva
{
	/******************************************************************************/
	//////////////////////////////////////////////////////////////////////////////////////////////////
	/******************************************************************************/
	// Add a comparison-operator for our tests
	bool operator==(const CircleTriangle& lhs, const CircleTriangle& rhs)
	{
		return (
				(lhs.r == rhs.r) &&
				(lhs.g == rhs.g) &&
				(lhs.b == rhs.b) &&
				(lhs.a == rhs.a) &&
				(lhs.cx == rhs.cx) &&
				(lhs.cy == rhs.cy) &&
				(lhs.radius == rhs.radius) &&
				(lhs.angle1 == rhs.angle1) &&
				(lhs.angle2 == rhs.angle2) &&
				(lhs.angle3 == rhs.angle3)
			);
	}

	// Output operator for the CircleStruct
	std::ostream& operator<<(std::ostream& os, const CircleTriangle& ct)
	{
		os << "CircleTriangle(" << std::endl;
		os << "r=" << static_cast<int>(ct.r) << ", " << std::endl;
		os << "g=" << static_cast<int>(ct.g) << ", " << std::endl;
		os << "b=" << static_cast<int>(ct.b) << ", " << std::endl;
		os << "a=" << static_cast<int>(ct.a) << ", " << std::endl;
		os << "cx=" << ct.cx << ", " << std::endl;
		os << "cy=" << ct.cy << ", " << std::endl;
		os << "radius=" << ct.radius << ", " << std::endl;
		os << "angle1=" << ct.angle1 << ", " << std::endl;
		os << "angle2=" << ct.angle2 << ", " << std::endl;
		os << "angle3=" << ct.angle3 << std::endl;
		os << ")" << std::endl << std::endl;
		return os;
	}

	/******************************************************************************/
	//////////////////////////////////////////////////////////////////////////////////////////////////
	/******************************************************************************/
	/**
	 * Fills the individual with parameters. Our parameter set consists of nTriangles GParameterObjectCollection
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
		, const bool& alphaSort
		, const bool& changeBGColor
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
	)
	{
		if (minSize < 0. || maxSize > 1. || minSize >= maxSize)
		{
			throw gemfony_exception(
				g_error_streamer(DO_LOG, time_and_place)
				<< "In GImageIndividual::init() : Error!" << std::endl
				<< "Invalid values for minSize and maxSize provided: " << minSize << " / " << maxSize << std::endl
			);
		}

		if (startSize < minSize)
		{
			// Cannot be < 0 as minSize may not be <= 0
			throw gemfony_exception(
				g_error_streamer(DO_LOG, time_and_place)
				<< "In GImageIndividual::init() : Error!" << std::endl
				<< "Invalid values for minSize and startSize provided: " << minSize << " / " << startSize << std::endl
			);
		}

		if (startSize > maxSize)
		{
			throw gemfony_exception(
				g_error_streamer(DO_LOG, time_and_place)
				<< "In GImageIndividual::init() : Error!" << std::endl
				<< "Invalid values for maxSize and startSize provided: " << maxSize << " / " << startSize << std::endl
			);
		}

		if (adaptAdProb < 0. || adaptAdProb > 1.)
		{
			throw gemfony_exception(
				g_error_streamer(DO_LOG, time_and_place)
				<< "In GImageIndividual::init() : Error!" << std::endl
				<< "Invalid value for adaptAdProb provided: " << adaptAdProb << std::endl
			);
		}

		if (loc_adaptAdProb < 0. || loc_adaptAdProb > 1.)
		{
			throw gemfony_exception(
				g_error_streamer(DO_LOG, time_and_place)
				<< "In GImageIndividual::init() : Error!" << std::endl
				<< "Invalid value for loc_adaptAdProb provided: " << loc_adaptAdProb << std::endl
			);
		}

		if (minAdProb >= maxAdProb || minAdProb < 0. || maxAdProb > 1. || adProb < minAdProb || adProb > maxAdProb)
		{
			throw gemfony_exception(
				g_error_streamer(DO_LOG, time_and_place)
				<< "In GImageIndividual::init() : Error!" << std::endl
				<< "Invalid values for minAdprob, maxAdProb or adProb provided: " << minAdProb << " / " << maxAdProb <<
				" / " << adProb << std::endl
			);
		}

		if (loc_minAdProb >= loc_maxAdProb || loc_minAdProb < 0. || loc_maxAdProb > 1. || loc_adProb < loc_minAdProb ||
			loc_adProb > loc_maxAdProb)
		{
			throw gemfony_exception(
				g_error_streamer(DO_LOG, time_and_place)
				<< "In GImageIndividual::init() : Error!" << std::endl
				<< "Invalid values for loc_minAdprob, loc_maxAdProb or loc_adProb provided: " << loc_minAdProb << " / "
				<< loc_maxAdProb << " / " << loc_adProb << std::endl
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
		for (std::size_t t_cnt = 0; t_cnt < nTriangles_; t_cnt++)
		{
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

			if (startSize < 0.)
			{
				// Random initialization of radius
				radius_ptr = std::shared_ptr<GConstrainedDoubleObject>(new GConstrainedDoubleObject(minSize, maxSize));
			}
			else
			{
				// Radius will be set to startSize
				radius_ptr = std::shared_ptr<GConstrainedDoubleObject>(
					new GConstrainedDoubleObject(startSize, minSize, maxSize));
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
			std::shared_ptr<GConstrainedDoubleObject> color_a_ptr(
				new GConstrainedDoubleObject(minOpaqueness, maxOpaqueness));

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
		// Add three parameters for the background color, ...
		std::shared_ptr<GConstrainedDoubleObject> bg_color_r_ptr;
		std::shared_ptr<GConstrainedDoubleObject> bg_color_g_ptr;
		std::shared_ptr<GConstrainedDoubleObject> bg_color_b_ptr;

		if (bgRed < 0)
		{
			bg_color_r_ptr = std::shared_ptr<GConstrainedDoubleObject>(new GConstrainedDoubleObject(0., 1.));
		}
		else
		{
			bg_color_r_ptr = std::shared_ptr<GConstrainedDoubleObject>(new GConstrainedDoubleObject(bgRed, 0., 1.));
		}

		if (bgGreen < 0)
		{
			bg_color_g_ptr = std::shared_ptr<GConstrainedDoubleObject>(new GConstrainedDoubleObject(0., 1.));
		}
		else
		{
			bg_color_g_ptr = std::shared_ptr<GConstrainedDoubleObject>(new GConstrainedDoubleObject(bgGreen, 0., 1.));
		}

		if (bgBlue < 0)
		{
			bg_color_b_ptr = std::shared_ptr<GConstrainedDoubleObject>(new GConstrainedDoubleObject(0., 1.));
		}
		else
		{
			bg_color_b_ptr = std::shared_ptr<GConstrainedDoubleObject>(new GConstrainedDoubleObject(bgBlue, 0., 1.));
		}

		// ... equip them with an adaptor,
		bg_color_r_ptr->addAdaptor(gdga_ptr_tmpl);
		bg_color_g_ptr->addAdaptor(gdga_ptr_tmpl);
		bg_color_b_ptr->addAdaptor(gdga_ptr_tmpl);

		// ... chech whether they shall be modifiable
		changeBGColor_ = changeBGColor;
		if (not changeBGColor)
		{
			bg_color_r_ptr->setAdaptionsInactive();
			bg_color_g_ptr->setAdaptionsInactive();
			bg_color_b_ptr->setAdaptionsInactive();
		}
		else
		{
			bg_color_r_ptr->setAdaptionsActive();
			bg_color_g_ptr->setAdaptionsActive();
			bg_color_b_ptr->setAdaptionsActive();
		}

		// ... and add them to the object
		this->push_back(bg_color_r_ptr);
		this->push_back(bg_color_g_ptr);
		this->push_back(bg_color_b_ptr);
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
	void GImageIndividual::compare_(
		const GObject& cp
		, const Gem::Common::expectation& e
		, const double& limit
	) const
	{
		using namespace Gem::Common;

		// Check that we are dealing with a GImageIndividual reference independent of this object and convert the pointer
		const GImageIndividual* p_load = Gem::Common::g_convert_and_compare<GObject, GImageIndividual>(cp, this);

		GToken token("GImageIndividual", e);

		// Compare our parent data ...
		Gem::Common::compare_base_t<Gem::Geneva::GParameterSet>(*this, *p_load, token);

		// ... and then the local data
		Gem::Common::compare_t(IDENTITY(nTriangles_, p_load->nTriangles_), token);
		Gem::Common::compare_t(IDENTITY(alphaSort_, p_load->alphaSort_), token);
		Gem::Common::compare_t(IDENTITY(changeBGColor_, p_load->changeBGColor_), token);

		// React on deviations from the expectation
		token.evaluate();
	}

	/*******************************************************************************************/
	/**
	 * Retrieves the number of triangles
	 */
	std::size_t GImageIndividual::getNTriangles() const
	{
		return nTriangles_;
	}

	/*******************************************************************************************/
	/**
	 * Retrieve an array with the triangles' data, using the circular triangle definition.
	 * Note that this array might be sorted in ascending order of opacity and might thus not be
	 * identical to the order in which triangles are sorted in this individual.
	 *
	 * @param nT The number of triangles (return parameter)
	 * @return An array with the triangle data
	 */
	std::vector<CircleTriangle> GImageIndividual::getTriangleData() const
	{
#ifdef DEBUG
		if (this->size() != 10 * nTriangles_ + 3)
		{
			// including background color
			throw gemfony_exception(
				g_error_streamer(DO_LOG, time_and_place)
				<< "In GImageIndividual::getTriangleData(): Error!" << std::endl
				<< "Invalid number of entries in this class " << this->size() << " / " << nTriangles_ << std::endl
			);
		}
#endif /* DEBUG */

		std::size_t offset = 0;
		double angle1 = 0., angle2 = 0., angle3 = 0.;
		std::vector<CircleTriangle> circle_cnt(nTriangles_);
		for (std::size_t i = 0; i < nTriangles_; i++)
		{
			offset = i * 10;

			circle_cnt[i].cx = static_cast<float>(this->at<GConstrainedDoubleObject>(offset + 0)->value());
			circle_cnt[i].cy = static_cast<float>(this->at<GConstrainedDoubleObject>(offset + 1)->value());
			circle_cnt[i].radius = static_cast<float>(this->at<GConstrainedDoubleObject>(offset + 2)->value());

			angle1 = this->at<GConstrainedDoubleObject>(offset + 3)->value();
			angle2 = this->at<GConstrainedDoubleObject>(offset + 4)->value();
			angle3 = this->at<GConstrainedDoubleObject>(offset + 5)->value();

			std::array<float, 3> angles = {
				static_cast<float>(angle1 * 2*M_PI), static_cast<float>(angle2 * 2*M_PI), static_cast<float>(angle3 * 2*M_PI)
			};
			std::sort(angles.begin(), angles.end(), std::less<float>());

			circle_cnt[i].angle1 = angles[0];
			circle_cnt[i].angle2 = angles[1];
			circle_cnt[i].angle3 = angles[2];

			circle_cnt[i].r =
				std::clamp(static_cast<unsigned char>(this->at<GConstrainedDoubleObject>(offset + 6)->value() * 255.),
				           static_cast<unsigned char>(0.),
				           static_cast<unsigned char>(255.f));
			circle_cnt[i].g =
				std::clamp(static_cast<unsigned char>(this->at<GConstrainedDoubleObject>(offset + 7)->value() * 255.),
				           static_cast<unsigned char>(0.),
				           static_cast<unsigned char>(255.f));

			circle_cnt[i].b =
				std::clamp(static_cast<unsigned char>(this->at<GConstrainedDoubleObject>(offset + 8)->value() * 255.),
				           static_cast<unsigned char>(0.),
				           static_cast<unsigned char>(255.f));

			circle_cnt[i].a =
				std::clamp(static_cast<unsigned char>(this->at<GConstrainedDoubleObject>(offset + 9)->value() * 255.),
				           static_cast<unsigned char>(0.),
				           static_cast<unsigned char>(255.f));
		}

		if (alphaSort_)
		{
			// Sort circle_cnt so that items with higher opacity are in the front position
			std::sort(
				circle_cnt.begin()
				, circle_cnt.end()
				, [](CircleTriangle& x, CircleTriangle& y)
				{
					return x.a > y.a;
				}
			);
		}

		return circle_cnt;
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
		const GImageIndividual* p_load = Gem::Common::g_convert_and_compare<GObject, GImageIndividual>(cp, this);

		// Load our parent's data
		GParameterSet::load_(cp);

		// Load local data
		nTriangles_ = p_load->nTriangles_;
		alphaSort_ = p_load->alphaSort_;
		changeBGColor_ = p_load->changeBGColor_;
	}

	/******************************************************************************/
	/**
	 * Creates a deep clone of this object
	 *
	 * @return A deep clone of this object, camouflaged as a GObject
	 */
	GObject* GImageIndividual::clone_() const
	{
		return new GImageIndividual(*this);
	}

	/******************************************************************************/
	/**
	 * The actual fitness calculation takes place here.
	 *
	 * @return The value of this object
	 */
	double GImageIndividual::fitnessCalculation()
	{
		throw gemfony_exception(
			g_error_streamer(DO_LOG, time_and_place)
			<< "In GImageIndividual::fitnessCalculation(): Error!" << std::endl
			<< "This function is not meant to be called for this individual" << std::endl
		);

		// Make the compiler happy
		return 0.;
	}

	/******************************************************************************/
	/**
	 * Applies modifications to this object. This is needed for testing purposes
	 *
	 * @return A boolean which indicates whether modifications were made
	 */
	bool GImageIndividual::modify_GUnitTests_()
	{
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
	void GImageIndividual::specificTestsNoFailureExpected_GUnitTests_()
	{
#ifdef GEM_TESTING
		using namespace Gem::Geneva;

		using boost::unit_test_framework::test_suite;
		using boost::unit_test_framework::test_case;

		// Call the parent classes' functions
		Gem::Geneva::GParameterSet::specificTestsNoFailureExpected_GUnitTests();

		const std::size_t NTESTS = 100;

		//------------------------------------------------------------------------------

		{
			// Test that repeated extraction of an object's data results in the same output
			const std::shared_ptr<GImageIndividual> p_test = this->clone<GImageIndividual>();

			const auto circles = p_test->getTriangleData();

			for (std::size_t i = 0; i < NTESTS; i++)
			{
				auto circles_new = p_test->getTriangleData();
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
	void GImageIndividual::specificTestsFailuresExpected_GUnitTests_()
	{
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
	{
		/* nothing */
	}

	/******************************************************************************/
	/**
	 * Creates items of this type
	 *
	 * @return Items of the desired type
	 */
	std::shared_ptr<GImageIndividual> GImageIndividualFactory::getObject_(
		Gem::Common::GParserBuilder& gpb
		, const std::size_t& id
	)
	{
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
	void GImageIndividualFactory::describeLocalOptions_(Gem::Common::GParserBuilder& gpb)
	{
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
		checkValueRange(startSize_.value(), minSize_.value(), maxSize_.value(), GFPLOWERCLOSED, GFPUPPEROPEN,
		                GFNOWARNING, "startSize");

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
		checkValueRange(maxOpaqueness_.value(), minOpaqueness_.value(), 1., GFPLOWEROPEN, GFPUPPEROPEN, GFNOWARNING,
		                "maxOpaqueness");

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
		checkValueRange(maxAdProb_.value(), minAdProb_.value(), 1., GFPLOWERCLOSED, GFPUPPERCLOSED, GFNOWARNING,
		                "maxAdProb");

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
		checkValueRange(loc_maxAdProb_.value(), loc_minAdProb_.value(), 1., GFPLOWERCLOSED, GFPUPPERCLOSED, GFNOWARNING,
		                "loc_maxAdProb");

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
		checkValueRange(adProb_.value(), minAdProb_.value(), maxAdProb_.value(), GFPLOWERCLOSED, GFPUPPERCLOSED,
		                GFNOWARNING, "adProb");

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
		checkValueRange(loc_adProb_.value(), loc_minAdProb_.value(), loc_maxAdProb_.value(), GFPLOWERCLOSED,
		                GFPUPPERCLOSED, GFNOWARNING, "loc_adProb");

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
		checkValueRange(loc_maxSigma_.value(), loc_minSigma_.value(), 1., GFPLOWEROPEN, GFPUPPEROPEN, GFNOWARNING,
		                "loc_maxSigma");

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

		comment = "";
		comment += "Whether the background color shall be mutated;";
		gpb.registerFileParameter<bool>(
			"changeBGColor"
			, changeBGColor_.reference()
			, GII_DEF_CHBGCOLOR
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
	void GImageIndividualFactory::postProcess_(std::shared_ptr<GImageIndividual>& p)
	{
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
			, changeBGColor_
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
	double GImageIndividualFactory::getStartSize() const
	{
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
	void GImageIndividualFactory::setAdaptAdProb(double adaptAdProb)
	{
#ifdef DEBUG
		if (adaptAdProb < 0. || adaptAdProb > 1.)
		{
			throw gemfony_exception(
				g_error_streamer(DO_LOG, time_and_place)
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
	void GImageIndividualFactory::setLocAdaptAdProb(double loc_adaptAdProb)
	{
#ifdef DEBUG
		if (loc_adaptAdProb < 0. || loc_adaptAdProb > 1.)
		{
			throw gemfony_exception(
				g_error_streamer(DO_LOG, time_and_place)
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
	auto GImageIndividualFactory::getAdProbRange() const
	{
		return std::tuple<double, double>{minAdProb_.value(), maxAdProb_.value()};
	}

	/******************************************************************************/
	/**
	 * Allows to retrieve the allowed range for loc_adProb_ variation
	 */
	auto GImageIndividualFactory::getLocAdProbRange() const
	{
		return std::tuple<double, double>{loc_minAdProb_.value(), loc_maxAdProb_.value()};
	}

	/******************************************************************************/
	/**
	 * Allows to set the allowed range for adaption probability variation
	 */
	void GImageIndividualFactory::setAdProbRange(
		double minAdProb
		, double maxAdProb
	)
	{
#ifdef DEBUG
		if (minAdProb < 0.)
		{
			throw gemfony_exception(
				g_error_streamer(DO_LOG, time_and_place)
				<< "In GImageIndividualFactory::setAdProbRange(): Error!" << std::endl
				<< "minAdProb < 0: " << minAdProb << std::endl
			);
		}

		if (minAdProb > maxAdProb)
		{
			throw gemfony_exception(
				g_error_streamer(DO_LOG, time_and_place)
				<< "In GImageIndividualFactory::setAdProbRange(): Error!" << std::endl
				<< "Invalid minAdProb and/or maxAdProb: " << minAdProb << " / " << maxAdProb << std::endl
			);
		}

		if (maxAdProb > 1.)
		{
			throw gemfony_exception(
				g_error_streamer(DO_LOG, time_and_place)
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
	)
	{
#ifdef DEBUG
		if (minLocAdProb < 0.)
		{
			throw gemfony_exception(
				g_error_streamer(DO_LOG, time_and_place)
				<< "In GImageIndividualFactory::setLocAdProbRange(): Error!" << std::endl
				<< "minLocAdProb < 0: " << minLocAdProb << std::endl
			);
		}

		if (minLocAdProb > maxLocAdProb)
		{
			throw gemfony_exception(
				g_error_streamer(DO_LOG, time_and_place)
				<< "In GImageIndividualFactory::setLocAdProbRange(): Error!" << std::endl
				<< "Invalid minLocAdProb and/or maxLocAdProb: " << minLocAdProb << " / " << maxLocAdProb << std::endl
			);
		}

		if (maxLocAdProb > 1.)
		{
			throw gemfony_exception(
				g_error_streamer(DO_LOG, time_and_place)
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
	double GImageIndividualFactory::getLocSigmaSigma() const
	{
		return loc_sigmaSigma_;
	}

	/******************************************************************************/
	/**
	 * Returns the value of the sigmaSigma variable
	 */
	double GImageIndividualFactory::getSigmaSigma() const
	{
		return sigmaSigma_;
	}

	/******************************************************************************/
	/**
	 * Returns the value of the bgRed variable
	 */
	double GImageIndividualFactory::getBGRed() const
	{
		return bgRed_;
	}

	/******************************************************************************/
	/**
	 * Returns the value of the bgGreen variable
	 */
	double GImageIndividualFactory::getBGGreen() const
	{
		return bgGreen_;
	}

	/*******************************************************************************/
	/**
	 * Returns the value of the bgBlue variable
	 */
	double GImageIndividualFactory::getBGBlue() const
	{
		return bgBlue_;
	}

	/******************************************************************************/
	/**
	 * Returns the value of the nTriangles variable
	 */
	std::size_t GImageIndividualFactory::getNTriangles() const
	{
		return nTriangles_;
	}

	/******************************************************************************/
	/**
	 * Returns the value of the alphaSort variable
	 */
	bool GImageIndividualFactory::getAlphaSort() const
	{
		return alphaSort_;
	}

	/******************************************************************************/
} // namespace Gem::Geneva


/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

#ifdef GEM_TESTING

template <>
std::shared_ptr<Gem::Geneva::GImageIndividual> TFactory_GUnitTests<Gem::Geneva::GImageIndividual>()
{
	// Create an image individual factory and create the first individual
	Gem::Geneva::GImageIndividualFactory f("../../config/GImageIndividual.json");
	return f();
}

#endif /* GEM_TESTING */
