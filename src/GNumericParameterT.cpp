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
 * an undesirable value.
 *
 * @param stream The output stream the double values should be written to
 */
template<> void GNumericParameterT<double>::writeToStream(std::ostream& stream) const {
#ifdef DEBUG
		// Check that the stream is in a valid condition
		if(!stream.good()) {
			std::cerr << "In GNumericParameterT<double>::writeToStream(): Error!" << std::endl
				          << "Stream is in a bad condition. Leaving ..." << std::endl;
			exit(1);
		}
#endif /* DEBUG*/

		// Retrieve the current precision
		std::streamsize precisionStore = stream.precision();

		// Now set the output precision to the desired value
		stream.precision(precision_);

		stream << param_ << std::endl
		            << lowerBoundary_ << std::endl
		            << upperBoundary_ << std::endl;

		// Restore the original value
		stream.precision(precisionStore);
}

/***********************************************************************************************/
/**
 * Writes char parameters to a stream, including their boundaries. This specialization
 * is needed so we can store characters as numbers.
 *
  * @param stream The output stream the char values should be written to
 */
template<> void GNumericParameterT<char>::writeToStream(std::ostream& stream) const {
#ifdef DEBUG
		// Check that the stream is in a valid condition
		if(!stream.good()) {
			std::cerr << "In GNumericParameterT<char>::writeToStream(): Error!" << std::endl
				          << "Stream is in a bad condition. Leaving ..." << std::endl;
			exit(1);
		}
#endif /* DEBUG*/

		// Store all characters as numbers, so they are more easily recognizable in the output file
		std::cout << param_ << " " << lowerBoundary_ << " " << upperBoundary_ << std::endl;
		stream << boost::lexical_cast<short>(param_) << std::endl
		            << boost::lexical_cast<short>(lowerBoundary_) << std::endl
		            << boost::lexical_cast<short>(upperBoundary_) << std::endl;
}

/***********************************************************************************************/

template<> void GNumericParameterT<char>::readFromStream(std::istream& stream) {
#ifdef DEBUG
		// Check that the stream is in a valid condition
		if(!stream.good()) {
			std::cerr << "In GNumericParameterT<char>::readFromStream(): Error!" << std::endl
				          << "Stream is in a bad condition. Leaving ..." << std::endl;
			exit(1);
		}
#endif /* DEBUG*/

		unsigned short int_param_;
		unsigned short int_lowerBoundary_;
		unsigned short int_upperBoundary_;

		stream >> int_param_;
		stream >> int_lowerBoundary_;
		stream >> int_upperBoundary_;

		param_ = boost::lexical_cast<char>(int_param_);
		lowerBoundary_ = boost::lexical_cast<char>(int_lowerBoundary_);
		upperBoundary_ = boost::lexical_cast<char>(int_upperBoundary_);
}

} /* namespace Util */
} /* namespace Gem */
