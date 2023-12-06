/**
 * @file GImageOpenCLWorker.cpp
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

#include "GImageOpenCLWorker.hpp"

namespace Gem::Geneva {

/******************************************************************************/
/**
 * convenience-function allowing easier access to the content of this struct
 */
std::ostream& operator<<(std::ostream& out, const t_ocl_cart& tc) {
	out
		<< "tr_one.x   = " << tc.tr_one.s[0]   << std::endl
		<< "tr_one.y   = " << tc.tr_one.s[1]   << std::endl
		<< "tr_two.x   = " << tc.tr_two.s[0]   << std::endl
		<< "tr_two.y   = " << tc.tr_two.s[1]   << std::endl
		<< "tr_three.x = " << tc.tr_three.s[0] << std::endl
		<< "tr_three.y = " << tc.tr_three.s[1] << std::endl
		<< "rgba_f.x   = " << tc.rgba_f.s[0] << std::endl
		<< "rgba_f.y   = " << tc.rgba_f.s[1] << std::endl
		<< "rgba_f.z   = " << tc.rgba_f.s[2] << std::endl
		<< "rgba_f.w   = " << tc.rgba_f.s[3] << std::endl;
	return out;
}

/******************************************************************************/
/** @brief Assignment of a Gem::Common::triangle_circle_struct struct */
void triangle_ocl_circle_struct::operator=(
	const Gem::Common::triangle_circle_struct& tcs
) {
	middleX = tcs.middle.x;
	middleY = tcs.middle.y;
	radius = tcs.radius;
	angle1 = tcs.angle1;
	angle2 = tcs.angle2;
	angle3 = tcs.angle3;
	rgba_f.s[0] = static_cast<cl_float>(tcs.r);
	rgba_f.s[1] = static_cast<cl_float>(tcs.g);
	rgba_f.s[2] = static_cast<cl_float>(tcs.b);
	rgba_f.s[3] = static_cast<cl_float>(tcs.a);
}

/******************************************************************************/
/** @brief Assignment of another triangle_ocl_circle_struct */
const triangle_ocl_circle_struct& triangle_ocl_circle_struct::operator=(
	const triangle_ocl_circle_struct& toclcs
) {
	middleX = toclcs.middleX;
	middleY = toclcs.middleY;
	radius = toclcs.radius;
	angle1 = toclcs.angle1;
	angle2 = toclcs.angle2;
	angle3 = toclcs.angle3;
	rgba_f.s[0] = toclcs.rgba_f.s[0];
	rgba_f.s[1] = toclcs.rgba_f.s[1];
	rgba_f.s[2] = toclcs.rgba_f.s[2];
	rgba_f.s[3] = toclcs.rgba_f.s[3];

	return *this;
}

/******************************************************************************/
/**
 * Comparison with self
 */
bool operator==(
	const triangle_ocl_circle_struct& a
	, triangle_ocl_circle_struct& b
) {
	if(a.middleX != b.middleX) return false;
	if(a.middleY != b.middleY) return false;
	if(a.radius != b.radius) return false;
	if(a.angle1 != b.angle1) return false;
	if(a.angle2 != b.angle2) return false;
	if(a.angle3 != b.angle3) return false;
	if(a.rgba_f.s[0] != b.rgba_f.s[0]) return false;
	if(a.rgba_f.s[1] != b.rgba_f.s[1]) return false;
	if(a.rgba_f.s[2] != b.rgba_f.s[2]) return false;
	if(a.rgba_f.s[3] != b.rgba_f.s[3]) return false;

	return true;
}


/******************************************************************************/
/**
 * Comparison with self
 */
bool operator!=(
	const triangle_ocl_circle_struct& a
	, triangle_ocl_circle_struct& b
) {
	return !operator==(a,b);
}

/******************************************************************************/
/**
 * Output operator for easier access
 */
std::ostream& operator<<(std::ostream& o, const t_ocl_circle& t) {
	o
		<< "t.middleX = " << t.middleX << std::endl
		<< "t.middleY = " << t.middleY << std::endl
		<< "t.radius = " << t.radius << std::endl
		<< "t.angle1 = " << t.angle1 << std::endl
		<< "t.angle2 = " << t.angle2 << std::endl
		<< "t.angle3 = " << t.angle3 << std::endl
		<< "t.rgba_f.x = " << t.rgba_f.s[0] << std::endl
		<< "t.rgba_f.y = " << t.rgba_f.s[1] << std::endl
		<< "t.rgba_f.z = " << t.rgba_f.s[2] << std::endl
		<< "t.rgba_f.w = " << t.rgba_f.s[3] << std::endl;

	return o;
}

/******************************************************************************/
/**
 * Initialization with an external OpenCL context and the name of a configuration file.
 */
GImageOpenCLWorker::GImageOpenCLWorker(
	const cl::Device& device
	, const std::string& configFile
)
	: Gem::Courtier::GOpenCLWorkerT<GParameterSet>(device, configFile)
      , global_results_(nullptr)
	  , circle_triangles_(nullptr)
	  , imageFile_()
	  , targetCanvas_()
	  , dimX_(0), dimY_(0)
	  , targetSize_(0)
	  , nWorkGroups_(0)
	  , useGPU_(false)
	  , nTriangles_(GII_DEF_NTRIANGLES)
{
	// By parsing the file here we make available the
	// image dimensions. Likewise, we can load the canvas
	// data here, so that we can simply copy it in the
	// copy constructor and don't need to load the data
	// over and over again for each worker.
	this->parseConfigFile(configFile);

	// Load the image
	loadTargetFromFile();

	// No need to initialize the buffers and arrays here
}

/******************************************************************************/
/**
 * Initialization with the data needed for an optimization run
 */
GImageOpenCLWorker::GImageOpenCLWorker(
	const GImageOpenCLWorker& cp
)
	: Gem::Courtier::GOpenCLWorkerT<GParameterSet>(cp)
	  , global_results_(nullptr)
	  , circle_triangles_(nullptr)
	  , imageFile_(cp.imageFile_)
	  , targetCanvas_(cp.targetCanvas_)
	  , dimX_(cp.dimX_), dimY_(cp.dimY_)
	  , targetSize_(cp.targetSize_)
	  , nWorkGroups_(cp.nWorkGroups_)
	  , useGPU_(cp.useGPU_)
	  , nTriangles_(cp.nTriangles_)
{
	// Initialization of local variables directly related to OpenCL is done in initOpenCL()
}

/******************************************************************************/
/**
 * The destructor. The arrays are allocated inside of initOpenCL().
 */
GImageOpenCLWorker::~GImageOpenCLWorker()
{
	if(global_results_) delete [] global_results_;
	if(circle_triangles_) delete [] circle_triangles_;
}

/******************************************************************************/
/**
 * Creates a deep clone of this object, camouflaged as a GWorker
 */
std::shared_ptr<Gem::Courtier::GWorkerT<GParameterSet>>
GImageOpenCLWorker::clone_() const {
	return std::shared_ptr<GImageOpenCLWorker>(new GImageOpenCLWorker(*this));
}


/******************************************************************************/
/**
 * Allows to perform any initialization work required prior to building the program objects.
 * Particularly, it is possible to set up the data needed for the OpenCL compiler options.
 */
void GImageOpenCLWorker::initOpenCL(std::shared_ptr<GParameterSet> p) {
	std::shared_ptr<GImageIndividual> p_conv;

#ifdef DEBUG
	// Check that p actually points somewhere
	if(!p) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GImageOpenCLWorker::initOpenCL() : Error!" << std::endl
				<< "p is empty" << std::endl
		);
	}

	p_conv = std::dynamic_pointer_cast<GImageIndividual>(p);
	if(!p_conv) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GImageOpenCLWorker::initOpenCL(): Error!" << std::endl
				<< "Conversion failed" << std::endl
		);
	}
#else
	// Translate the individual to the target type GImageIndividual
   p_conv = std::static_pointer_cast<GImageIndividual>(p);
#endif /* DEBUG */

	nTriangles_ = p_conv->getNTriangles();

	// Initialize the candidate triangles and result arrays
	global_results_ = new cl_float[nWorkGroups_]; // We do not want to re-initialize the existing buffer
	circle_triangles_ = new t_ocl_circle[nTriangles_];

	std::tuple<std::shared_ptr<cl_float>, std::size_t> oclCanvasData = targetCanvas_.getOpenCLCanvasF();

	// Initialize buffers and load the target image to "our" device
	target_image_buffer_ = cl::Image2D(
		Gem::Courtier::GOpenCLWorkerT<GParameterSet>::m_context
		, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR // This will copy the data in the host_ptr
		, cl::ImageFormat(CL_RGBA, CL_FLOAT)
		, dimX_, dimY_
		, 0
		, (void *)(std::get<0>(oclCanvasData).get()) // TODO: Does this copy the data ?
	);

	candidate_image_buffer_ = cl::Image2D(
		Gem::Courtier::GOpenCLWorkerT<GParameterSet>::m_context
		, CL_MEM_READ_WRITE
		, cl::ImageFormat(CL_RGBA, CL_FLOAT) // alpha will be 1.f or 255
		, dimX_, dimY_
	);

	global_results_buffer_ = cl::Buffer(Gem::Courtier::GOpenCLWorkerT<GParameterSet>::m_context, CL_MEM_WRITE_ONLY, nWorkGroups_*sizeof(cl_float));
	circ_triangle_buffer_  = cl::Buffer(Gem::Courtier::GOpenCLWorkerT<GParameterSet>::m_context, CL_MEM_READ_ONLY,  nTriangles_*sizeof(t_ocl_circle)); // A host-ptr will be copied later
	cart_triangle_buffer_  = cl::Buffer(Gem::Courtier::GOpenCLWorkerT<GParameterSet>::m_context, CL_MEM_READ_WRITE, nTriangles_*sizeof(t_ocl_cart));
}

/******************************************************************************/
/**
 * Initialization of kernel objects
 */
void GImageOpenCLWorker::initKernels(std::shared_ptr<GParameterSet> p) {
	tr_transcode_kernel_ = cl::Kernel(Gem::Courtier::GOpenCLWorkerT<GParameterSet>::m_program, "monalisa_triangle_transcode");
	candidate_creator_kernel_ = cl::Kernel(Gem::Courtier::GOpenCLWorkerT<GParameterSet>::m_program, "monalisa_candidate_creator");
	candidate_deviation_kernel_ = cl::Kernel(Gem::Courtier::GOpenCLWorkerT<GParameterSet>::m_program, "monalisa_candidate_deviation");
}

/******************************************************************************/
/**
 * Emits compiler options for OpenCL
 */
std::string GImageOpenCLWorker::getCompilerOptions() const {
	std::string compilerOptions =
		" -DNTRIANGLES=" + Gem::Common::to_string(nTriangles_)
		+ " -DXDIM=" + Gem::Common::to_string(dimX_)
		+ " -DYDIM=" + Gem::Common::to_string(dimY_)
		+ " -DXDIMINV=" + Gem::Common::to_string(1.f/float(dimX_))
		+ " -DYDIMINV=" + Gem::Common::to_string(1.f/float(dimY_))
		+ Gem::Courtier::GOpenCLWorkerT<GParameterSet>::getCompilerOptions();

	return compilerOptions;
}

/******************************************************************************/
/**
 * Perform the OpenCL-based evaluation
 */
std::vector<double> GImageOpenCLWorker::openCLCalc(std::shared_ptr<GImageIndividual> p_conv) {
	float result = 0.f;
	std::vector<double> results;

	//---------------------------------------------------------------------------------------------
	// Extract the triangle data and background colors

	std::vector<Gem::Common::t_circle> t_data  = p_conv->getTriangleData();

	// Transfer the data to an array of t_ocl_circle structs
	for(std::size_t i=0; i<nTriangles_; i++) {
		circle_triangles_[i] = t_data[i];
	}

	// Transfer the triangle array to the device
	Gem::Courtier::GOpenCLWorkerT<GParameterSet>::m_queue.enqueueWriteBuffer(circ_triangle_buffer_, CL_TRUE, 0, sizeof(t_ocl_circle) * nTriangles_, (void *)circle_triangles_);

	//set the arguments of our kernel
	tr_transcode_kernel_.setArg(0, circ_triangle_buffer_);
	tr_transcode_kernel_.setArg(1, cart_triangle_buffer_);

	// Execute the kernel and wait for its termination
	{ // Compare e.g. https://bbs.archlinux.org/viewtopic.php?id=138292 . Could this be due to the profiling option of the queue ?
		cl::Event event;
        Gem::Courtier::GOpenCLWorkerT<GParameterSet>::m_queue.enqueueNDRangeKernel(tr_transcode_kernel_, cl::NullRange, cl::NDRange(nTriangles_), cl::NullRange, NULL, &event);
		event.wait();
	}

	//---------------------------------------------------------------------------------------------
	// Run the candidate creator kernel

	auto bgcol = p_conv->getBackGroundColor();
	cl_float4 oclBgCol;
	oclBgCol.s[0] = std::get<0>(bgcol);
	oclBgCol.s[1] = std::get<1>(bgcol);
	oclBgCol.s[2] = std::get<2>(bgcol);
	oclBgCol.s[3] = (cl_float)(1.f);

	candidate_creator_kernel_.setArg(0, cart_triangle_buffer_);
	candidate_creator_kernel_.setArg(1, candidate_image_buffer_);
	candidate_creator_kernel_.setArg(2, oclBgCol);

	// Start the actual execution
	{
		cl::Event event;
        Gem::Courtier::GOpenCLWorkerT<GParameterSet>::m_queue.enqueueNDRangeKernel(candidate_creator_kernel_, cl::NullRange, cl::NDRange(dimX_, dimY_), cl::NullRange, NULL, &event);
		event.wait();
	}

	//---------------------------------------------------------------------------------------------
	// Calculate the deviation between candidate and target

	candidate_deviation_kernel_.setArg(0, candidate_image_buffer_);
	candidate_deviation_kernel_.setArg(1, target_image_buffer_);
	candidate_deviation_kernel_.setArg(2, global_results_buffer_);

	// Start the actual execution
	{
		cl::Event event;
        Gem::Courtier::GOpenCLWorkerT<GParameterSet>::m_queue.enqueueNDRangeKernel(candidate_deviation_kernel_, cl::NullRange, cl::NDRange(targetSize_), cl::NDRange(m_workGroupSize), NULL, &event);
		event.wait();
	}

	// Retrieve the results buffer
    Gem::Courtier::GOpenCLWorkerT<GParameterSet>::m_queue.enqueueReadBuffer(global_results_buffer_, CL_TRUE, 0, nWorkGroups_*sizeof(cl_float), global_results_);
	for(int i=0; i<nWorkGroups_; i++) {
		result += global_results_[i];
	}

	results.push_back((double)result);
    std::cout << "Result =" << result << std::endl;
	return results;
}

/******************************************************************************/
/**
 * Perform the OpenCL-based evaluation
 */
std::vector<double> GImageOpenCLWorker::cpuCalc(std::shared_ptr<GImageIndividual> p_conv) {
	std::vector<double> results;
	results.push_back((double)p_conv->toCanvas(std::tuple<std::size_t, std::size_t>(dimX_, dimY_))->diff(targetCanvas_));
	return results;
}


/******************************************************************************/
/**
 * The actual per-item work is done here
 */
void GImageOpenCLWorker::process_(std::shared_ptr<GParameterSet> p) {
	std::shared_ptr<GImageIndividual> p_conv;

#ifdef DEBUG
	// Check that p actually points somewhere
	if(!p) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GImageConsumer::GOpenCLWorker : Error!" << std::endl
				<< "p is empty" << std::endl
		);
	}

	p_conv = std::dynamic_pointer_cast<GImageIndividual>(p);
	if(!p_conv) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GImageConsumer::process(): Error!" << std::endl
				<< "Conversion failed" << std::endl
		);
	}
#else
	// Translate the individual to the target type GImageIndividual
   p_conv = std::static_pointer_cast<GImageIndividual>(p);
#endif /* DEBUG */

	std::function<std::vector<double>()> f;
	if(useGPU_) {
          auto openCLResults = this->openCLCalc(p_conv);
          std::size_t pos = 0;
          for(auto result: openCLResults) p_conv->setResult(pos++, result);
	} else {
          auto cpuResults = this->cpuCalc(p_conv);
          std::size_t pos = 0;
          for(auto result: cpuResults) p_conv->setResult(pos++, result);
	}
}

/******************************************************************************/
/**
 * Retrieve the image dimensions
 */
std::tuple<std::size_t,std::size_t>
GImageOpenCLWorker::getImageDimensions() const {
	return std::tuple<std::size_t,std::size_t>{(std::size_t)dimX_, (std::size_t)dimY_};
}

/******************************************************************************/
/**
 * Sets the amount of triangles constituting each image
 */
[[maybe_unused]] void GImageOpenCLWorker::setNTriangles(const std::size_t& nT) {
	nTriangles_ = nT;
}

/******************************************************************************/
/**
 * Adds local configuration options to a GParserBuilder object
 */
void GImageOpenCLWorker::addConfigurationOptions_(
	Gem::Common::GParserBuilder& gpb
) {
	// Call our parent class'es function
	Gem::Courtier::GOpenCLWorkerT<GParameterSet>::addConfigurationOptions_(gpb);

	std::string comment;

	comment = "";
	comment += "The name of the file holding the target image;";
	gpb.registerFileParameter<std::string>(
		"imageFile"
		, imageFile_
		, GII_DEF_IMAGEFILE
		, Gem::Common::VAR_IS_ESSENTIAL
		, comment
	);

	comment = "";
	comment += "Indicates whether evaluation should run on the GPU (1); or the CPU (0)";
	gpb.registerFileParameter<bool>(
		"useGPU"
		, useGPU_
		, GII_DEF_USEGPU
		, Gem::Common::VAR_IS_ESSENTIAL
		, comment
	);
}

/******************************************************************************/
/**
 * Loads the target image from disk and extracts dimension information. It then
 * sets up the Image buffer for the GPU.
 *
 * @param target The name of the target image file
 */
void GImageOpenCLWorker::loadTargetFromFile() {
	targetCanvas_.loadFromFile(imageFile_);
	dimX_ = targetCanvas_.getXDim();
	dimY_ = targetCanvas_.getYDim();
	targetSize_ = targetCanvas_.getNPixels();

	if(targetSize_ % Gem::Courtier::GOpenCLWorkerT<GParameterSet>::m_workGroupSize != 0) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GImageOpenCLWorker::loadTargetFromFile(): Error!" << std::endl
				<< "Image has invalid dimensions " << dimX_ << "/" << dimY_ << std::endl
				<< "The number of pixels should be a multiple of the work group size " << m_workGroupSize << std::endl
		);
	}

	nWorkGroups_ = targetSize_/Gem::Courtier::GOpenCLWorkerT<GParameterSet>::m_workGroupSize;
}

/******************************************************************************/

} /* namespace Gem::Geneva */
