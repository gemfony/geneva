/**
 * @file GNumericParameterT.cpp
 */

/* Copyright (C) 2009 Dr. Ruediger Berlich and Karlsruhe Institute of Technology
 * (University of the State of Baden-Wuerttemberg and National Laboratory
 * of the Helmholtz Association)
 *
 * Contact: info [at] gemfony (dot) com
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
 *
 * For further information on Gemfony scientific and Geneva, visit
 * http://www.gemfony.com .
 */

#include "dataexchange/GNumericParameterT.hpp"

namespace Gem
{
namespace Dataexchange
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

			throw(Gem::Common::gemfony_error_condition(error.str()));
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

			throw(Gem::Common::gemfony_error_condition(error.str()));
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

			throw(Gem::Common::gemfony_error_condition(error.str()));
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

			throw(Gem::Common::gemfony_error_condition(error.str()));
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
 * Specialization of the default constructor for typeof(T)==bool. It is needed
 * as boolean values always have a lower and upper boundary.
 */
template <> GNumericParameterT<bool>::GNumericParameterT(void)
	:param_(false),
	 lowerBoundary_(false),
	 upperBoundary_(true)
{ /* nothing */ }

/***********************************************************************************************/
/**
 * Specialization of the default constructor for typeof(T)==bool. It is needed
 * as boolean values always have a lower and upper boundary.
 */
template <> GNumericParameterT<bool>::GNumericParameterT(const bool& param)
	:param_(param),
	 lowerBoundary_(false),
	 upperBoundary_(true)
{ /* nothing */ }

/***********************************************************************************************/
/**
 * Specialization of the default constructor for typeof(T)==bool. It is needed
 * as boolean values always have a lower and upper boundary.
 */
template <> GNumericParameterT<bool>::GNumericParameterT(const bool& param, const bool&, const bool&)
	:param_(param),
	 lowerBoundary_(false),
	 upperBoundary_(true)
{ /* nothing */ }

/***********************************************************************************************/
/**
 * Specialization of the default constructor for typeof(T)==bool. It is needed
 * as boolean values always have a lower and upper boundary.
 */
template <> GNumericParameterT<bool>::GNumericParameterT(const GNumericParameterT<bool>& cp)
	:param_(cp.param_),
	 lowerBoundary_(false),
	 upperBoundary_(true)
{ /* nothing */ }

/***********************************************************************************************/

template <> void GNumericParameterT<bool>::setParameter(const bool& param, const bool&, const bool&) {
	param_ = param;
	lowerBoundary_ = false;
	upperBoundary_ = true;
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

	return result;
}

/***********************************************************************************************/

} /* namespace Dataexchange */
} /* namespace Gem */
