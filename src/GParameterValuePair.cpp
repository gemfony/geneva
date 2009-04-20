/**
 * @file GParameterValuePair.cpp
 */

/* Copyright (C) 2009 Dr. Ruediger Berlich
 *
 * This file is part of Geneva, Gemfony scientific's optimization library.
 *
 * This file is free software: you can redistribute it and/or modify
 * it under the terms of version 3 of the GNU Affero General Public License,
 * or, at your option, under the terms of version 2 of the GNU General Public
 * License, as published by the Free Software Foundation.
 *
 * NOTE THAT THIS FORM OF DUAL-LICENSING DOES NOT APPLY TO ANY OTHER FILES
 * OF THE GENEVA LIBRARY, UNLESS THIS IS EXPLICITLY STATED IN THE CORRSPONDING FILE.
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

#include "GParameterValuePair.hpp"

/**
 * Included here so no conflicts occur. See explanation at
 * http://www.boost.org/libs/serialization/doc/special.html#derivedpointers
 */
#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(Gem::Util::GParameterValuePair)

namespace Gem
{
	namespace Util
	{

	/******************************************************************************/
	/**
	 * The standard constructor
	 */
	GParameterValuePair::GParameterValuePair()
		:value_(0.),
		hasValue_(false)
	{ /* nothing */ }

	/******************************************************************************/
	/**
	 * A standard copy constructor
	 */
	GParameterValuePair::GParameterValuePair(const GParameterValuePair& cp) {
		// Copy the double vector's content
		std::vector<boost::shared_ptr<GDoubleParameter> >::const_iterator dcit;
		for(dcit=cp.dArray_.begin(); dcit!=cp.dArray_.end(); ++dcit) {
			boost::shared_ptr<GDoubleParameter> p(new GDoubleParameter(*(dcit->get())));
			dArray_.push_back(p);
		}

		// Copy the long vector's content
		std::vector<boost::shared_ptr<GLongParameter> >::const_iterator lcit;
		for(lcit=cp.lArray_.begin(); lcit!=cp.lArray_.end(); ++lcit) {
			boost::shared_ptr<GLongParameter> p(new GLongParameter(*(lcit->get())));
			lArray_.push_back(p);
		}

		// Copy the bool vector's content
		std::vector<boost::shared_ptr<GBoolParameter> >::const_iterator bcit;
		for(bcit=cp.bArray_.begin(); bcit!=cp.bArray_.end(); ++bcit) {
			boost::shared_ptr<GBoolParameter> p(new GBoolParameter(*(bcit->get())));
			bArray_.push_back(p);
		}

		// Copy the char vector's content
		std::vector<boost::shared_ptr<GCharParameter> >::const_iterator ccit;
		for(ccit=cp.cArray_.begin(); ccit!=cp.cArray_.end(); ++ccit) {
			boost::shared_ptr<GCharParameter> p(new GCharParameter(*(ccit->get())));
			cArray_.push_back(p);
		}

		value_ = cp.value_;
		hasValue_ = cp.hasValue_;
	}

	/******************************************************************************/
	/**
	 * The destructor
	 */
	GParameterValuePair::~GParameterValuePair(){
		dArray_.clear();
		lArray_.clear();
		bArray_.clear();
		cArray_.clear();
	}

	/******************************************************************************/
	/**
	 * A standard assignment operator. As it needs to take care of differing
	 * vector sizes, it is far more complicated than the copy constructor. We
	 * thus use an external helper function to carry out the procedure.
	 */
	const GParameterValuePair& GParameterValuePair::operator=(const GParameterValuePair& cp) {
		copySmartPointerVector<GDoubleParameter>(cp.dArray_, dArray_); 		// Copy the double vector's content
		copySmartPointerVector<GLongParameter>(cp.lArray_, lArray_); 			// Copy the long vector's content
		copySmartPointerVector<GBoolParameter>(cp.bArray_, bArray_); 			// Copy the bool vector's content
		copySmartPointerVector<GCharParameter>(cp.cArray_, cArray_);			// Copy the char vector's content

		value_ = cp.value_;
		hasValue_ = cp.hasValue_;

		return cp;
	}

#define GENEVATESTING

	/******************************************************************************/
	/**
	 * Checks equality of this object with another object of the same type.
	 * Equality means in this context that the values all parameters and arrays
	 * are equal.
	 *
	 * @param cp A constant reference to another GParameterValuePair object
	 * @return A boolean indicating whether both objects are equal
	 */
	bool GParameterValuePair::operator==(const GParameterValuePair& cp) const {
		// Check the "easy" values
		if(hasValue_ != cp.hasValue_) {
#ifdef GENEVATESTING
			std::cout << "hasValue_ != cp.hasValue_" << std::endl;
#endif
			return false;
		}

		if(value_ != cp.value_) {
#ifdef GENEVATESTING
			std::cout << "value_ != cp.value_: " << value_ << " " << cp.value_ << std::endl;
#endif
			return false;
		}

		// Check the equality of the double vector
		if(dArray_.size() != cp.dArray_.size()) {
#ifdef GENEVATESTING
			std::cout << "dArray_.size() != cp.dArray_.size(): " << dArray_.size() << " " << cp.dArray_.size() << std::endl;
#endif
			return false;
		}
		std::vector<boost::shared_ptr<GDoubleParameter> >::const_iterator dcit, cp_dcit;
		for(dcit=dArray_.begin(), cp_dcit=cp.dArray_.begin();
		dcit!=dArray_.end(), cp_dcit!=cp.dArray_.end(); ++dcit, ++cp_dcit) {
			if(**dcit != **cp_dcit) {
#ifdef GENEVATESTING
			std::cout << "**dcit != **cp_dcit : " << **dcit << " " << **cp_dcit << std::endl;
#endif
				return false;
			}
		}

		// Check the equality of the long vector
		if(lArray_.size() != cp.lArray_.size()) {
#ifdef GENEVATESTING
			std::cout << "lArray_.size() != cp.lArray_.size(): " << lArray_.size() << " " << cp.lArray_.size() << std::endl;
#endif
			return false;
		}
		std::vector<boost::shared_ptr<GLongParameter> >::const_iterator lcit, cp_lcit;
		for(lcit=lArray_.begin(), cp_lcit=cp.lArray_.begin();
		lcit!=lArray_.end(), cp_lcit!=cp.lArray_.end(); ++lcit, ++cp_lcit) {
			if(**lcit != **cp_lcit) {
#ifdef GENEVATESTING
				std::cout << "**lcit != **cp_lcit : " << **lcit << " " << **cp_lcit << std::endl;
#endif
				return false;
			}
		}

		// Check the equality of the bool vector
		if(bArray_.size() != cp.bArray_.size()) {
#ifdef GENEVATESTING
			std::cout << "bArray_.size() != cp.bArray_.size(): " << bArray_.size() << " " << cp.bArray_.size() << std::endl;
#endif
			return false;
		}
		std::vector<boost::shared_ptr<GBoolParameter> >::const_iterator bcit, cp_bcit;
		for(bcit=bArray_.begin(), cp_bcit=cp.bArray_.begin();
		bcit!=bArray_.end(), cp_bcit!=cp.bArray_.end(); ++bcit, ++cp_bcit) {
			if(**bcit != **cp_bcit) {
#ifdef GENEVATESTING
				std::cout << "**bcit != **cp_bcit : " << **bcit << " " << **cp_bcit << std::endl;
#endif
				return false;
			}
		}

		// Check the equality of the char vector
		if(cArray_.size() != cp.cArray_.size()) {
#ifdef GENEVATESTING
			std::cout << "cArray_.size() != cp.cArray_.size(): " << cArray_.size() << " " << cp.cArray_.size() << std::endl;
#endif
			return false;
		}
		std::vector<boost::shared_ptr<GCharParameter> >::const_iterator ccit, cp_ccit;
		for(ccit=cArray_.begin(), cp_ccit=cp.cArray_.begin();
		ccit!=cArray_.end(), cp_ccit!=cp.cArray_.end(); ++ccit, ++cp_ccit) {
			if(**ccit != **cp_ccit) {
#ifdef GENEVATESTING
				std::cout << "**ccit != **cp_ccit : " << **ccit << " " << **cp_ccit << std::endl;
#endif
				return false;
			}
		}

		// Now we are sure that all parameters are equal
		return true;
	}

	/******************************************************************************/
	/**
	 * Checks inequality of this object with another object of the same type.
	 *
	 * @param cp A constant reference to another GParameterValuePair object
	 * @param A boolean indicating whether both objects are not equal
	 */
	bool GParameterValuePair::operator!=(const GParameterValuePair& cp) const {
		return !operator==(cp);
	}

	/******************************************************************************/
	/**
	 * Checks whether this object is similar to another. For most parameters in this
	 * object this means equality. However, double values may differ by a certain
	 * amount indicated by the "limit" parameter.
	 *
	 * @param cp A constant reference to another GParameterValuePair object
	 * @param limit Acceptable differences between double values
	 * @return A boolean indicating whether both objects are similar to each other
	 */
	bool GParameterValuePair::isSimilarTo(const GParameterValuePair& cp, const double& limit) const {
		// Check the "easy" values
		if(hasValue_ != cp.hasValue_) {
#ifdef GENEVATESTING
			std::cerr << "hasValue_ != cp.hasValue_ : " << hasValue_ << " " << cp.hasValue_ << std::endl;
#endif /* GENEVATESTING */

			return false;
		}

		// This is a double value. Hence we cannot test equality, but need
		// to test for similarity, using the same limit as for all other double values
		if(fabs(value_ - cp.value_) > fabs(limit)) {
#ifdef GENEVATESTING
			std::cerr << "value_ != cp.value_ : " << value_ << " " << cp.value_ << std::endl;
#endif /* GENEVATESTING */

			return false;
		}

		// Check the equality of the double vector
		if(dArray_.size() != cp.dArray_.size()) {
#ifdef GENEVATESTING
			std::cerr << "dArray_.size() != cp.dArray_.size() : " << dArray_.size() << " " << cp.dArray_.size() << std::endl;
#endif /* GENEVATESTING */

			return false;
		}
		std::vector<boost::shared_ptr<GDoubleParameter> >::const_iterator dcit, cp_dcit;

		//-----------------------------------------------------------------------------------------------------
		// Here we use the GDoubleParameter value's own function to determine similarity
		for(dcit=dArray_.begin(), cp_dcit=cp.dArray_.begin();
			  dcit!=dArray_.end(), cp_dcit!=cp.dArray_.end(); ++dcit, ++cp_dcit) {
			if(!(*dcit)->isSimilarTo(**cp_dcit, limit)) {
#ifdef GENEVATESTING
			std::cerr << "**dcit is not similar to **cp_dcit" << std::endl;
#endif /* GENEVATESTING */

				return false;
			}
		}
		//-----------------------------------------------------------------------------------------------------

		// Check the equality of the long vector
		if(lArray_.size() != cp.lArray_.size()) {
#ifdef GENEVATESTING
			std::cerr << "lArray_.size() != cp.lArray_.size() : " << lArray_.size() << " " << cp.lArray_.size() << std::endl;
#endif /* GENEVATESTING */

			return false;
		}
		std::vector<boost::shared_ptr<GLongParameter> >::const_iterator lcit, cp_lcit;
		for(lcit=lArray_.begin(), cp_lcit=cp.lArray_.begin();
		lcit!=lArray_.end(), cp_lcit!=cp.lArray_.end(); ++lcit, ++cp_lcit) {
			if(**lcit != **cp_lcit) {
#ifdef GENEVATESTING
			std::cerr << "**lcit != **cp_lcit" << std::endl;
#endif /* GENEVATESTING */

				return false;
			}
		}

		// Check the equality of the bool vector
		if(bArray_.size() != cp.bArray_.size()) {
#ifdef GENEVATESTING
			std::cerr << "bArray_.size() != cp.bArray_.size() : " << bArray_.size() << " " << cp.bArray_.size() << std::endl;
#endif /* GENEVATESTING */

			return false;
		}
		std::vector<boost::shared_ptr<GBoolParameter> >::const_iterator bcit, cp_bcit;
		for(bcit=bArray_.begin(), cp_bcit=cp.bArray_.begin();
		bcit!=bArray_.end(), cp_bcit!=cp.bArray_.end(); ++bcit, ++cp_bcit) {
			if(**bcit != **cp_bcit) {
#ifdef GENEVATESTING
			std::cerr << "**bcit != **cp_bcit" << std::endl;
#endif /* GENEVATESTING */

				return false;
			}
		}

		// Check the equality of the char vector
		if(cArray_.size() != cp.cArray_.size()) {
#ifdef GENEVATESTING
			std::cerr << "cArray_.size() != cp.cArray_.size() : " << cArray_.size() << " " << cp.cArray_.size() << std::endl;
#endif /* GENEVATESTING */

			return false;
		}
		std::vector<boost::shared_ptr<GCharParameter> >::const_iterator ccit, cp_ccit;
		for(ccit=cArray_.begin(), cp_ccit=cp.cArray_.begin();
		ccit!=cArray_.end(), cp_ccit!=cp.cArray_.end(); ++ccit, ++cp_ccit) {
			if(**ccit != **cp_ccit) {
#ifdef GENEVATESTING
			std::cerr << "**ccit != **cp_ccit" << std::endl;
#endif /* GENEVATESTING */

				return false;
			}
		}

		// Now we are sure that all parameters are equal
		return true;
	}

#undef GENEVATESTING

	/******************************************************************************/
	/**
	 * Resets the structure to its initial state
	 */
	void GParameterValuePair::reset() {
		dArray_.clear();
		lArray_.clear();
		bArray_.clear();
		cArray_.clear();

		value_ = 0.;
		hasValue_ = false;
	}

	/**************************************************************************/
	/**
	 * Gives access to the object's value
	 *
	 * @return The value of the object
	 */
	double GParameterValuePair::value() {
		return value_;
	}

	/**************************************************************************/
	/**
	 * Indicates whether a value has been set.
	 *
	 * @return A boolean indicating whether a value has been set
	 */
	bool GParameterValuePair::hasValue() {
		return hasValue_;
	}

	/**************************************************************************/
	/**
	 * Checks whether any data is available locally
	 *
	 * @return A boolean indicating whether any data is available locally
	 */
	bool GParameterValuePair::hasData() {
		if(!dArray_.empty()) return true;
		if(!lArray_.empty()) return true;
		if(!bArray_.empty()) return true;
		if(!cArray_.empty()) return true;

		// Nothing found ...
		return false;
	}

	/**************************************************************************/
	/**
	 * Sets the precision of FP IO in ASCII mode
	 *
	 * @param The desired precision for FP IO
	 */
	void GParameterValuePair::setPrecision(const std::streamsize& precision) {
		std::vector<boost::shared_ptr<GDoubleParameter> >::iterator it;
		for(it=dArray_.begin(); it!=dArray_.end(); ++it) (*it)->setPrecision(precision);
	}

	/**************************************************************************/
	/**
	 * Writes the class'es data to a stream
	 *
	 * @param stream The external output stream to write to
	 */
	void GParameterValuePair::writeToStream(std::ostream& stream) const {
#ifdef DEBUG
		// Check that the stream is in a valid condition
		if(!stream.good()) {
			std::ostringstream error;
			error << "In GParameterValuePair::writeToStream(): Error!" << std::endl
				     << "Stream is in a bad condition. Leaving ..." << std::endl;

			throw(GDataExchangeException(error.str()));
		}
#endif /* DEBUG*/

		std::size_t dArraySize = dArray_.size();
		stream << dArraySize << std::endl;
		if(dArraySize) {
			std::vector<boost::shared_ptr<GDoubleParameter> >::const_iterator dcit;
			for(dcit=dArray_.begin(); dcit!=dArray_.end(); ++dcit) stream << **dcit; // std::endl provided by GDoubleParameter
		}

		std::size_t lArraySize = lArray_.size();
		stream << lArraySize << std::endl;
		if(lArraySize) {
			std::vector<boost::shared_ptr<GLongParameter> >::const_iterator lcit;
			for(lcit=lArray_.begin(); lcit!=lArray_.end(); ++lcit) stream << **lcit;
		}

		std::size_t bArraySize = bArray_.size();
		stream << bArraySize << std::endl;
		if(bArraySize) {
			std::vector<boost::shared_ptr<GBoolParameter> >::const_iterator bcit;
			for(bcit=bArray_.begin(); bcit!=bArray_.end(); ++bcit) stream << **bcit;
		}

		std::size_t cArraySize = cArray_.size();
		stream << cArraySize << std::endl;
		if(cArraySize) {
			std::vector<boost::shared_ptr<GCharParameter> >::const_iterator ccit;
			for(ccit=cArray_.begin(); ccit!=cArray_.end(); ++ccit) stream << **ccit;
		}

		stream << value_ << std::endl;
		stream << hasValue_ << std::endl;
	}

	/**************************************************************************/
	/**
	 * Reads the class'es data from a stream.
	 *
	 * Possible improvement: Do not clear the arrays but load the date into the
	 * existing objects, wherever possible.
	 *
	 * @param stream The external input stream to read from
	 */
	void GParameterValuePair::readFromStream(std::istream& stream) {
#ifdef DEBUG
		// Check that the stream is in a valid condition
		if(!stream.good()) {
			std::ostringstream error;
			error << "In GParameterValuePair::readFromStream(): Error!" << std::endl
				     << "Stream is in a bad condition. Leaving ..." << std::endl;

			throw(GDataExchangeException(error.str()));
		}
#endif /* DEBUG*/

		// Read in double data
		std::size_t file_dArraySize;
		stream >> file_dArraySize;
		std::size_t dArraySize = dArray_.size();

		std::vector<boost::shared_ptr<GDoubleParameter> >::iterator dit;
		if(file_dArraySize == dArraySize) { // The most likely case
			for(dit=dArray_.begin(); dit!=dArray_.end(); ++dit) (*dit)->readFromStream(stream);
		}
		else if(file_dArraySize > dArraySize) {
			for(dit=dArray_.begin(); dit!=dArray_.end(); ++dit) (*dit)->readFromStream(stream);

			for(std::size_t i=dArraySize; i<file_dArraySize; i++){
				boost::shared_ptr<GDoubleParameter> p(new GDoubleParameter());
				p->readFromStream(stream);
				dArray_.push_back(p);
			}
		}
		else if(file_dArraySize < dArraySize) {
			dArray_.resize(file_dArraySize); // Get rid of surplus items
			for(dit=dArray_.begin(); dit!=dArray_.end(); ++dit) (*dit)->readFromStream(stream);
		}

		// Read in long data
		std::size_t file_lArraySize;
		stream >> file_lArraySize;
		std::size_t lArraySize = lArray_.size();
		std::vector<boost::shared_ptr<GLongParameter> >::iterator lit;
		if(file_lArraySize == lArraySize) { // The most likely case
			for(lit=lArray_.begin(); lit!=lArray_.end(); ++lit) (*lit)->readFromStream(stream);
		}
		else if(file_lArraySize > lArraySize) {
			for(lit=lArray_.begin(); lit!=lArray_.end(); ++lit) (*lit)->readFromStream(stream);

			for(std::size_t i=lArraySize; i<file_lArraySize; i++){
				boost::shared_ptr<GLongParameter> p(new GLongParameter());
				p->readFromStream(stream);
				lArray_.push_back(p);
			}
		}
		else if(file_lArraySize < lArraySize) {
			lArray_.resize(file_lArraySize); // Get rid of surplus items
			for(lit=lArray_.begin(); lit!=lArray_.end(); ++lit) (*lit)->readFromStream(stream);
		}

		// Read in bool data
		std::size_t file_bArraySize;
		stream >> file_bArraySize;
		std::size_t bArraySize = bArray_.size();
		std::vector<boost::shared_ptr<GBoolParameter> >::iterator bit;
		if(file_bArraySize == bArraySize) { // The most likely case
			for(bit=bArray_.begin(); bit!=bArray_.end(); ++bit) (*bit)->readFromStream(stream);
		}
		else if(file_bArraySize > bArraySize) {
			for(bit=bArray_.begin(); bit!=bArray_.end(); ++bit) (*bit)->readFromStream(stream);

			for(std::size_t i=bArraySize; i<file_bArraySize; i++){
				boost::shared_ptr<GBoolParameter> p(new GBoolParameter());
				p->readFromStream(stream);
				bArray_.push_back(p);
			}
		}
		else if(file_bArraySize < bArraySize) {
			bArray_.resize(file_bArraySize); // Get rid of surplus items
			for(bit=bArray_.begin(); bit!=bArray_.end(); ++bit) (*bit)->readFromStream(stream);
		}

		// Read in char data
		std::size_t file_cArraySize;
		stream >> file_cArraySize;
		std::size_t cArraySize = cArray_.size();
		std::vector<boost::shared_ptr<GCharParameter> >::iterator cit;
		if(file_cArraySize == cArraySize) { // The most likely case
			for(cit=cArray_.begin(); cit!=cArray_.end(); ++cit) (*cit)->readFromStream(stream);
		}
		else if(file_cArraySize > cArraySize) {
			for(cit=cArray_.begin(); cit!=cArray_.end(); ++cit) (*cit)->readFromStream(stream);

			for(std::size_t i=cArraySize; i<file_cArraySize; i++){
				boost::shared_ptr<GCharParameter> p(new GCharParameter());
				p->readFromStream(stream);
				cArray_.push_back(p);
			}
		}
		else if(file_cArraySize < cArraySize) {
			cArray_.resize(file_cArraySize); // Get rid of surplus items
			for(cit=cArray_.begin(); cit!=cArray_.end(); ++cit) (*cit)->readFromStream(stream);
		}

		// Finally read the object's value and the flag which indicates, whether the value is valid
		stream >> value_;
		stream >> hasValue_;
	}

	/**************************************************************************/
	/**
	 * Writes the object's data to a stream in binary mode.
	 *
	 * @param stream The external output stream to write to
	 */
	void GParameterValuePair::binaryWriteToStream(std::ostream& stream) const {
#ifdef DEBUG
		// Check that the stream is in a valid condition
		if(!stream.good()) {
			std::ostringstream error;
			error << "In GParameterValuePair::binaryWriteToStream(): Error!" << std::endl
				     << "Stream is in a bad condition. Leaving ..." << std::endl;

			throw(GDataExchangeException(error.str()));
		}
#endif /* DEBUG*/

		// Write the double data out in binary mode
		std::size_t dArraySize = dArray_.size();
		stream.write(reinterpret_cast<const char *>(&dArraySize), sizeof(dArraySize));
		if(dArraySize) {
			std::vector<boost::shared_ptr<GDoubleParameter> >::const_iterator dcit;
			for(dcit=dArray_.begin(); dcit!=dArray_.end(); ++dcit) (*dcit)->binaryWriteToStream(stream);
		}

		// Write the long data out in binary mode
		std::size_t lArraySize = lArray_.size();
		stream.write(reinterpret_cast<const char *>(&lArraySize), sizeof(lArraySize));
		if(lArraySize) {
			std::vector<boost::shared_ptr<GLongParameter> >::const_iterator lcit;
			for(lcit=lArray_.begin(); lcit!=lArray_.end(); ++lcit) (*lcit)->binaryWriteToStream(stream);
		}

		// Write the bool data out in binary mode
		std::size_t bArraySize = bArray_.size();
		stream.write(reinterpret_cast<const char *>(&bArraySize), sizeof(bArraySize));
		if(bArraySize) {
			std::vector<boost::shared_ptr<GBoolParameter> >::const_iterator bcit;
			for(bcit=bArray_.begin(); bcit!=bArray_.end(); ++bcit) (*bcit)->binaryWriteToStream(stream);
		}

		// Write the char data out in binary mode
		std::size_t cArraySize = cArray_.size();
		stream.write(reinterpret_cast<const char *>(&cArraySize), sizeof(cArraySize));
		if(cArraySize) {
			std::vector<boost::shared_ptr<GCharParameter> >::const_iterator ccit;
			for(ccit=cArray_.begin(); ccit!=cArray_.end(); ++ccit) (*ccit)->binaryWriteToStream(stream);
		}

		stream.write(reinterpret_cast<const char *>(&value_), sizeof(&value_));
		stream.write(reinterpret_cast<const char *>(&hasValue_), sizeof(&hasValue_));
	}

	/**************************************************************************/
	/**
	 * Reads the class'es data from a stream in binary mode.
	 *
	 * Possible improvement: Do not clear the arrays but load the date into the
	 * existing objects, wherever possible.
	 *
	 * @param stream The external input stream to read the binary data from
	 */
	void GParameterValuePair::binaryReadFromStream(std::istream& stream) {
#ifdef DEBUG
		// Check that the stream is in a valid condition
		if(!stream.good()) {
			std::ostringstream error;
			error << "In GParameterValuePair::binaryReadFromStream(): Error!" << std::endl
				     << "Stream is in a bad condition. Leaving ..." << std::endl;

			throw(GDataExchangeException(error.str()));
		}
#endif /* DEBUG*/

		// Read in double data
		std::size_t file_dArraySize;
		stream.read(reinterpret_cast<char *>(&file_dArraySize), sizeof(file_dArraySize));
		std::size_t dArraySize = dArray_.size();
		std::vector<boost::shared_ptr<GDoubleParameter> >::iterator dit;
		if(file_dArraySize == dArraySize) { // The most likely case
			for(dit=dArray_.begin(); dit!=dArray_.end(); ++dit) (*dit)->binaryReadFromStream(stream);
		}
		else if(file_dArraySize > dArraySize) {
			for(dit=dArray_.begin(); dit!=dArray_.end(); ++dit) (*dit)->binaryReadFromStream(stream);

			for(std::size_t i=dArraySize; i<file_dArraySize; i++){
				boost::shared_ptr<GDoubleParameter> p(new GDoubleParameter());
				p->binaryReadFromStream(stream);
				dArray_.push_back(p);
			}
		}
		else if(file_dArraySize < dArraySize) {
			dArray_.resize(file_dArraySize); // Get rid of surplus items
			for(dit=dArray_.begin(); dit!=dArray_.end(); ++dit) (*dit)->binaryReadFromStream(stream);
		}

		// Read in long data
		std::size_t file_lArraySize;
		stream.read(reinterpret_cast<char *>(&file_lArraySize), sizeof(file_lArraySize));
		std::size_t lArraySize = lArray_.size();
		std::vector<boost::shared_ptr<GLongParameter> >::iterator lit;
		if(file_lArraySize == lArraySize) { // The most likely case
			for(lit=lArray_.begin(); lit!=lArray_.end(); ++lit) (*lit)->binaryReadFromStream(stream);
		}
		else if(file_lArraySize > lArraySize) {
			for(lit=lArray_.begin(); lit!=lArray_.end(); ++lit) (*lit)->binaryReadFromStream(stream);

			for(std::size_t i=lArraySize; i<file_lArraySize; i++){
				boost::shared_ptr<GLongParameter> p(new GLongParameter());
				p->binaryReadFromStream(stream);
				lArray_.push_back(p);
			}
		}
		else if(file_lArraySize < lArraySize) {
			lArray_.resize(file_lArraySize); // Get rid of surplus items
			for(lit=lArray_.begin(); lit!=lArray_.end(); ++lit) (*lit)->binaryReadFromStream(stream);
		}

		// Read in bool data
		std::size_t file_bArraySize;
		stream.read(reinterpret_cast<char *>(&file_bArraySize), sizeof(file_bArraySize));
		std::size_t bArraySize = bArray_.size();
		std::vector<boost::shared_ptr<GBoolParameter> >::iterator bit;
		if(file_bArraySize == bArraySize) { // The most likely case
			for(bit=bArray_.begin(); bit!=bArray_.end(); ++bit) (*bit)->binaryReadFromStream(stream);
		}
		else if(file_bArraySize > bArraySize) {
			for(bit=bArray_.begin(); bit!=bArray_.end(); ++bit) (*bit)->binaryReadFromStream(stream);

			for(std::size_t i=bArraySize; i<file_bArraySize; i++){
				boost::shared_ptr<GBoolParameter> p(new GBoolParameter());
				p->binaryReadFromStream(stream);
				bArray_.push_back(p);
			}
		}
		else if(file_bArraySize < bArraySize) {
			bArray_.resize(file_bArraySize); // Get rid of surplus items
			for(bit=bArray_.begin(); bit!=bArray_.end(); ++bit) (*bit)->binaryReadFromStream(stream);
		}

		// Read in char data
		std::size_t file_cArraySize;
		stream.read(reinterpret_cast<char *>(&file_cArraySize), sizeof(file_cArraySize));
		std::size_t cArraySize = cArray_.size();
		std::vector<boost::shared_ptr<GCharParameter> >::iterator cit;
		if(file_cArraySize == cArraySize) { // The most likely case
			for(cit=cArray_.begin(); cit!=cArray_.end(); ++cit) (*cit)->binaryReadFromStream(stream);
		}
		else if(file_cArraySize > cArraySize) {
			for(cit=cArray_.begin(); cit!=cArray_.end(); ++cit) (*cit)->binaryReadFromStream(stream);

			for(std::size_t i=cArraySize; i<file_cArraySize; i++){
				boost::shared_ptr<GCharParameter> p(new GCharParameter());
				p->binaryReadFromStream(stream);
				cArray_.push_back(p);
			}
		}
		else if(file_cArraySize < cArraySize) {
			cArray_.resize(file_cArraySize); // Get rid of surplus items
			for(cit=cArray_.begin(); cit!=cArray_.end(); ++cit) (*cit)->binaryReadFromStream(stream);
		}

		// Finally read the value and the flag indicating whether it is valid
		stream.read(reinterpret_cast<char *>(&value_), sizeof(&value_));
		stream.read(reinterpret_cast<char *>(&hasValue_), sizeof(&hasValue_));
	}

	/******************************************************************************/

	} /* namespace Util */
} /* namespace Gem */
