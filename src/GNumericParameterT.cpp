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
 */
template<> void GNumericParameterT<double>::writeToStream(std::ostream& stream) const {
#ifdef DEBUG
		// Check that the stream is in a valid condition
		if(!stream.good()) {
			std::cerr << "In GNumericParameterT::writeToStream(): Error!" << std::endl
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

} /* namespace Util */
} /* namespace Gem */
