/**
 * @file GDataExchange.hpp
 */

/* Copyright (C) 2004-2008 Dr. Ruediger Berlich
 * Copyright (C) 2007-2008 Forschungszentrum Karlsruhe GmbH
 *
 * This file is part of Geneva, Gemfony scientific's optimization library.
 *
 * This file is free software: you can redistribute it and/or modify
 * it under the terms of version 3 of the GNU Affero General Public License,
 * or, at your option, under the terms of version 2 of the GNU General Public
 * License, as published by the Free Software Foundation.
 *
 * NOTE THAT THIS FORM OF DUAL-LICENSING DOES NOT APPLY TO ANY OTHER FILES
 * OF THE GENEVA LIBRARY, UNLESS THIS IS EXPLICITLY STATED IN A PARTICULAR FILE.
 *
 * Geneva is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of version 2 of the GNU General Public License
 * and of version 3 of the GNU Affero General Public License along with the Geneva
 * library. If not, see <http://www.gnu.org/licenses/>.
 */

// Standard headers go here
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

// Boost headers go here
#include <boost/version.hpp>

#if BOOST_VERSION < 103600
#error "Error: Boost should at least have version 1.36 !"
#endif /* BOOST_VERSION */

#include <boost/cstdint.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/exception.hpp>
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
#include <boost/shared_ptr.hpp>

#ifndef GDATAEXCHANGE_HPP_
#define GDATAEXCHANGE_HPP_

namespace Gem
{
namespace Util
{

/*******************************************************************************/
/**
 * This class serves as an exchange vehicle between external programs and
 * the Geneva library. It allows to store settings particular to a given
 * individual.
 */
class GIndividualData {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template<typename Archive>
    void serialize(Archive & ar, const unsigned int version){
      using boost::serialization::make_nvp;

      ar & make_nvp("dArrays_",dArrays_);
      ar & make_nvp("lArrays_",lArrays_);
      ar & make_nvp("bArrays_",bArrays_);
    }
    ///////////////////////////////////////////////////////////////////////

public:
	/**************************************************************************/
	/**
	 * The default constructor.
	 */
	GIndividualData() throw() { /* nothing */ }

    /**************************************************************************/
	/**
	 * Saves the data associated with this object to a file. Serialization
	 * always happens in binary mode, as it is assumed that this happens on
	 * the same machine as de-serialization..
	 *
	 * @param fileName The name of the file the information should be saved to
	 * @return true if the operation was successful, false otherwise
	 */
	bool saveToFile(const std::string& fileName) {
		std::ofstream paramStream(fileName.c_str(), ios_base::out | ios_base::binary | ios_base::trunc);
		if(!paramStream) {
			std::cerr << "In GIndividualData::saveToFile(): Error!" << std::endl
				      << "Could not open file " << fileName << ". Leaving ..." << std::endl;
			return false;
		}

		{ // note: explicit scope here is essential so the oa-destructor gets called
			boost::archive::binary_oarchive oa(paramStream);
			oa << boost::serialization::make_nvp("GIndividualData", this);
		}

		paramStream.close();
	}

	/**************************************************************************/
	/**
	 * Loads the data associated with this object from a file. De-serialization
	 * always happens in binary mode, as it is assumed that this happens on the
	 * same machine as serialization.
	 *
	 * @param fileName The name of the file the information should be loaded from
	 * @return true if the operation was successful, false otherwise
	 */
	bool loadFromFile(const std::string& fileName) {
		std::ifstream paramStream(fname.c_str(), ios_base::in | ios_base::binary);
		if(!paramStream) {
			std::cerr << "In GIndividualData::loadFromFile(): Error!" << std::endl
				      << "Could not open file " << fileName << ". Leaving ..." << std::endl;
			return false;
		}

 		{ // note: explicit scope here is essential so the ia-destructor gets called
		    boost::archive::binary_iarchive ia(paramStream);
		    ia >> boost::serialization::make_nvp("GIndividualData", this);
 		}

		paramStream.close();
	}

	/**************************************************************************/
	/**
	 * Adds a double vector to the list
	 *
	 * @param dArray An array of double values
	 */
	void appendArray(const std::vector<double>& dArray) {
		dArrays_.push_back(dArray);
	}

	/**************************************************************************/
	/**
	 * Adds a long vector to the list
	 *
	 * @param lArray An array of long values
	 */
	void appendArray(const std::vector<boost::uint32_t>& lArray) {
		lArrays_.push_back(lArray);
	}

	/**************************************************************************/
	/**
	 * Adds a boolean vector to the list
	 *
	 * @param bArray An array of boolean values
	 */
	void appendArray(const std::vector<bool>& bArray) {
		bArrays_.push_back(bArray);
	}

	/**************************************************************************/
	/**
	 * Gives access to the number of double arrays
	 *
	 * @return The number of double arrays
	 */
	std::size_t numberOfDoubleArrays() const throw() {
		return dArrays_.size();
	}

	/**************************************************************************/
	/**
	 * Gives access to the number of long arrays
	 *
	 * @return The number of long arrays
	 */
	std::size_t numberOfLongArrays() const throw() {
		return lArrays_.size();
	}

	/**************************************************************************/
	/**
	 * Gives access to the number of boolean arrays
	 *
	 * @return The number of boolean arrays
	 */
	std::size_t numberOfBooleanArrays() const throw() {
		return bArrays_.size();
	}

	/**************************************************************************/
	/**
	 * Gives access to a given array of double values. Note that this function
	 * will throw if an incorrect position was asked for.
	 *
	 * @param pos The position in the array of vectors
	 * @return The vector at position pos
	 */
	const std::vector<double>& d_at(std::size_t pos) const {
		return dArrays_.at(pos);
	}

	/**************************************************************************/
	/**
	 * Gives access to a given array of long values. Note that this function
	 * will throw if an incorrect position was asked for.
	 *
	 * @param pos The position in the array of vectors
	 * @return The vector at position pos
	 */
	const std::vector<boost::uint32_t>& l_at(std::size_t pos) const {
		return lArrays_.at(pos);
	}

	/**************************************************************************/
	/**
	 * Gives access to a given array of boolean values. Note that this function
	 * will throw if an incorrect position was asked for.
	 *
	 * @param pos The position in the array of vectors
	 * @return The vector at position pos
	 */
	const std::vector<bool>& b_at(std::size_t pos) const {
		return bArrays_.at(pos);
	}

private:
	/**************************************************************************/
	std::vector<std::vector<double> > dArrays_; ///< Arrays holding double values
	std::vector<std::vector<boost::uint32_t> > lArrays_; ///< Arrays holding long values
	std::vector<std::vector<bool> > bArrays_; ///< Arrays holding boolean values
};

/*******************************************************************************/
/**
 * This class serves as an exchange vehicle between external programs and
 * the Geneva library. It allows to store settings particular to a given
 * population.
 */
class GPopulationData {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template<typename Archive>
    void serialize(Archive & ar, const unsigned int version){
      using boost::serialization::make_nvp;

      ar & make_nvp("individuals_",individuals_);
      ar & make_nvp("popSize_",popSize_);
      ar & make_nvp("nParents_",nParents_);
    }
    ///////////////////////////////////////////////////////////////////////

public:
	/**************************************************************************/
	/**
	 * The standard constructor
	 */
	GPopulationData() throw() :nParents_(0), popSize_(0)
	{ /* nothing */ }

    /**************************************************************************/
	/**
	 * Saves the data associated with this object to a file. Serialization
	 * always happens in binary mode, as it is assumed that this happens on
	 * the same machine as de-serialization..
	 *
	 * @param fileName The name of the file the information should be saved to
	 * @return true if the operation was successful, false otherwise
	 */
	bool saveToFile(const std::string& fileName) {
		std::ofstream paramStream(fileName.c_str(), ios_base::out | ios_base::binary | ios_base::trunc);
		if(!paramStream) {
			std::cerr << "In GPopulationData::saveToFile(): Error!" << std::endl
				      << "Could not open file " << fileName << ". Leaving ..." << std::endl;
			return false;
		}

		{ // note: explicit scope here is essential so the oa-destructor gets called
			boost::archive::binary_oarchive oa(paramStream);
			oa << boost::serialization::make_nvp("GPopulationData", this);
		}

		paramStream.close();
	}

	/**************************************************************************/
	/**
	 * Loads the data associated with this object from a file. De-serialization
	 * always happens in binary mode, as it is assumed that this happens on the
	 * same machine as serialization.
	 *
	 * @param fileName The name of the file the information should be loaded from
	 * @return true if the operation was successful, false otherwise
	 */
	bool loadFromFile(const std::string& fileName) {
		std::ifstream paramStream(fname.c_str(), ios_base::in | ios_base::binary);
		if(!paramStream) {
			std::cerr << "In GPopulationData::loadFromFile(): Error!" << std::endl
				      << "Could not open file " << fileName << ". Leaving ..." << std::endl;
			return false;
		}

 		{ // note: explicit scope here is essential so the ia-destructor gets called
		    boost::archive::binary_iarchive ia(paramStream);
		    ia >> boost::serialization::make_nvp("GPopulationData", this);
 		}

		paramStream.close();
	}


	/**************************************************************************/
	/**
	 * Sets the desired number of parents and the size of the population.
	 *
	 * @param popSize The desired size of the population
	 * @param nParents The desired number of parents
	 */
	void setPopulationSize(const std::size_t& popSize, const std::size_t& nParents) throw() {
		if(popSize == 0 || nParents == 0 || nParents >= (std::size_t)(double(popSize)/2.)) {
			std::cerr << "In GPopulationData::setPopulationSize(): Error!" << std::endl
			          << "Invalid population sizes: " << nParents << " " << popSize << std::endl;

			exit(1);
		}

		popSize_ = popSize;
		nParents_ = nParents;
	}

	/**************************************************************************/
	/**
	 * Retrieves the desired population size
	 *
	 * @return The desired population size
	 */
	std::size_t getPopulationSize() const throw() {
		return popSize_;
	}

	/**************************************************************************/
	/**
	 * Retrieves the desired number of parents
	 *
	 * @return The desired number of parents
	 */
	std::size_t getNumberOfParents() const throw() {
		return nParents_;
	}

	/**************************************************************************/
	/**
	 * Retrieves the number of individuals stored in the object
	 *
	 * @return The number of individuals stored in this object
	 */
	std::size_t numberOfIndividuals() const throw() {
		return individuals_.size();
	}

	/**************************************************************************/
	/**
	 * Adds the data for an individual to the object
	 *
	 * @param individual A boost::shared_ptr to an individual
	 */
	void appendIndividual(const boost::shared_ptr<GIndividualData>& individual) {
		if(!individual) { // does it point somewhere ?
			std::cerr << "In  GPopulationData::appendIndividual(): Error!" << std::endl
				      << "Received empty individual" << std::endl;

			exit(1);
		}

		individuals_.push_back(individual);
	}

	/**************************************************************************/
	/**
	 * Retrieves an individual at a given position. Note that this function will
	 * throw if an invalid position is given.
	 *
	 * @param pos The position of the individual in the object
	 * @return A boost::shared_ptr to the desired GIndividualData object
	 */
	boost::shared_ptr<GIndividualData> at(const std::size_t& pos) {
		return individuals_.at(pos);
	}

private:
	/**************************************************************************/
	std::vector<boost::shared_ptr<GIndividualData> > individuals_; ///< An array holding a number of individuals
	std::size_t nParents_; ///< The number of parents in a population
	std::size_t popSize_; ///< The envisaged size of the population
};

/*******************************************************************************/
/**
 * This class serves as an exchange vehicle between external programs and
 * the Geneva library. It allows to retrieve the result of a evaluation.
 */
class GResultData {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template<typename Archive>
    void serialize(Archive & ar, const unsigned int version){
      using boost::serialization::make_nvp;

      ar & make_nvp("result_",result_);
    }
    ///////////////////////////////////////////////////////////////////////

public:
	/**************************************************************************/
	/**
	 * The default constructor.
	 */
	GResultData() throw() :result_(0.) { /* nothing */ }

	/**************************************************************************/
	/**
	 * A constructor that allows to set the result parameter.
	 *
	 * @param result The parameter to be stored in this class
	 */
	GResultData(double result) throw() :result_(result) { /* nothing */ }

    /**************************************************************************/
	/**
	 * Saves the data associated with this object to a file. Serialization
	 * always happens in binary mode, as it is assumed that this happens on
	 * the same machine as de-serialization..
	 *
	 * @param fileName The name of the file the information should be saved to
	 * @return true if the operation was successful, false otherwise
	 */
	bool saveToFile(const std::string& fileName) {
		std::ofstream paramStream(fileName.c_str(), ios_base::out | ios_base::binary | ios_base::trunc);
		if(!paramStream) {
			std::cerr << "In GResultData::saveToFile(): Error!" << std::endl
				      << "Could not open file " << fileName << ". Leaving ..." << std::endl;
			return false;
		}

		{ // note: explicit scope here is essential so the oa-destructor gets called
			boost::archive::binary_oarchive oa(paramStream);
			oa << boost::serialization::make_nvp("GResultData", this);
		}

		paramStream.close();
	}

	/**************************************************************************/
	/**
	 * Loads the data associated with this object from a file. De-serialization
	 * always happens in binary mode, as it is assumed that this happens on the
	 * same machine as serialization.
	 *
	 * @param fileName The name of the file the information should be loaded from
	 * @return true if the operation was successful, false otherwise
	 */
	bool loadFromFile(const std::string& fileName) {
		std::ifstream paramStream(fname.c_str(), ios_base::in | ios_base::binary);
		if(!paramStream) {
			std::cerr << "In GResultData::loadFromFile(): Error!" << std::endl
				      << "Could not open file " << fileName << ". Leaving ..." << std::endl;
			return false;
		}

 		{ // note: explicit scope here is essential so the ia-destructor gets called
		    boost::archive::binary_iarchive ia(paramStream);
		    ia >> boost::serialization::make_nvp("GResultData", this);
 		}

		paramStream.close();
	}

	/**************************************************************************/
	/**
	 * Sets the result parameter.
	 *
	 * @param result The new value to be assigned to the result_ variable
	 */
	void setResult(const double& result) throw() {
		result_ = result;
	}

	/**************************************************************************/
	/**
	 * Retrieves the value of the result_ variable.
	 *
	 * @return The value of the result_ variable
	 */
	double getResult() {
		return result_;
	}

private:
	/**************************************************************************/
	double result_; ///< The result of the calculation
};

/*******************************************************************************/

} /* namespace Util */
} /* namespace Gem */

#endif /* GDATAEXCHANGE_HPP_ */
