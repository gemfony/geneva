/**
 * @file observer2.cpp
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
 * This example shows how to repeatedly update a very large number of
 * objects (300000) by storing pointers to them and directly calling the
 * relevant functions.
 *
 * The purpose of this test was to determine whether this simple solution
 * or Boost.Signals2 would yield better results. It currently appears as
 * if this solution is much faster and needs less memory.
 *
 * Compile with a command line similar to
 * g++ -o observer2 -O2 -I /opt/boost141/include/ observer2.cpp
 */


#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>

#include <vector>
#include <iostream>


/*******************************************************/
/*
 * A class that is interested in changes occuring in
 * a data set
 */
class observer
{
public:
	observer(int id)
	: id_(id)
	  , secret_(0)
	  { /* nothing */ }

	~observer() { /* nothing */ }

	void setSecret(int secret) {
		secret_ = secret;
	}

	int getId() const {
		return id_;
	}

	void printSecret() {
		// std::cout << secret_ << " " << sizeof(m_sig) << " " << sizeof(m_conn) << std::endl;
		std::cout << secret_ << std::endl;
	}

private:
	int id_;
	int secret_;
};

/*******************************************************/
/*
 * A class holding the data to be updated
 */
class dataHolder
{
public:
	dataHolder() {
		std::vector<boost::shared_ptr<observer> > observers;
		std::vector<boost::shared_ptr<observer> >::iterator o_it;

		// Create the required number of objects
		for(int j=0; j<300000; j++) {
			// Create a new observer
			boost::shared_ptr<observer> p(new observer(j));
			observers.push_back(p);
		}

		// Perform the same task a number of times
		for(int i=0; i<10; i++) {
			std::cout << "In iteration " << i << std::endl;

			// Update the data of all observers and print the result
			for(o_it=observers.begin(); o_it!=observers.end(); ++o_it) {
				(*o_it)->setSecret((*o_it)->getId() + i);
				// (*o_it)->printSecret();
			}
		}

		observers.clear();
	}
};

/*******************************************************/
/*
 * The main function
 */
int main() {
	dataHolder DataHolder;
}
