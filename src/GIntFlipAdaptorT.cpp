/**
 * @file GIntFlipAdaptorT.cpp
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

#include "GIntFlipAdaptorT.hpp"

namespace Gem {
namespace GenEvA {

/***********************************************************************************************/
//////////////////////// Specialization for some integer and/or bool types///////////////////////
/***********************************************************************************************/
/**
 * Specialization for typeof(int_type) == typeof(bool).
 *
 * @param value The value to be mutated
 */
template<>
void GIntFlipAdaptorT<bool>::customMutations(bool& value) {
	double probe = gr.evenRandom(0.,1.);
	if(probe < mutProb_.value()) {
		value==true?value=false:value=true;
	}
}

/***********************************************************************************************/
/**
 * Specialization for typeof(int_type) == typeof(short). The main difference to the generic
 * version is that the boundary check is mandatory.
 *
 * @param value The value to be mutated
 */
template<>
void GIntFlipAdaptorT<short>::customMutations(short& value) {
	double probe = this->gr.evenRandom(0.,1.);
	if(probe < mutProb_.value()) {
		bool up = this->gr.boolRandom();
		if(up){
			if(std::numeric_limits<short>::max() == value) value -= 1;
			else value += 1;
		}
		else {
			if(std::numeric_limits<short>::min() == value) value += 1;
			else value -= 1;
		}
	}
}

/***********************************************************************************************/
/**
 * Specialization for typeof(int_type) == typeof(unsigned short). The main difference to the generic
 * version is that the boundary check is mandatory.
 *
 * @param value The value to be mutated
 */
template<>
void GIntFlipAdaptorT<unsigned short>::customMutations(unsigned short& value) {
	double probe = this->gr.evenRandom(0.,1.);
	if(probe < mutProb_.value()) {
		bool up = this->gr.boolRandom();
		if(up){
			if(std::numeric_limits<unsigned short>::max() == value) value -= 1;
			else value += 1;
		}
		else {
			if(std::numeric_limits<unsigned short>::min() == value) value += 1;
			else value -= 1;
		}
	}
}

/***********************************************************************************************/
/**
 * Specialization for typeof(int_type) == typeof(boost::uint8_t). The main difference to the generic
 * version is that the boundary check is mandatory.
 *
 * @param value The value to be mutated
 */
template<>
void GIntFlipAdaptorT<boost::uint8_t>::customMutations(boost::uint8_t& value) {
	double probe = this->gr.evenRandom(0.,1.);
	if(probe < mutProb_.value()) {
		bool up = this->gr.boolRandom();
		if(up){
			if(std::numeric_limits<boost::uint8_t>::max() == value) value -= 1;
			else value += 1;
		}
		else {
			if(std::numeric_limits<boost::uint8_t>::min() == value) value += 1;
			else value -= 1;
		}
	}
}

/***********************************************************************************************/
/**
 * Specialization for typeof(int_type) == typeof(boost::int8_t). The main difference to the generic
 * version is that the boundary check is mandatory.
 *
 * @param value The value to be mutated
 */
template<>
void GIntFlipAdaptorT<boost::int8_t>::customMutations(boost::int8_t& value) {
	double probe = this->gr.evenRandom(0.,1.);
	if(probe < mutProb_.value()) {
		bool up = this->gr.boolRandom();
		if(up){
			if(std::numeric_limits<boost::int8_t>::max() == value) value -= 1;
			else value += 1;
		}
		else {
			if(std::numeric_limits<boost::int8_t>::min() == value) value += 1;
			else value -= 1;
		}
	}
}

/***********************************************************************************************/
/**
 * Specialization for typeof(int_type) == typeof(char). The main difference to the generic
 * version is that the boundary check is mandatory.
 *
 * @param value The value to be mutated
 */
template<>
void GIntFlipAdaptorT<char>::customMutations(char& value) {
	double probe = this->gr.evenRandom(0.,1.);
	if(probe < mutProb_.value()) {
		bool up = this->gr.boolRandom();
		if(up){
			if(std::numeric_limits<char>::max() == value) value -= 1;
			else value += 1;
		}
		else {
			if(std::numeric_limits<char>::min() == value) value += 1;
			else value -= 1;
		}
	}
}

/***********************************************************************************************/

} /* namespace GenEvA  */
} /* namespace Gem */
