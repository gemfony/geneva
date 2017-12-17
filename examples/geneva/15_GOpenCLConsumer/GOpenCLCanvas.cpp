/**
 * @file GOpenCLCanvas.cpp
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

#include "GOpenCLCanvas.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GOpenCLCanvas);

namespace Gem {
namespace Geneva {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * The default constructor
 */
GOpenCLCanvas::GOpenCLCanvas()
	: Gem::Common::GCanvas8()
{ /* nothing */ }

/******************************************************************************/
/**
 * Initialization with dimensions and colors
 */
GOpenCLCanvas::GOpenCLCanvas(
	const std::tuple<std::size_t, std::size_t>& dim
	, const std::tuple<float,float,float>& color
) : Gem::Common::GCanvas8(dim,color)
{ /* nothing */ }

/******************************************************************************/
/**
 * Initialization from data held in a string -- uses the PPM-P3 format
 */
GOpenCLCanvas::GOpenCLCanvas(const std::string& ppmData)
	: Gem::Common::GCanvas8(ppmData)
{ /* nothing */ }

/******************************************************************************/
/**
 * Copy construction
 */
GOpenCLCanvas::GOpenCLCanvas(const GOpenCLCanvas& cp)
	: Gem::Common::GCanvas8(cp)
{ /* nothing */ }

/******************************************************************************/
/**
 * The destructor
 */
GOpenCLCanvas::~GOpenCLCanvas()
{ /* nothing */ }

/******************************************************************************/
/**
 * Find out the deviation between this and another canvas
 */
float GOpenCLCanvas::diff(const GOpenCLCanvas& cp) const {
	return Gem::Common::GCanvas8::diff(cp);
}

/******************************************************************************/
/**
 * Emits the canvas data suitable for transfer to an OpenCL context
 */
std::tuple<std::shared_ptr<cl_uchar>, std::size_t> GOpenCLCanvas::getOpenCLCanvasI() const {
	std::shared_ptr<cl_uchar> oclCanvasData(
		new cl_uchar[4*this->getNPixels()]
		, [](cl_uchar *a) { delete [] a; }
	);
	std::size_t offset = 0;
	for(std::size_t i_y=0; i_y<yDim_; i_y++) {
		for(std::size_t i_x=0; i_x<xDim_; i_x++) {
			offset = 4*(i_y*xDim_ + i_x);

			oclCanvasData.get()[offset+0] = (cl_uchar)(canvasData_[i_x][i_y].r*255.0f);
			oclCanvasData.get()[offset+1] = (cl_uchar)(canvasData_[i_x][i_y].g*255.0f);
			oclCanvasData.get()[offset+2] = (cl_uchar)(canvasData_[i_x][i_y].b*255.0f);
			oclCanvasData.get()[offset+3] = (cl_uchar)255;
		}
	}

	return std::tuple<std::shared_ptr<cl_uchar>, std::size_t>(oclCanvasData, 4*this->getNPixels());
}


/******************************************************************************/
/**
 * Loads the canvas data from a cl_uchar array
 */
void GOpenCLCanvas::loadFromOpenCLArrayI(const std::tuple<std::shared_ptr<cl_uchar>, std::size_t>& c) {
	std::shared_ptr<cl_uchar> oclCanvasData = std::get<0>(c);
	std::size_t nEntries = std::get<1>(c);

	// Check that our dimension fit the number of entries in the array
	if(xDim_*yDim_*4 != nEntries) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GOpenCLCanvas::loadFromOpenCLArray(): Error!" << std::endl
				<< "Dimensions don't fit: " << xDim_ << "/" << yDim_ << "/" << 4*xDim_*yDim_ << "/" << nEntries << std::endl
		);
	}

	// Transfer the data row-wise into the canvas
	std::size_t offset = 0;
	for(std::size_t i_y=0; i_y<yDim_; i_y++) {
		for(std::size_t i_x=0; i_x<xDim_; i_x++) {
			offset = 4*(i_y*xDim_ + i_x);

			canvasData_[i_x][i_y].r = (float)(oclCanvasData.get()[offset+0])/255.f;
			canvasData_[i_x][i_y].g = (float)(oclCanvasData.get()[offset+1])/255.f;
			canvasData_[i_x][i_y].b = (float)(oclCanvasData.get()[offset+2])/255.f;
		}
	}
}

/******************************************************************************/
/**
 * Emits the canvas data suitable for transfer to an OpenCL context
 */
std::tuple<std::shared_ptr<cl_float>, std::size_t> GOpenCLCanvas::getOpenCLCanvasF() const {
	std::shared_ptr<cl_float> oclCanvasData(
		new cl_float[4*this->getNPixels()]
		, [](cl_float *a){ delete [] a; }
	);
	std::size_t offset = 0;
	for(std::size_t i_y=0; i_y<yDim_; i_y++) {
		for(std::size_t i_x=0; i_x<xDim_; i_x++) {
			offset = 4*(i_y*xDim_ + i_x);

			oclCanvasData.get()[offset + 0] = static_cast<cl_float>(canvasData_[i_x][i_y].r);
			oclCanvasData.get()[offset + 1] = static_cast<cl_float>(canvasData_[i_x][i_y].g);
			oclCanvasData.get()[offset + 2] = static_cast<cl_float>(canvasData_[i_x][i_y].b);
			oclCanvasData.get()[offset + 3] = static_cast<cl_float>(255.f);
		}
	}

	return std::tuple<std::shared_ptr<cl_float>, std::size_t>(oclCanvasData, 4*this->getNPixels());
}


/******************************************************************************/
/**
 * Loads the canvas data from a cl_uchar array
 */
void GOpenCLCanvas::loadFromOpenCLArrayF(const std::tuple<std::shared_ptr<cl_float>, std::size_t>& c) {
	std::shared_ptr<cl_float> oclCanvasData = std::get<0>(c);
	std::size_t nEntries = std::get<1>(c);

	// Check that our dimension fit the number of entries in the array
	if(4*xDim_*yDim_ != nEntries) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GOpenCLCanvas::loadFromOpenCLArrayF(): Error!" << std::endl
				<< "Dimensions don't fit: " << xDim_ << "/" << yDim_ << "/" << 4*xDim_*yDim_ << "/" << nEntries << std::endl
		);
	}

	// Transfer the data row-wise into the canvas
	std::size_t offset = 0;
	for(std::size_t i_y=0; i_y<yDim_; i_y++) {
		for(std::size_t i_x=0; i_x<xDim_; i_x++) {
			offset = 4*(i_y*xDim_ + i_x);

			canvasData_[i_x][i_y].r = static_cast<float>(oclCanvasData.get()[offset + 0]);
			canvasData_[i_x][i_y].g = static_cast<float>(oclCanvasData.get()[offset + 1]);
			canvasData_[i_x][i_y].b = static_cast<float>(oclCanvasData.get()[offset + 2]);
		}
	}
}

/******************************************************************************/
/**
 * Emits the canvas data suitable for transfer to an OpenCL context
 */
std::tuple<std::shared_ptr<cl_float4>, std::size_t> GOpenCLCanvas::getOpenCLCanvasF4() const {
	std::shared_ptr<cl_float4> oclCanvasData(
		new cl_float4[this->getNPixels()]
		, [](cl_float4 *a){ delete [] a; }
	);
	std::size_t offset = 0;
	for(std::size_t i_y=0; i_y<yDim_; i_y++) {
		for(std::size_t i_x=0; i_x<xDim_; i_x++) {
			offset = i_y*xDim_ + i_x;

			oclCanvasData.get()[offset].s[0] = static_cast<cl_float>(canvasData_[i_x][i_y].r);
			oclCanvasData.get()[offset].s[1] = static_cast<cl_float>(canvasData_[i_x][i_y].g);
			oclCanvasData.get()[offset].s[2] = static_cast<cl_float>(canvasData_[i_x][i_y].b);
			oclCanvasData.get()[offset].s[3] = static_cast<cl_float>(255.f);
		}
	}

	return std::tuple<std::shared_ptr<cl_float4>, std::size_t>(oclCanvasData, this->getNPixels());
}


/******************************************************************************/
/**
 * Loads the canvas data from a cl_uchar array
 */
void GOpenCLCanvas::loadFromOpenCLArrayF4(const std::tuple<std::shared_ptr<cl_float4>, std::size_t>& c) {
	std::shared_ptr<cl_float4> oclCanvasData = std::get<0>(c);
	std::size_t nEntries = std::get<1>(c);

	// Check that our dimension fit the number of entries in the array
	if(xDim_*yDim_ != nEntries) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GOpenCLCanvas::loadFromOpenCLArrayF4(): Error!" << std::endl
				<< "Dimensions don't fit: " << xDim_ << "/" << yDim_ << "/" << xDim_*yDim_ << "/" << nEntries << std::endl
		);
	}

	// Transfer the data row-wise into the canvas
	std::size_t offset = 0;
	for(std::size_t i_y=0; i_y<yDim_; i_y++) {
		for(std::size_t i_x=0; i_x<xDim_; i_x++) {
			offset = i_y*xDim_ + i_x;

			canvasData_[i_x][i_y].r = static_cast<float>(oclCanvasData.get()[offset].s[0]);
			canvasData_[i_x][i_y].g = static_cast<float>(oclCanvasData.get()[offset].s[1]);
			canvasData_[i_x][i_y].b = static_cast<float>(oclCanvasData.get()[offset].s[2]);
		}
	}
}

/******************************************************************************/
/**
  * Convenience function for the calculation of the difference between two canvasses
  */
float operator-(const GOpenCLCanvas& x, const GOpenCLCanvas& y) {
	return x.diff(y);
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */
