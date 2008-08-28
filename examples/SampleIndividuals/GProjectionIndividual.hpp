/**
 * @file GProjectionIndividual.hpp
 */

/* Copyright (C) 2004-2008 Dr. Ruediger Berlich
 * Copyright (C) 2007-2008 Forschungszentrum Karlsruhe GmbH
 *
 * This file is part of Geneva, Gemfony scientific's optimization library.
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
#include <boost/serialization/vector.hpp>

#ifndef GPROJECTIONINDIVIDUAL_HPP_
#define GPROJECTIONINDIVIDUAL_HPP_

// GenEvA header files go here
#include "GRandom.hpp"
#include "GDoubleCollection.hpp"
#include "GParameterSet.hpp"
#include "GDoubleGaussAdaptor.hpp"
#include "GenevaExceptions.hpp"
#include "GLogger.hpp"
#include "GLogTargets.hpp"
#include "GBoostThreadPopulation.hpp"

namespace Gem
{
namespace GenEvA
{

/************************************************************************************************/
/** @brief Class to be thrown as an error if the supplied dimensions or number of datasets where incorrect */
class geneva_invalid_dimensions : public boost::exception {};
/** @brief Class to be thrown as an error if the data file could not be opened */
class geneva_invalid_datafile : public boost::exception {};

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
	void serialize(Archive & ar, const unsigned int version) {
		using boost::serialization::make_nvp;

		ar & make_nvp("source", source);
		ar & nake_nvp("nData", nData);
		ar & make_nvp("nDimOrig", nDimOrig);
		ar & make_nvp("nDimTarget", nDimTarget);
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

		ar & make_nvp("ParameterSet", boost::serialization::base_object<GParameterSet>(*this));
		// Note that we (de-)serialize none of the local data here, as it is assumed to be
		// constant and present in every individual for which the load() function could be
		// called. This happens for performance reasons, so we do not have to transfer the
		// potentially very large source_ data over a network. You can uncomment the following
		// lines to try out the difference.
		//
		// ar & make_nvp("source_", source_);
		// ar & make_nvp("nData_", nData_);
		// ar & make_nvp("nDimOrig_", nDimOrig_);
		// ar & make_nvp("nDimTarget_", nDimTarget_);
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
		// knows how to be mutated. We want a sigma of 0.5, sigma-adaption of 0.05 and
		// a minimum sigma of 0.02.
		boost::shared_ptr<GDoubleGaussAdaptor> gdga(new GDoubleGaussAdaptor(0.5,0.05,0.02,"gauss_mutation"));
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

			LOGGER.log(error.str(), Gem::GLogFramework::CRITICAL);
			throw geneva_invalid_dimensions() << error_string(error.str());
		}

		if(source_.size() != nDimOrig_*nData_){
			std::ostringstream error;
			error << "In GProjectionIndividual::GProjectionIndividual([...]) : Error!" << std::endl
				  << "Supplied number of data sets and/or origin-dimension is invalid:" << std::endl
				  << "nData = " << nData_ << std::endl
				  << "nDimOrig = " << nDimOrig_ << std::endl;

			LOGGER.log(error.str(), Gem::GLogFramework::CRITICAL);
			throw geneva_invalid_dimensions() << error_string(error.str());
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
	GProjectionIndividual(const str::string& filename)
		:source_(),
		 nData_(0),
		 nDimOrig_(0),
		 nDimTarget_(0)
	{
		projectionData pD;

		std::ifstream projDat(filename.c_str());

		if(!projData){
			std::ostringstream error;
			error << "In GProjectionIndividual::GProjectionIndividual(const std::string&) : Error!" << std::endl
				  << "Data file " << filename << " could not be opened for reading." << std::endl;

			LOGGER.log(error.str(), Gem::GLogFramework::CRITICAL);
			throw geneva_invalid_datafile() << error_string(error.str());
		}

		// Load the data, using the Boost.Serialization library
		{
			boost::archive::xml_iarchive ia(istr);
			ia >> boost::serialization::make_nvp("projectionData", pD);
		} // Explicit scope at this point is essential so that ia's destructor is called

		projDat.close();

		// Now copy the data over
		source_ = projDat.source_;
		nData_ = projDat.nData_;
		nDimOrig_ = projData.nDimOrig_;
		nDimTarget_ = projData.nDimTarget_;

		// Set up a GDoubleCollection with nval values, each initialized
		// with a random number in the range [min,max[
		boost::shared_ptr<GDoubleCollection> gdc(new GDoubleCollection(nDimTarget_*nData_,min,max));

		// Set up and register an adaptor for the collection, so it
		// knows how to be mutated. We want a sigma of 0.5, sigma-adaption of 0.05 and
		// a minimum sigma of 0.02.
		boost::shared_ptr<GDoubleGaussAdaptor> gdga(new GDoubleGaussAdaptor(0.5,0.05,0.02,"gauss_mutation"));
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

			LOGGER.log(error.str(), Gem::GLogFramework::CRITICAL);
			throw geneva_invalid_dimensions() << error_string(error.str());
		}

		if(source_.size() != nDimOrig_*nData_){
			std::ostringstream error;
			error << "In GProjectionIndividual::GProjectionIndividual(const std::string&) : Error!" << std::endl
				  << "Supplied number of data sets and/or origin-dimension is invalid:" << std::endl
				  << "nData = " << nData_ << std::endl
				  << "nDimOrig = " << nDimOrig_ << std::endl;

			LOGGER.log(error.str(), Gem::GLogFramework::CRITICAL);
			throw geneva_invalid_dimensions() << error_string(error.str());
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
		 source_(cp.source_)
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
	virtual GObject* clone(){
		return new GProjectionIndividual(*this);
	}

	/********************************************************************************************/
	/**
	 * Loads the data of another GProjectionIndividual, camouflaged as a GObject
	 *
	 * @param cp A copy of another GProjectionIndividual, camouflaged as a GObject
	 */
	virtual void load(const GObject* cp){
		// Load the parent class'es data
		GParameterSet::load(cp);

		// We do not load cp's source_ vector, as its data is assumed to be constant
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
	 * GProjectionInvididual::createHyerCubeFile("someFileName.xml");
	 *
	 * @param fileName Name of the file where
	 */
	static void createHyperCubeFile(const std::string& fileName,
				                    std::size_t nData,
				                    std::size_t nDimOrig,
				                    std::size_t nDimTarget,
				                    double edgelength){
		// Check the data
		if(nDimOrig<nDimTarget){
			std::ostringstream error;
			error << "In GProjectionIndividual::createDataFile([...]) : Error!" << std::endl
				  << "Supplied dimensions are invalid:" << std::endl
				  << "nDimOrig = " << nDimOrig << std::endl
				  << "nDimTarget = " << nDimTarget << std::endl;

			LOGGER.log(error.str(), Gem::GLogFramework::CRITICAL);
			throw geneva_invalid_dimensions() << error_string(error.str());
		}

		// Create the required data.
		projectionData pD;

		pD.nData = nData;
		pD.nDimOrig = nDimOrig;
		pD.nDimTarget = nDimTarget;

		for(std::size_t i=0; i<nDimOrig*nData; i++)
			pD.source.push_back(this->gr.evenRandom(-edgelength/2.,edgelength/2.));

		// Serialize and write to a file.
		std::ofstream fileStream(fileName.c_str());
		if(!fileStream){
			std::ostringstream error;
			error << "In GProjectionIndividual::createDataFile([...]) : Error!" << std::endl
				  << "Data file " << filename << " could not be opened for writing." << std::endl;

			LOGGER.log(error.str(), Gem::GLogFramework::CRITICAL);
			throw geneva_invalid_datafile() << error_string(error.str());
		}
		else {
			boost::archive::xml_oarchive oa(fileStream);
			oa << boost::serialization::make_nvp("projectionData", pD);
		}

		fileStream.close();
	}

	/********************************************************************************************/
	/**
	 * This static function can be called in main() in order to create a suitable input file for
	 * this class. It is added here as this individual is for demonstration purposes only and as the
	 * creation of a separate helper program could be avoided in this way. We create a sphere of
	 * radius 2. The projection e.g. into 2d should then simply be a filled circle. See
	 * http://en.wikipedia.org/wiki/Hypersphere for a description of the formula used.
	 *
	 * You can call this function in the following way:
	 *
	 * GProjectionInvididual::createSphereFile("someFileName.xml");
	 *
	 * @param fileName Name of the file where
	 * @param nData The number of data sets to create
	 * @param nDimOrig The dimension of the origin distribution
	 * @param nDimTarget The dimension of the target distribution
	 *
	 * UNFINISHED
	 *
	 */
	static void createSphereFile(const std::string& fileName,
								 std::size_t nData,
								 std::size_t nDimOrig,
								 std::size_t nDimTarget,
								 double radius){
		// Check the data
		if(nDimOrig<nDimTarget){
			std::ostringstream error;
			error << "In GProjectionIndividual::createDataFile([...]) : Error!" << std::endl
				  << "Supplied dimensions are invalid:" << std::endl
				  << "nDimOrig = " << nDimOrig << std::endl
				  << "nDimTarget = " << nDimTarget << std::endl;

			LOGGER.log(error.str(), Gem::GLogFramework::CRITICAL);
			throw geneva_invalid_dimensions() << error_string(error.str());
		}

		// Create the required data.
		projectionData pD;

		pD.nData = nData;
		pD.nDimOrig = nDimOrig;
		pD.nDimTarget = nDimTarget;

		for(std::size_t i=0; i<nData; i++){
			//////////////////////////////////////////////////////////////////
			// Special cases
			if(nDimOrig==1){
				pD.source.push_back(this->gr.evenRandom(0,radius));
				continue;
			}

			if(nDimOrig==2){
				local_radius = this->gr.evenRandom(0,radius);
				phi = this->gr.evenRandom(0,2*M_PI);

				pd.source.push_back(local_radius*sin(phi)); // x
				pd.source.push_back(local_radius*cos(phi)); // y

				continue;
			}

			//////////////////////////////////////////////////////////////////
			// Create the required random number in spherical coordinates.
			// nDimOrig will be at least 3 here.
			std::size_t one_mpi_angles = nDimOrig - 2;
			double local_radius = this->gr.evenRandom(0,radius);
			std::vector<double> angle_collection(one_mpi_angles+1);
			for(std::size_t i=0; i<one_mpi_angles; i++)
				one_mpi_collection.push_back(this->gr.evenRandom(0, M_PI));
			angle_collection[one_mpi_angles]=this->gr.evenRandom(0, 2*M_PI);

			//////////////////////////////////////////////////////////////////
			// Now we can fill the source-vector itself
			pD.source[0]=local_radius*cos(angle_collection[0]);

		}

		// Serialize and write to a file.
		std::ofstream fileStream(fileName.c_str());
		if(!fileStream){
			std::ostringstream error;
			error << "In GProjectionIndividual::createDataFile([...]) : Error!" << std::endl
				  << "Data file " << filename << " could not be opened for writing." << std::endl;

			LOGGER.log(error.str(), Gem::GLogFramework::CRITICAL);
			throw geneva_invalid_datafile() << error_string(error.str());
		}
		else {
			boost::archive::xml_oarchive oa(fileStream);
			oa << boost::serialization::make_nvp("projectionData", pD);
		}

		fileStream.close();
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
		int i, j, k;
		double enumerator = 0.;
		double denominator = 0.;

		for (i = 0; i < nData_; i++) {
			for (j = i + 1; j < nData_; j++) {
				double targetVal = 0;
				double origVal = 0;

				for (k = 0; k < nDimTarget_; k++)
					targetVal += pow(data[i*nDimTarget_ + k] - data[j*nDimTarget_ + k], 2);

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
BOOST_CLASS_EXPORT(Gem::GenEvA::GProjectionIndividual)

#endif /* GPROJECTIONINDIVIDUAL_HPP_ */
