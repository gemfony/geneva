/**
 * @file GSerializationHelperFunctions.cpp
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

#include "GSerializationHelperFunctions.hpp"

namespace Gem
{
namespace GenEvA
{

/*********************************************************************/
/**
 * Converts a shared_ptr<GIndividual> into its string representation.
 *
 * @param gi_ptr A shared_ptr to the GIndividual to be serialized
 * @param sm The corresponding serialization mode
 * @return A string representation of gi_ptr
 */
std::string indptrToString(boost::shared_ptr<GIndividual> gi_ptr, const serializationMode& serMod){
	std::ostringstream oarchive_stream;

	switch(serMod){
	case TEXTSERIALIZATION:
		{
			boost::archive::text_oarchive oa(oarchive_stream);
			oa << boost::serialization::make_nvp("classHierarchyFromGIndividual_ptr", gi_ptr);
		} // note: explicit scope here is essential so the oa-destructor gets called

		break;

	case XMLSERIALIZATION:
		{
			boost::archive::xml_oarchive oa(oarchive_stream);
			oa << boost::serialization::make_nvp("classHierarchyFromGIndividual_ptr", gi_ptr);
		}
		break;

	case BINARYSERIALIZATION:
		{
			boost::archive::binary_oarchive oa(oarchive_stream);
			oa << boost::serialization::make_nvp("classHierarchyFromGIndividual_ptr", gi_ptr);
		}

		break;
	}

	return oarchive_stream.str();
}

/*********************************************************************/
/**
 * Loads a shared_ptr<GIndividual> from its string representation.
 *
 * @param gi_string A string representation of the individual to be restored
 * @param sm The corresponding serialization mode
 * @return A shared_ptr to the restored GIndividual object
 */
boost::shared_ptr<GIndividual> indptrFromString(const std::string& gi_string, const serializationMode& serMod){
	std::istringstream istr(gi_string);
	boost::shared_ptr<GIndividual> gi_ptr;

	switch(serMod){
	case TEXTSERIALIZATION:
		{
			boost::archive::text_iarchive ia(istr);
			ia >> boost::serialization::make_nvp("classHierarchyFromGIndividual_ptr", gi_ptr);
		} // note: explicit scope here is essential so the ia-destructor gets called

		break;

	case XMLSERIALIZATION:
		{
			boost::archive::xml_iarchive ia(istr);
			ia >> boost::serialization::make_nvp("classHierarchyFromGIndividual_ptr", gi_ptr);
		}

		break;

	case BINARYSERIALIZATION:
		{
			boost::archive::binary_iarchive ia(istr);
			ia >> boost::serialization::make_nvp("classHierarchyFromGIndividual_ptr", gi_ptr);
		}
		break;
	}

	return gi_ptr;
}

/*********************************************************************/
/**
 * Puts a Gem::GenEvA::serializationMode item into a stream
 *
 * @param o The ostream the item should be added to
 * @param serMode the item to be added to the stream
 * @return The std::ostream object used to add the item to
 */
std::ostream& operator<<(std::ostream& o, const Gem::GenEvA::serializationMode& serMode){
	boost::uint16_t tmp = static_cast<boost::uint16_t>(serMode);
	o << tmp;
	return o;
}

/*********************************************************************/
/**
 * Reads a Gem::GenEvA::serializationMode item from a stream
 *
 * @param i The stream the item should be read from
 * @param serMode The item read from the stream
 * @return The std::istream object used to read the item from
 */
std::istream& operator>>(std::istream& i, Gem::GenEvA::serializationMode& serMode){
	boost::uint16_t tmp;
	i >> tmp;

#ifdef DEBUG
	serMode = boost::numeric_cast<Gem::GenEvA::serializationMode>(tmp);
#else
	serMode = static_cast<Gem::GenEvA::serializationMode>(tmp);
#endif  /* DEBUG */

	return i;
}

/*********************************************************************/
/**
 * Puts a Gem::GenEvA::dataExchangeMode item into a stream
 *
 * @param o The ostream the item should be added to
 * @param exchMode the item to be added to the stream
 * @return The std::ostream object used to add the item to
 */
std::ostream& operator<<(std::ostream& o, const Gem::GenEvA::dataExchangeMode& exchMode){
	boost::uint16_t tmp = static_cast<boost::uint16_t>(exchMode);
	o << tmp;
	return o;
}

/*********************************************************************/
/**
 * Reads a Gem::GenEvA::dataExchangeMode item from a stream
 *
 * @param i The stream the item should be read from
 * @param exchMode The item read from the stream
 * @return The std::istream object used to read the item from
 */
std::istream& operator>>(std::istream& i, Gem::GenEvA::dataExchangeMode& exchMode){
	boost::uint16_t tmp;
	i >> tmp;

#ifdef DEBUG
	exchMode = boost::numeric_cast<Gem::GenEvA::dataExchangeMode>(tmp);
#else
	exchMode = static_cast<Gem::GenEvA::dataExchangeMode>(tmp);
#endif /* DEBUG */

	return i;
}

/*********************************************************************/
/**
 * Puts a Gem::GenEvA::recoScheme item into a stream
 *
 * @param o The ostream the item should be added to
 * @param rc the item to be added to the stream
 * @return The std::ostream object used to add the item to
 */
std::ostream& operator<<(std::ostream& o, const Gem::GenEvA::recoScheme& rc){
	boost::uint16_t tmp = static_cast<boost::uint16_t>(rc);
	o << tmp;
	return o;
}

/*********************************************************************/
/**
 * Reads a Gem::GenEvA::recoScheme item from a stream
 *
 * @param i The stream the item should be read from
 * @param rc The item read from the stream
 * @return The std::istream object used to read the item from
 */
std::istream& operator>>(std::istream& i, Gem::GenEvA::recoScheme& rc){
	boost::uint16_t tmp;
	i >> tmp;

#ifdef DEBUG
	rc = boost::numeric_cast<Gem::GenEvA::recoScheme>(tmp);
#else
	rc = static_cast<Gem::GenEvA::recoScheme>(tmp);
#endif /* DEBUG */

	return i;
}

/*********************************************************************/
/**
 * Puts a Gem::GenEvA::infoMode item into a stream
 *
 * @param o The ostream the item should be added to
 * @param im the item to be added to the stream
 * @return The std::ostream object used to add the item to
 */
std::ostream& operator<<(std::ostream& o, const Gem::GenEvA::infoMode& im){
	boost::uint16_t tmp = static_cast<boost::uint16_t>(im);
	o << tmp;
	return o;
}

/*********************************************************************/
/**
 * Reads a Gem::GenEvA::infoMode item from a stream
 *
 * @param i The stream the item should be read from
 * @param im The item read from the stream
 * @return The std::istream object used to read the item from
 */
std::istream& operator>>(std::istream& i, Gem::GenEvA::infoMode& im){
	boost::uint16_t tmp;
	i >> tmp;

#ifdef DEBUG
	im = boost::numeric_cast<Gem::GenEvA::infoMode>(tmp);
#else
	im = static_cast<Gem::GenEvA::infoMode>(tmp);
#endif /* DEBUG */

	return i;
}

/*********************************************************************/
/**
 * Puts a Gem::GenEvA::adaptorId item into a stream
 *
 * @param o The ostream the item should be added to
 * @param aid the item to be added to the stream
 * @return The std::ostream object used to add the item to
 */
std::ostream& operator<<(std::ostream& o, const Gem::GenEvA::adaptorId& aid){
	boost::uint16_t tmp = static_cast<boost::uint16_t>(aid);
	o << tmp;
	return o;
}

/*********************************************************************/
/**
 * Reads a Gem::GenEvA::adaptorId item from a stream
 *
 * @param i The stream the item should be read from
 * @param aid The item read from the stream
 * @return The std::istream object used to read the item from
 */
std::istream& operator>>(std::istream& i, Gem::GenEvA::adaptorId& aid){
	boost::uint16_t tmp;
	i >> tmp;

#ifdef DEBUG
	aid = boost::numeric_cast<Gem::GenEvA::adaptorId>(tmp);
#else
	aid = static_cast<Gem::GenEvA::adaptorId>(tmp);
#endif /* DEBUG */

	return i;
}

/*********************************************************************/
/**
 * Puts a Gem::GenEvA::sortingMode item into a stream
 *
 * @param o The ostream the item should be added to
 * @param smode the item to be added to the stream
 * @return The std::ostream object used to add the item to
 */
std::ostream& operator<<(std::ostream& o, const Gem::GenEvA::sortingMode& smode){
	boost::uint16_t tmp = static_cast<boost::uint16_t>(smode);
	o << tmp;
	return o;
}

/*********************************************************************/
/**
 * Reads a Gem::GenEvA::sortingMode item from a stream
 *
 * @param i The stream the item should be read from
 * @param smode The item read from the stream
 * @return The std::istream object used to read the item from
 */
std::istream& operator>>(std::istream& i, Gem::GenEvA::sortingMode& smode){
	boost::uint16_t tmp;
	i >> tmp;

#ifdef DEBUG
	smode = boost::numeric_cast<Gem::GenEvA::sortingMode>(tmp);
#else
	smode = static_cast<Gem::GenEvA::sortingMode>(tmp);
#endif /* DEBUG */

	return i;
}

/*********************************************************************/

} /* namespace GenEvA */

namespace Util {

/*********************************************************************/
/**
 * Puts a Gem::Util::rnrGenerationMode item into a stream
 *
 * @param o The ostream the item should be added to
 * @param rnrgen the item to be added to the stream
 * @return The std::ostream object used to add the item to
 */
std::ostream& operator<<(std::ostream& o, const Gem::Util::rnrGenerationMode& rnrgen){
	boost::uint16_t tmp = static_cast<boost::uint16_t>(rnrgen);
	o << tmp;
	return o;
}

/*********************************************************************/
/**
 * Reads a Gem::Util::rnrGenerationMode item from a stream
 *
 * @param i The stream the item should be read from
 * @param rnrgen The item read from the stream
 * @return The std::istream object used to read the item from
 */
std::istream& operator>>(std::istream& i, Gem::Util::rnrGenerationMode& rnrgen){
	boost::uint16_t tmp;
	i >> tmp;

#ifdef DEBUG
	rnrgen = boost::numeric_cast<Gem::Util::rnrGenerationMode>(tmp);
#else
	rnrgen = static_cast<Gem::Util::rnrGenerationMode>(tmp);
#endif /* DEBUG */

	return i;
}

/*********************************************************************/
} /* namespace Util */

} /* namespace Gem */
