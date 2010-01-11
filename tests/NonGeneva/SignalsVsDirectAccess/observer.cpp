/**
 * @file observer.cpp
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
 * This example shows how to use Boost.Signals2 library to repeatedly update
 * a very large number of objects (300000).
 *
 * While signals are an elegant solution to the problem, this code needs far
 * more compute time and memory than accessing the objects' functions directly.
 *
 * Another experience made was that Boost.Signals2 is approximately 25% faster
 * than the older (and not thread-safe) Boost.Signals library.
 *
 * Compile with a command line similar to
 * g++ -o observer -O2 -I /opt/boost141/include/ observer.cpp
 */

#include <boost/signals2.hpp>
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
	typedef boost::signals2::signal<int (int)>  signal_t;
	typedef boost::signals2::connection connection_t;

public:
	observer(int id)
	: id_(id)
	  , secret_(0)
	  { /* nothing */ }

	~observer() {
		m_conn.disconnect();
	}

	void updateSettings() {
		secret_ = *m_sig(id_);
	}

	void printSecret() {
		// std::cout << secret_ << " " << sizeof(m_sig) << " " << sizeof(m_conn) << std::endl;
		std::cout << secret_ << std::endl;
	}

	connection_t connect(signal_t::slot_function_type subscriber)
	{
		return m_sig.connect(subscriber);
	}

private:
	signal_t m_sig;
	connection_t m_conn;

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
		boost::signals2::signal<void ()> updateCall;
		std::vector<boost::shared_ptr<observer> > observers;
		std::vector<boost::shared_ptr<observer> >::iterator o_it;

		std::vector<boost::signals2::connection> subscribers;
		std::vector<boost::signals2::connection>::iterator s_it;

		for(int j=0; j<300000; j++) {
			// Create a new observer
			boost::shared_ptr<observer> p(new observer(j));

			// Wire our getSecretUpdate function to the observer's signal
			p->connect(boost::bind(&dataHolder::getSecretUpdate, this, _1));

			// Wire its updateSettings function to our signal
			subscribers.push_back(updateCall.connect(boost::bind(&observer::updateSettings, p)));

			observers.push_back(p);
		}

		// Perform the same task a number of times
		for(int i=0; i<10; i++) {
			i_ = i;

			std::cout << "In iteration " << i << std::endl;

			// Ask all observers to update their data
			updateCall();

			/*
      for(o_it=observers.begin(); o_it!=observers.end(); ++o_it) {
        (*o_it)->printSecret();
      }
			 */
		}

		for(s_it=subscribers.begin(); s_it!=subscribers.end(); ++s_it) {
			s_it->disconnect();
		}

		observers.clear();
		subscribers.clear();
	}

	int getSecretUpdate(int id) {
		return 10*i_ + id;
	}

private:
	int i_;
};

/*******************************************************/
/*
 * The main function
 */
int main() {
	dataHolder DataHolder;
}
