/**
 * @file conversionIterator.cpp
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

/**
 * This example demonstrates how to write a simple iterator that allows to
 * select "views" of a vector of base objects. Depending on the template
 * argument of the iterator, only derived1 or derived2 objects are returned
 * (converted from the base object). The intention is to use this technique
 * to select different GParameterBase-derivatives in individuals
 * (e.g. "give me all GConstrainedDoubleCollection objects!").
 *
 * Compile with a command line similar to
 * g++ -g -I/opt/boost141/include/ -o conversionIterator conversionIterator.cpp
 */

#include <string>
#include <memory>
#include <iostream>
#include <algorithm>
#include <functional>
#include <vector>

#include <boost/iterator/iterator_facade.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/cstdint.hpp>

/****************************************************************/

class base {
public:
	base() {}
	virtual ~base() {}

	virtual void printSecret() const = 0;
	virtual boost::int32_t getSecret() const = 0;
};

/****************************************************************/

class derived1: public base
{
public:
	derived1(const boost::int32_t secret):
		secret_(secret)
		{}

	virtual void printSecret() const {
		std::cout << secret_ << std::endl;
	}

	virtual boost::int32_t getSecret() const {
		return secret_;
	}

private:
	boost::int32_t secret_;
};

/****************************************************************/

class derived2: public base
{
public:
	derived2(const boost::int32_t secret):
		secret_(secret)
		{}

	virtual void printSecret() const {
		std::cout << secret_ << std::endl;
	}

	virtual boost::int32_t getSecret() const {
		return secret_;
	}

private:
	boost::int32_t secret_;
};

/****************************************************************/

class derived3: public base
{
public:
	derived3(const boost::int32_t secret):
		secret_(secret)
		{}

	virtual void printSecret() const {
		std::cout << secret_ << std::endl;
	}

	virtual boost::int32_t getSecret() const {
		return secret_;
	}

private:
	boost::int32_t secret_;
};

/****************************************************************/

template <typename baseType, typename returnType>
class conversion_iterator
: public boost::iterator_facade<
  conversion_iterator<baseType, returnType>
, boost::shared_ptr<baseType>
, boost::forward_traversal_tag
, boost::shared_ptr<returnType>
>
{
public:
	conversion_iterator(typename std::vector<boost::shared_ptr<baseType> >::iterator const& end)
	:end_(end)
	 { /* nothing */ }

	void operator=(typename std::vector<boost::shared_ptr<baseType> >::iterator const& current) {
		current_ = current;
		// Skip to first "good" entry
		boost::shared_ptr<returnType> p;
		while(current_ != end_ &&
				!(p = boost::dynamic_pointer_cast<returnType>(*current_))) {
			++current_;
		}
	}

	bool operator!=(typename std::vector<boost::shared_ptr<baseType> >::iterator const& other) const {
		return current_ != other;
	}

	void resetEndPosition(typename std::vector<boost::shared_ptr<baseType> >::iterator const& end) {
		end_ = end;
	}

private:
	friend class boost::iterator_core_access;

	boost::shared_ptr<returnType> dereference() const {
		if(current_ == end_) {
			std::cout << "Error: current position at end of sequence" << std::endl;
			throw;
		}

		boost::shared_ptr<returnType> p = boost::dynamic_pointer_cast<returnType>(*current_);

		if(!p) {
			std::cout << "Error: empty pointer" << std::endl;
			throw;
		}

		return p;
	}

	bool equal(typename std::vector<boost::shared_ptr<baseType> >::iterator const& other) const {
		return current_ == other;
	}

	void increment() {
		boost::shared_ptr<returnType> p;

		while (current_ != end_) {
			++current_;
			if(current_!=end_ && (p = boost::dynamic_pointer_cast<returnType>(*current_))) break;
		}
	}

	/**************************************************************/
	typename std::vector<boost::shared_ptr<baseType> >::iterator current_; ///< Marks the current position in the iteration sequence
	typename std::vector<boost::shared_ptr<baseType> >::iterator end_; ///< Marks the end of the iteration sequence
};

/****************************************************************/

int main()
{
	std::vector<boost::shared_ptr<base> > baseVec;

	for(boost::int32_t i=-10; i<10; i++) {
		if(i%2==0) {
			baseVec.push_back(boost::shared_ptr<base>(new derived1(i)));
		}
		else {
			baseVec.push_back(boost::shared_ptr<base>(new derived2(i)));
		}
	}

	std::cout << "Sequence derived1:" << std::endl;
	conversion_iterator<base, derived1> conv_it1(baseVec.end());
	for(conv_it1=baseVec.begin(); conv_it1 != baseVec.end(); ++conv_it1) {
		(*conv_it1)->printSecret();
	}

	std::cout << "Sequence derived2:" << std::endl;
	conversion_iterator<base, derived2> conv_it2(baseVec.end());
	for(conv_it2=baseVec.begin(); conv_it2 != baseVec.end(); ++conv_it2) {
		(*conv_it2)->printSecret();
	}

	std::cout << "Sequence derived3:" << std::endl;
	conversion_iterator<base, derived3> conv_it3(baseVec.end());
	for(conv_it3=baseVec.begin(); conv_it3 != baseVec.end(); ++conv_it3) {
		(*conv_it3)->printSecret();
	}
	std::cout << "Should be empty!" << std::endl;

	std::cout << "Add a single derived 3 entry and try again" << std::endl;
	baseVec.push_back(boost::shared_ptr<base>(new derived3(42)));
	// baseVec.end() has changed so we need to let the iterator know
	conv_it3.resetEndPosition(baseVec.end());
	for(conv_it3=baseVec.begin(); conv_it3 != baseVec.end(); ++conv_it3) {
		(*conv_it3)->printSecret();
	}
	std::cout << "Now there should have been a single entry with value 42" << std::endl;
}

