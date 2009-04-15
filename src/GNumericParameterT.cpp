/**
 * @file GNumericParameterT.cpp
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
 * OF THE GENEVA LIBRARY, UNLESS THIS IS EXPLICITLY STATED IN THE CORRESPONDING FILE.
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

#include "GNumericParameterT.hpp"

namespace Gem
{
namespace Util
{

/***********************************************************************************************/
/**
 * A trap designed to catch attempts to use this class with types it was
 * not designed for. If not implemented, compilation will fail. Specialization
 * for double.
 *
 * @param unknown A dummy parameter
 */
template<>
double GNumericParameterT<double>::unknownParameterTypeTrap(double unknown) {
	return unknown; // Make the compiler happy
}

/***********************************************************************************************/
/**
 * A trap designed to catch attempts to use this class with types it was
 * not designed for. If not implemented, compilation will fail. Specialization
 * for boost::int32_t.
 *
 * @param unknown A dummy parameter
 */
template<>
boost::int32_t GNumericParameterT<boost::int32_t>::unknownParameterTypeTrap(boost::int32_t unknown) {
	return unknown; // Make the compiler happy
}

/***********************************************************************************************/
/**
 * A trap designed to catch attempts to use this class with types it was
 * not designed for. If not implemented, compilation will fail. Specialization
 * for char.
 *
 * @param unknown A dummy parameter
 */
template<>
char GNumericParameterT<char>::unknownParameterTypeTrap(char unknown) {
	return unknown; // Make the compiler happy
}

/***********************************************************************************************/
/**
 * A trap designed to catch attempts to use this class with types it was
 * not designed for. If not implemented, compilation will fail. Specialization
 * for bool.
 *
 * @param unknown A dummy parameter
 */
template<>
bool GNumericParameterT<bool>::unknownParameterTypeTrap(bool unknown) {
	return unknown; // Make the compiler happy
}

/***********************************************************************************************/
/**
 * Writes a double parameter to a stream, including its boundaries. This specialization
 * is needed as the precision of the output of floating point variables might be set to
 * an undesirable value. The precision is part of the output format.
 *
 * @param stream The output stream the double values should be written to
 */
template<> void GNumericParameterT<double>::writeToStream(std::ostream& stream) const {
#ifdef DEBUG
		// Check that the stream is in a valid condition
		if(!stream.good()) {
			std::ostringstream error;
			error << "In GNumericParameterT<double>::writeToStream(): Error!" << std::endl
			         << "Stream is in a bad condition. Leaving ..." << std::endl;

			throw(GDataExchangeException(error.str()));
		}
#endif /* DEBUG*/

		// Back up the current precision
		std::streamsize precisionStore = stream.precision();

		// We need to be able to restore the required precision later on
		stream << precision_ << std::endl;

		// Now set the output precision to the desired value
		stream.precision(precision_);

		stream << param_ << std::endl
		            << lowerBoundary_ << std::endl
		            << upperBoundary_ << std::endl;

		// Restore the original precision
		stream.precision(precisionStore);
}

/***********************************************************************************************/
/**
 * Reads a double parameter from a stream, including its boundaries. This specialization
 * is needed as the precision of the output of floating point variables might be set to
 * an undesirable value. The precision is part of the data format.
 *
 * @param stream The input stream the double values should be read from
 */
template<> void GNumericParameterT<double>::readFromStream(std::istream& stream) {
#ifdef DEBUG
		// Check that the stream is in a valid condition
		if(!stream.good()) {
			std::ostringstream error;
			error << "In GNumericParameterT<double>::readFromStream(): Error!" << std::endl
			         << "Stream is in a bad condition. Leaving ..." << std::endl;

			throw(GDataExchangeException(error.str()));
		}
#endif /* DEBUG*/

		// Back up the current precision
		std::streamsize precisionStore = stream.precision();

		// Retrieve the stored precision and set our own precision accordingly for the time being
		stream >> precision_;
		stream.precision(precision_);

		// Retrieve the parameter and its boundaries
		stream >> param_;
		stream >> lowerBoundary_;
		stream >> upperBoundary_;

		// Restore the original precision
		stream.precision(precisionStore);
}

/***********************************************************************************************/
/**
 * Writes a double parameter to a stream in binary mode, including its boundaries.
 * This specialization is needed so the precision of FP IO gets stored.
 *
 * @param stream The output stream the double values should be written to
 */
template<> void GNumericParameterT<double>::binaryWriteToStream(std::ostream& stream) const {
#ifdef DEBUG
		// Check that the stream is in a valid condition
		if(!stream.good()) {
			std::ostringstream error;
			std::cerr << "In GNumericParameterT<double>::binaryWriteToStream(): Error!" << std::endl
				          << "Stream is in a bad condition. Leaving ..." << std::endl;

			throw(GDataExchangeException(error.str()));
		}
#endif /* DEBUG*/

		// Write the data out in binary mode, including the precision_ variable.
		stream.write(reinterpret_cast<const char *>(&precision_), sizeof(precision_));
		stream.write(reinterpret_cast<const char *>(&param_), sizeof(param_));
		stream.write(reinterpret_cast<const char *>(&lowerBoundary_), sizeof(lowerBoundary_));
		stream.write(reinterpret_cast<const char *>(&upperBoundary_), sizeof(upperBoundary_));
}

/***********************************************************************************************/
/**
 * Reads a double parameter from a stream in binary mode, including its boundaries.
 * This specialization so the precision_ variable gets restored.
 *
 * @param stream The input stream the double values should be read from
 */
template<> void GNumericParameterT<double>::binaryReadFromStream(std::istream& stream) {
#ifdef DEBUG
		// Check that the stream is in a valid condition
		if(!stream.good()) {
			std::ostringstream error;
			std::cerr << "In GNumericParameterT<double>::binaryReadFromStream(): Error!" << std::endl
				          << "Stream is in a bad condition. Leaving ..." << std::endl;

			throw(GDataExchangeException(error.str()));
		}
#endif /* DEBUG*/

		// Read data from the stream in binary mode
		stream.read(reinterpret_cast<char *>(&precision_), sizeof(precision_));
		stream.read(reinterpret_cast<char *>(&param_), sizeof(param_));
		stream.read(reinterpret_cast<char *>(&lowerBoundary_), sizeof(lowerBoundary_));
		stream.read(reinterpret_cast<char *>(&upperBoundary_), sizeof(upperBoundary_));
}

/***********************************************************************************************/
/**
 * Writes a bool parameter to a stream. This specialization is needed as
 * boundaries do not have to be written out for bool parameters.
 *
 * @param stream The output stream the bool value should be written to
 */
template<> void GNumericParameterT<bool>::writeToStream(std::ostream& stream) const {
#ifdef DEBUG
		// Check that the stream is in a valid condition
		if(!stream.good()) {
			std::ostringstream error;
			error << "In GNumericParameterT<bool>::writeToStream(): Error!" << std::endl
			         << "Stream is in a bad condition. Leaving ..." << std::endl;

			throw(GDataExchangeException(error.str()));
		}
#endif /* DEBUG*/

		stream << param_ << std::endl;
}

/***********************************************************************************************/
/**
 * Reads a bool parameter from a stream. This specialization is needed as
 * boundaries do not get stored for bool parameters.
 *
 * @param stream The input stream the bool value should be read from
 */
template<> void GNumericParameterT<bool>::readFromStream(std::istream& stream) {
#ifdef DEBUG
		// Check that the stream is in a valid condition
		if(!stream.good()) {
			std::ostringstream error;
			error << "In GNumericParameterT<bool>::readFromStream(): Error!" << std::endl
			         << "Stream is in a bad condition. Leaving ..." << std::endl;

			throw(GDataExchangeException(error.str()));
		}
#endif /* DEBUG*/

		// Retrieve the parameter
		stream >> param_;

		// Make sure the boundaries are in a sane state
		lowerBoundary_ = false;
		upperBoundary_ = true;
}

/***********************************************************************************************/
/**
 * Writes a bool parameter to a stream in binary mode. This specialization is
 * needed as no boundaries need to get stored for boolean parameters.
 *
 * @param stream The output stream the bool value should be written to
 */
template<> void GNumericParameterT<bool>::binaryWriteToStream(std::ostream& stream) const {
#ifdef DEBUG
		// Check that the stream is in a valid condition
		if(!stream.good()) {
			std::ostringstream error;
			error << "In GNumericParameterT<bool>::binaryWriteToStream(): Error!" << std::endl
			         << "Stream is in a bad condition. Leaving ..." << std::endl;

			throw(GDataExchangeException(error.str()));
		}
#endif /* DEBUG*/

		// Write the data out in binary mode
		stream.write(reinterpret_cast<const char *>(&param_), sizeof(param_));
}

/***********************************************************************************************/
/**
 * Reads a bool parameter from a stream in binary mode. This specialization is
 * needed as no boundaries need to get stored for boolean parameters.
 *
 * @param stream The input stream the bool value should be read from
 */
template<> void GNumericParameterT<bool>::binaryReadFromStream(std::istream& stream) {
#ifdef DEBUG
		// Check that the stream is in a valid condition
		if(!stream.good()) {
			std::ostringstream error;
			error << "In GNumericParameterT<bool>::binaryReadFromStream(): Error!" << std::endl
			         << "Stream is in a bad condition. Leaving ..." << std::endl;

			throw(GDataExchangeException(error.str()));
		}
#endif /* DEBUG*/

		// Read data from the stream in binary mode
		stream.read(reinterpret_cast<char *>(&param_), sizeof(param_));

		// Make sure the boundaries are in a sane state
		lowerBoundary_ = false;
		upperBoundary_ = true;
}

/***********************************************************************************************/
/**
 * Writes a char parameter to a stream. This specialization is needed as
 * we want to write out characters as numbers in ASCII format.
 *
 * @param stream The output stream the char value should be written to
 */
template<> void GNumericParameterT<char>::writeToStream(std::ostream& stream) const {
#ifdef DEBUG
		// Check that the stream is in a valid condition
		if(!stream.good()) {
			std::ostringstream error;
			error << "In GNumericParameterT<char>::writeToStream(): Error!" << std::endl
			         << "Stream is in a bad condition. Leaving ..." << std::endl;

			throw(GDataExchangeException(error.str()));
		}
#endif /* DEBUG*/

		stream << static_cast<boost::uint16_t>(param_) << std::endl
		            << static_cast<boost::uint16_t>(lowerBoundary_) << std::endl
		            << static_cast<boost::uint16_t>(upperBoundary_) << std::endl;
}

/***********************************************************************************************/
/**
 * Reads in char parameters from a stream. This specialization is needed as
 * characters are stored as numbers..
 *
 * @param stream The output stream the char value should be read from
 */
template<> void GNumericParameterT<char>::readFromStream(std::istream& stream) {
#ifdef DEBUG
		// Check that the stream is in a valid condition
		if(!stream.good()) {
			std::ostringstream error;
			error << "In GNumericParameterT<char>::readFromStream(): Error!" << std::endl
			         << "Stream is in a bad condition. Leaving ..." << std::endl;

			throw(GDataExchangeException(error.str()));
		}
#endif /* DEBUG*/

		boost::uint16_t uint_param, uint_lowerBoundary, uint_upperBoundary;

		// Read parameters into integers
		stream >> uint_param;
		stream >> uint_lowerBoundary;
		stream >>  uint_upperBoundary;

		// And convert the integers to characters
		param_ = static_cast<char>(uint_param);
		lowerBoundary_ = static_cast<char>(uint_lowerBoundary);
		upperBoundary_ = static_cast<char>(uint_upperBoundary);
}

/***********************************************************************************************/
/**
 * Checks for similarity between the two objects. If the limit is set to 0, then
 * "similarity" means "equality".
 *
 * @param cp A constant reference to another GNumericParameterT<double> object
 * @param limit The maximum acceptable level of difference between two double
 * @return A boolean indicating whether both objects are similar to each other
 */
template <> bool GNumericParameterT<double>::isSimilarTo(const GNumericParameterT<double>& cp, const double& limit) const {
	if(limit == 0.) return this->operator==(cp);

	bool result = true;

	if(fabs(param_ - cp.param_) > fabs(limit)) result = false;
	else if(fabs(lowerBoundary_ - cp.lowerBoundary_) > fabs(limit)) result = false;
	else if(fabs(upperBoundary_ - cp.upperBoundary_) > fabs(limit)) result = false;
	else if(precision_ != cp.precision_) result=false;

	return result;
}

/***********************************************************************************************/
/**
 * Specialization of the default constructor for typeof(T)==bool. It is needed
 * as boolean values always have a lower and upper boundary.
 */
template <> GNumericParameterT<bool>::GNumericParameterT()
	:param_(false),
	 lowerBoundary_(false),
	 upperBoundary_(true),
	 precision_(DEFAULTPRECISION)
{ /* nothing */ }

/***********************************************************************************************/
/**
 * Specialization of the default constructor for typeof(T)==bool. It is needed
 * as boolean values always have a lower and upper boundary.
 */
template <> GNumericParameterT<bool>::GNumericParameterT(const bool& param)
	:param_(param),
	 lowerBoundary_(false),
	 upperBoundary_(true),
	 precision_(DEFAULTPRECISION)
{ /* nothing */ }

/***********************************************************************************************/
/**
 * Specialization of the default constructor for typeof(T)==bool. It is needed
 * as boolean values always have a lower and upper boundary.
 */
template <> GNumericParameterT<bool>::GNumericParameterT(const bool& param, const bool&, const bool&)
	:param_(param),
	 lowerBoundary_(false),
	 upperBoundary_(true),
	 precision_(DEFAULTPRECISION)
{ /* nothing */ }

/***********************************************************************************************/
/**
 * Specialization of the default constructor for typeof(T)==bool. It is needed
 * as boolean values always have a lower and upper boundary.
 */
template <> GNumericParameterT<bool>::GNumericParameterT(const GNumericParameterT<bool>& cp)
	:param_(cp.param_),
	 lowerBoundary_(false),
	 upperBoundary_(true),
     precision_(cp.precision_)
{ /* nothing */ }

/***********************************************************************************************/

template <> void GNumericParameterT<bool>::setParameter(const bool& param, const bool&, const bool&) {
	param_ = param;
	lowerBoundary_ = false;
	upperBoundary_ = true;
}

/***********************************************************************************************/

} /* namespace Util */
} /* namespace Gem */
