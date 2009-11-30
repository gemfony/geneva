/**
 * @file serializationTest.cpp
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

/*
 * This test tries to determine possible problems with Boost.Serialization,
 * in the field of memory-(de-)allocation.
 *
 * On OpenSUSE 11, with Boost 1.36 under /opt/boost136, compile like this:
 *
 * g++ -g -o serializationTest -I /opt/boost136/include/boost-1_36/ \
 *                             -L/opt/boost136/lib -lboost_system-gcc43-mt \
 *                             -lboost_serialization-gcc43-mt serializationTest.cpp
 *
 * Add the -DSHOWRESULT switch if you want to see the result of the serialization
 * every 1000 iterations. Note that this does not make sense for binary archives.
 */

// Standard header files go here
#include <iostream>
#include <cmath>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <functional>      // For greater<T>( )
// Boost header files go here
#include <boost/function.hpp>
#include <boost/cstdint.hpp>
#include <boost/utility.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/assume_abstract.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/date_time.hpp>
#include <boost/lexical_cast.hpp>

const boost::uint32_t MAXCOUNT = 100;
const boost::uint32_t MAXITERATIONS = 100000;
const std::size_t ARRAYSIZE = 10;

class base {
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version) {
		using boost::serialization::make_nvp;
		ar & make_nvp("secret_", secret_);
	}
	///////////////////////////////////////////////////////////////////////

public:
	base() {
		for (boost::uint32_t i = 0; i < MAXCOUNT; i++)
			secret_.push_back(i);
	}

	virtual ~base() {
	}
	virtual void doSomeWork() = 0;

protected:
	std::vector<boost::uint32_t> secret_;
};

BOOST_SERIALIZATION_ASSUME_ABSTRACT(base)

class derived :public base {
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version) {
		using boost::serialization::make_nvp;
		ar & make_nvp("base", boost::serialization::base_object<base>(*this));
	}
	///////////////////////////////////////////////////////////////////////

public:
	virtual ~derived() {}

	virtual void doSomeWork() {
		std::sort(secret_.begin(), secret_.end(), std::greater<boost::uint32_t>());
	}
};

BOOST_CLASS_EXPORT(derived)

// Try this out for different archive types.
// On a 3.3 GHz Intel Celeron (single CPU), execution took
// 42.8s for binary archives
// 2m10s for text archives
// 6m57s for xml archives, almost 10 times as much as for the binary archive ...

// #define ARCHIVETYPEIN  binary_iarchive
// #define ARCHIVETYPEOUT binary_oarchive
// #define ARCHIVETYPEIN  text_iarchive
// #define ARCHIVETYPEOUT text_oarchive
#define ARCHIVETYPEIN  xml_iarchive
#define ARCHIVETYPEOUT xml_oarchive

main() {
	std::ostringstream derivedStream;

	// Create a string representation of a derived object
	{
		boost::shared_ptr<base> bDerived (new derived());
		boost::archive::ARCHIVETYPEOUT oa(derivedStream);
		oa << boost::serialization::make_nvp("Derived" , bDerived);
	} // bDerived is destroyed at this point, and along with it the derived object

	for(boost::uint32_t i=0; i<MAXITERATIONS; i++) {
		boost::shared_ptr<base> baseArray[ARRAYSIZE];

		// De-serialize the text representation of Derived into the base pointers
		for(std::size_t j=0; j<ARRAYSIZE; j++) {
			std::istringstream istr(derivedStream.str());
			{
				boost::archive::ARCHIVETYPEIN ia(istr);
				ia >> boost::serialization::make_nvp("Derived", baseArray[j]);
			}
		}

		// Let the new objects do some work
		for(std::size_t j=0; j<ARRAYSIZE; j++) {
			baseArray[j]->doSomeWork();

#ifdef SHOWRESULT
			if(i%1000 == 0 && j==0) { // Emit serialized derived object
				std::ostringstream newBaseStream;
				boost::archive::ARCHIVETYPEOUT oa(newBaseStream);
				oa << boost::serialization::make_nvp("newBase" , baseArray[0]);
				std::cout << newBaseStream.str() << std::endl << std::endl;
			}
#endif /* SHOWRESULT */

			baseArray[j].reset();
		}

		if(i%100 == 0) {
			std::cout << "Passed " << i << std::endl;
		}
	}
}
