/**
 * @file GProjectionIndividual.hpp
 */

/* Copyright (C) Dr. Ruediger Berlich and Karlsruhe Institute of Technology
 * (University of the State of Baden-Wuerttemberg and National Laboratory
 * of the Helmholtz Association)
 *
 * Contact: info [at] gemfony (dot) com
 *
 * This file is part of the Geneva library, Gemfony scientific's optimization
 * library.
 *
 * Geneva is free software: you can redistribute it and/or modify
 * it under the terms of version 3 of the GNU Affero General Public License
 * as published by the Free Software Foundation.
 *
 * Geneva is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with the Geneva library.  If not, see <http://www.gnu.org/licenses/>.
 *
 * For further information on Gemfony scientific and Geneva, visit
 * http://www.gemfony.com .
 */

// Standard header files go here
#include <iostream>
#include <cmath>
#include <sstream>
#include <vector>
#include <fstream>

// Boost header files go here
#include <boost/version.hpp>

#if BOOST_VERSION < 103600
#error "Error: Boost should at least have version 1.36 !"
#endif /* BOOST_VERSION */

#include <boost/shared_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/tracking.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/export.hpp>


#ifndef GPROJECTIONINDIVIDUAL_HPP_
#define GPROJECTIONINDIVIDUAL_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif

// GenEvA header files go here
#include "GRandom.hpp"
#include "GDoubleCollection.hpp"
#include "GParameterSet.hpp"
#include "GDoubleGaussAdaptor.hpp"
#include "GenevaExceptions.hpp"
#include "GRandom.hpp"

namespace Gem
{
namespace GenEvA
{

/************************************************************************************************/
/**
 * This struct holds all necessary information for the projection individual. It is meant to
 * demonstrate initialization from a file. Here, we simply use serialized data generated from
 * the struct using the Boost.Serialization library.
 */
struct projectionData
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<class Archive>
	void serialize(Archive & ar, const unsigned int) {
		using boost::serialization::make_nvp;

		ar & BOOST_SERIALIZATION_NVP(source)
		   & BOOST_SERIALIZATION_NVP(nData)
		   & BOOST_SERIALIZATION_NVP(nDimOrig)
		   & BOOST_SERIALIZATION_NVP(nDimTarget);
	}
	///////////////////////////////////////////////////////////////////////

	std::vector<double> source; ///< Holds the m-dimensional data set
	std::size_t nData; ///< The amount of data sets
	std::size_t nDimOrig; ///< The dimension of the original distribution
	std::size_t nDimTarget; ///< The dimension of the target distribution
};

/************************************************************************************************/
/**
 * This individual searches for the best n-dimensional representation of an m-dimensional data set
 * (where m>=n). The calculation follows an example given in the book "Evolutionaere Algorithmen" by
 * Ingrid Gerdes, Frank Klawonn and Rudolf Krause (Vieweg Verlag). It extends this function to
 * arbitrary target dimensions (smaller or equal the original dimension of the data).
 *
 * The m-dimensional distribution can either be loaded from a file or is supplied as a constructor
 * argument. It is assumed that this data doesn't change, hence it is not copied in the load-function
 * (but is of course copied in the copy-constructor).
 *
 * For the sake of simplicity, this class contains static helper functions that you can call in
 * order to create a data file suitable for loading in this class. Just call
 *
 * GProjectionIndividual::createHyperCubeFile("someFileName.xml");
 *
 * in main(), if you want to use hypercube data. A sphere generator is also available.
 */
class GProjectionIndividual
	:public GParameterSet
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version) {
		using boost::serialization::make_nvp;

		ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GParameterSet)
		   & BOOST_SERIALIZATION_NVP(source_)
		   & BOOST_SERIALIZATION_NVP(nData_)
		   & BOOST_SERIALIZATION_NVP(nDimOrig_)
		   & BOOST_SERIALIZATION_NVP(nDimTarget_);
	}
	///////////////////////////////////////////////////////////////////////

public:
	/********************************************************************************************/
	/**
	 * A constructor which initializes the individual with a suitable set of random double values.
	 *
	 * @param min The minimum value of the random numbers used to fill the collection
	 * @param max The maximum value of the random numbers used to fill the collection
	 * @param source A vector with the m-dimensional data to be projected to n dimensions
	 * @param nData The amount of data sets in the original and target distributions
	 * @param nDimOrig The dimension of the original distribution
	 * @param nDimTarget The dimension of the target distribution
	 */
	GProjectionIndividual(double min, double max,
						  const std::vector<double>& source,
						  const std::size_t& nData,
						  const std::size_t& nDimOrig,
						  const std::size_t& nDimTarget)
		:source_(source),
		 nData_(nData),
		 nDimOrig_(nDimOrig),
		 nDimTarget_(nDimTarget)
	{
		// Set up a GDoubleCollection with nval values, each initialized
		// with a random number in the range [min,max[
		boost::shared_ptr<GDoubleCollection> gdc(new GDoubleCollection(nDimTarget_*nData_,min,max));

		// Set up and register an adaptor for the collection, so it
		// knows how to be mutated. We want a sigma dependent on the value of max, sigma-adaption of 0.001,
		// a minimum sigma of 0.002 and a maximum sigma dependent on the value of max.
		boost::shared_ptr<GDoubleGaussAdaptor> gdga(new GDoubleGaussAdaptor(fabs(max),0.001,0.002,fabs(max)));
		gdc->addAdaptor(gdga);

		// Make the parameter collection known to this individual
		this->data.push_back(gdc);

		// Check the data we have received
		if(nDimOrig_<nDimTarget_){
			std::ostringstream error;
			error << "In GProjectionIndividual::GProjectionIndividual([...]) : Error!" << std::endl
				  << "Supplied dimensions are invalid:" << std::endl
				  << "nDimOrig = " << nDimOrig_ << std::endl
				  << "nDimTarget = " << nDimTarget_ << std::endl;

			throw geneva_error_condition(error.str());
		}

		if(source_.size() != nDimOrig_*nData_){
			std::ostringstream error;
			error << "In GProjectionIndividual::GProjectionIndividual([...]) : Error!" << std::endl
				  << "Supplied number of data sets and/or origin-dimension is invalid:" << std::endl
				  << "nData = " << nData_ << std::endl
				  << "nDimOrig = " << nDimOrig_ << std::endl;

			throw geneva_error_condition(error.str());
		}
	}

	/********************************************************************************************/
	/**
	 * A constructor which initializes the individual with a suitable set of random double values,
	 * stored in a file. The data was generated by serializing the projectionData struct, as supplied
	 * at the top of this file. Serialization is assumed to have happened in XML mode, so the file
	 * could be edited manually, if necessary. It is assumed that the tag "projectionData" was used
	 * for the serialization.
	 *
	 * @param filename The name of the file holding the necessary data
	 */
	GProjectionIndividual(const std::string& filename, double min, double max)
		:source_(),
		 nData_(0),
		 nDimOrig_(0),
		 nDimTarget_(0)
	{
		projectionData pD;

		std::ifstream projDat(filename.c_str());

		if(!projDat){
			std::ostringstream error;
			error << "In GProjectionIndividual::GProjectionIndividual(const std::string&) : Error!" << std::endl
				  << "Data file " << filename << " could not be opened for reading." << std::endl;

			throw geneva_error_condition(error.str());
		}

		// Load the data, using the Boost.Serialization library
		{
			boost::archive::xml_iarchive ia(projDat);
			ia >> boost::serialization::make_nvp("projectionData", pD);
		} // Explicit scope at this point is essential so that ia's destructor is called

		projDat.close();

		// Now copy the data over
		source_ = pD.source;
		nData_ = pD.nData;
		nDimOrig_ = pD.nDimOrig;
		nDimTarget_ = pD.nDimTarget;

		// Set up a GDoubleCollection with nval values, each initialized
		// with a random number in the range [min,max[
		boost::shared_ptr<GDoubleCollection> gdc(new GDoubleCollection(nDimTarget_*nData_,min,max));

		// Set up and register an adaptor for the collection, so it
		// knows how to be mutated. We want a sigma dependent on the value of max, sigma-adaption of 0.001,
		// a minimum sigma of 0.002 and a maximum sigma dependent on max.
		boost::shared_ptr<GDoubleGaussAdaptor> gdga(new GDoubleGaussAdaptor(fabs(max),0.001,0.002,fabs(max)));
		gdc->addAdaptor(gdga);

		// Make the parameter collection known to this individual
		this->data.push_back(gdc);

		// Check the data we have received
		if(nDimOrig_<nDimTarget_){
			std::ostringstream error;
			error << "In GProjectionIndividual::GProjectionIndividual(const std::string&) : Error!" << std::endl
				  << "Supplied dimensions are invalid:" << std::endl
				  << "nDimOrig = " << nDimOrig_ << std::endl
				  << "nDimTarget = " << nDimTarget_ << std::endl;

			throw geneva_error_condition(error.str());
		}

		if(source_.size() != nDimOrig_*nData_){
			std::ostringstream error;
			error << "In GProjectionIndividual::GProjectionIndividual(const std::string&) : Error!" << std::endl
				  << "Supplied number of data sets and/or origin-dimension is invalid:" << std::endl
				  << "nData = " << nData_ << std::endl
				  << "nDimOrig = " << nDimOrig_ << std::endl;

			throw geneva_error_condition(error.str());
		}
	}

	/********************************************************************************************/
	/**
	 * A standard copy constructor
	 *
	 * @param cp A copy of another GProjectionIndividual object
	 */
	GProjectionIndividual(const GProjectionIndividual& cp)
		:GParameterSet(cp),
		 source_(cp.source_),
		 nData_(cp.nData_),
		 nDimOrig_(cp.nDimOrig_),
		 nDimTarget_(cp.nDimTarget_)
	{ /* nothing */ }

	/********************************************************************************************/
	/**
	 * The standard destructor
	 */
	~GProjectionIndividual()
	{ /* nothing */	}

	/********************************************************************************************/
	/**
	 * A standard assignment operator
	 *
	 * @param cp A copy of another GProjectionIndividual object
	 * @return A reference to this object
	 */
	const GProjectionIndividual& operator=(const GProjectionIndividual& cp){
		GProjectionIndividual::load(&cp);
		return *this;
	}

	/********************************************************************************************/
	/**
	 * Creates a deep clone of this object
	 *
	 * @return A deep clone of this object, camouflaged as a GObject
	 */
	virtual GObject* clone() const {
		return new GProjectionIndividual(*this);
	}

	/********************************************************************************************/
	/**
	 * Loads the data of another GProjectionIndividual, camouflaged as a GObject
	 *
	 * @param cp A copy of another GProjectionIndividual, camouflaged as a GObject
	 */
	virtual void load(const GObject* cp){
		const GProjectionIndividual *gpi_load = conversion_cast(cp, this);

		// Load the parent class'es data
		GParameterSet::load(cp);

		// Load our local data
		nData_ = gpi_load->nData_;
		nDimOrig_ = gpi_load->nDimOrig_;
		nDimTarget_ = gpi_load->nDimTarget_;
		source_ = gpi_load->source_;
	}

	/********************************************************************************************/
	/**
	 * This static function can be called in main() in order to create a suitable input file for
	 * this class. It is added here as this individual is for demonstration purposes only and as the
	 * creation of a separate helper program could be avoided in this way. We use a simple hyper-cube,
	 * ranging from [-edgelength/2,edgelength/2[ in each dimension. The projection e.g. into 2d
	 * should then simply be a square.
	 *
	 * You can call this function in the following way:
	 *
	 * GProjectionInvididual::createHyerCubeFile("someFileName.xml", [other arguments]);
	 *
	 * @param fileName Name of the file where
	 * @param nData The number of data sets to create
	 * @param nDimOrig The dimension of the origin distribution
	 * @param nDimTarget The dimension of the target distribution
	 * @param edgelength The desired edge length of the cube
	 * @return A copy of the projectionData struct that has been created
	 */
	static projectionData createHyperCubeFile(const std::string& fileName,
				                              std::size_t nData,
				                              std::size_t nDimOrig,
				                              std::size_t nDimTarget,
				                              double edgelength)
	{
		// Check the data
		if(nDimOrig<nDimTarget){
			std::ostringstream error;
			error << "In GProjectionIndividual::createDataFile([...]) : Error!" << std::endl
				  << "Supplied dimensions are invalid:" << std::endl
				  << "nDimOrig = " << nDimOrig << std::endl
				  << "nDimTarget = " << nDimTarget << std::endl;

			throw geneva_error_condition(error.str());
		}

		// Create a local random number generator. We cannot access the
		// class'es generator, as this function is static.
		Gem::Util::GRandom l_gr;

		// Create the required data.
		projectionData pD;

		pD.nData = nData;
		pD.nDimOrig = nDimOrig;
		pD.nDimTarget = nDimTarget;

		for(std::size_t i=0; i<nDimOrig*nData; i++)
			pD.source.push_back(l_gr.evenRandom(-edgelength/2.,edgelength/2.));

		if(fileName != ""){ // Serialize and write to a file, if requested.
			std::ofstream fileStream(fileName.c_str());
			if(!fileStream){
				std::ostringstream error;
				error << "In GProjectionIndividual::createDataFile([...]) : Error!" << std::endl
					  << "Data file " << fileName << " could not be opened for writing." << std::endl;

				throw geneva_error_condition(error.str());
			}
			else {
				boost::archive::xml_oarchive oa(fileStream);
				oa << boost::serialization::make_nvp("projectionData", pD);
			}

			fileStream.close();
		}

		return pD;
	}

	/********************************************************************************************/
	/**
	 * This static function can be called in main() in order to create a suitable input file for
	 * this class. It is added here as this individual is for demonstration purposes only and as the
	 * creation of a separate helper program could be avoided in this way. We create a sphere of
	 * radius "radius". The projection e.g. into 2d should then simply be a filled circle. See
	 * http://en.wikipedia.org/wiki/Hypersphere for a description of the formulae used.
	 *
	 * You can call this function in the following way:
	 *
	 * projectionData pd = GProjectionInvididual::createSphereFile("someFileName.xml", [other arguments]);
	 *
	 * If filename is an empty string ("") , then no serialization takes place and the data is retured as
	 * a struct only.
	 *
	 * @param fileName Name of the file the result should be written to
	 * @param nData The number of data sets to create
	 * @param nDimOrig The dimension of the origin distribution
	 * @param nDimTarget The dimension of the target distribution
	 * @param radius The desired radius of the sphere
	 * @return A copy of the projectionData struct that has been created
	 */
	static projectionData createSphereFile(const std::string& fileName,
										   std::size_t nData,
										   std::size_t nDimOrig,
										   std::size_t nDimTarget,
										   double radius)
	{
		// Check the data
		if(nDimOrig<nDimTarget){
			std::ostringstream error;
			error << "In GProjectionIndividual::createDataFile([...]) : Error!" << std::endl
				  << "Supplied dimensions are invalid:" << std::endl
				  << "nDimOrig = " << nDimOrig << std::endl
				  << "nDimTarget = " << nDimTarget << std::endl;

			throw geneva_error_condition(error.str());
		}

		// Create a local random number generator. We cannot access the
		// class'es generator, as this function is static.
		Gem::Util::GRandom l_gr;

		// Create the required data.
		projectionData pD;

		pD.nData = nData;
		pD.nDimOrig = nDimOrig;
		pD.nDimTarget = nDimTarget;

		double local_radius=1.;

		for(std::size_t i=0; i<nData; i++){
			local_radius = l_gr.evenRandom(0,radius);

			//////////////////////////////////////////////////////////////////
			// Special cases
			switch(nDimOrig){
			case 1:
				pD.source.push_back(local_radius);
				break;

			case 2:
				{
					double phi = l_gr.evenRandom(0,2*M_PI);
					pD.source.push_back(local_radius*sin(phi)); // x
					pD.source.push_back(local_radius*cos(phi)); // y
				}
				break;

			default: // dimensions 3 ... inf
				{
					//////////////////////////////////////////////////////////////////
					// Create the required random numbers in spherical coordinates.
					// nDimOrig will be at least 3 here.
					std::size_t nAngles = nDimOrig - 1;
					std::vector<double> angle_collection(nAngles);
					for(std::size_t j=0; j<(nAngles-1); j++){ // Angles in range [0,Pi[
						angle_collection[j]=l_gr.evenRandom(0., M_PI);
					}
					angle_collection[nAngles-1]=l_gr.evenRandom(0., 2.*M_PI); // Range of last angle is [0,2*Pi[

					//////////////////////////////////////////////////////////////////
					// Now we can fill the source-vector itself
					std::vector<double> cartCoord(nDimOrig);

					for(std::size_t j=0; j<nDimOrig; j++) cartCoord[j]=local_radius; // They all have that

					cartCoord[0] *= cos(angle_collection[0]); // x_1 / cartCoord[0]

					for(std::size_t j=1; j<nDimOrig-1; j++){ // x_2 ... x_(n-1) / cartCoord[1] .... cartCoord[n-2]
						for(std::size_t k=0; k<j; k++){
							cartCoord[j] *= sin(angle_collection[k]);
						}
						cartCoord[j] *= cos(angle_collection[j]);
					}

					for(std::size_t j=0; j<nAngles; j++){ // x_n / cartCoord[n-1]
						cartCoord[nDimOrig-1] *= sin(angle_collection[j]);
					}

					// Transfer the results
					for(std::size_t j=0; j<nDimOrig; j++){
						pD.source.push_back(cartCoord[j]);
					}
				}
				break;
			}
		}

		if(fileName != ""){ // Serialize and write to a file, if requested.
			// Serialize and write to a file.
			std::ofstream fileStream(fileName.c_str());
			if(!fileStream){
				std::ostringstream error;
				error << "In GProjectionIndividual::createDataFile([...]) : Error!" << std::endl
					  << "Data file " << fileName << " could not be opened for writing." << std::endl;

				throw geneva_error_condition(error.str());
			}
			else {
				boost::archive::xml_oarchive oa(fileStream);
				oa << boost::serialization::make_nvp("projectionData", pD);
			}

			fileStream.close();
		}

		return pD;
	}

protected:
	/********************************************************************************************/
	/**
	 * The actual fitness calculation (i.e. the projection) place here. See the explanation
	 * of the entire class for further information on what is done here.
	 *
	 * @return The value of this object
	 */
	virtual double fitnessCalculation() {
		std::size_t i, j, k;

		double enumerator = 0.;
		double denominator = 0.;

		// Retrieve the double vector. We have a single GParameterBase object in the individual,
		// of which we know that its "real" type is "GDoubleCollection".
		boost::shared_ptr<GDoubleCollection> data_ptr = pc_at<GDoubleCollection>(0);

		for (i = 0; i < nData_; i++) {
			for (j = i + 1; j < nData_; j++) {
				double targetVal = 0;
				double origVal = 0;

				for (k = 0; k < nDimTarget_; k++)
					targetVal += pow(data_ptr->at(i*nDimTarget_ + k) - data_ptr->at(j*nDimTarget_ + k), 2);

				for (k = 0; k < nDimOrig_; k++)
					origVal += pow(source_[i * nDimOrig_ + k] - source_[j * nDimOrig_ + k], 2);

				denominator += origVal;

				targetVal = sqrt(targetVal);
				origVal = sqrt(origVal);

				enumerator += pow(targetVal - origVal, 2);
			}
		}

		return enumerator / std::max(denominator, 0.000000000000001);
	}

	/********************************************************************************************/

private:
	GProjectionIndividual()	{ /* nothing */ } ///< Default constructor intentionally private and empty

	std::vector<double> source_; ///< Holds the m-dimensional data set

	std::size_t nData_; ///< The amount of data sets
	std::size_t nDimOrig_; ///< The dimension of the original distribution
	std::size_t nDimTarget_; ///< The dimension of the target distribution
};

} /* namespace GenEvA */
} /* namespace Gem */

#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(Gem::GenEvA::projectionData)
BOOST_CLASS_EXPORT(Gem::GenEvA::GProjectionIndividual)

#endif /* GPROJECTIONINDIVIDUAL_HPP_ */
