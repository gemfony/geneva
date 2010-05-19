/**
 * @file GIndividualSerializationHelperFunctions.cpp
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

#include "GIndividualSerializationHelperFunctions.hpp"

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


} /* namespace GenEvA */

} /* namespace Gem */
