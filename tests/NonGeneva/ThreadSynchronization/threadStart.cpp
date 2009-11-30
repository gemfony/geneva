/**
 * @file threadStart.cpp
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
 * This example tries to find suitable ways of simultaneously
 * starting execution of a number of Boost.Threads's main
 * execution function. Compile with a line similar to
 *
 * g++ -o threadStart -L/opt/boost136/lib -I/opt/boost136/include/boost-1_36/ \
 *     -lboost_thread-gcc43-mt -lboost_system-gcc43-mt threadStart.cpp
 *
 * Current problems: Not all threads get called even remotely equally often.
 * Call frequencies are 14564 1607 8676
 */

#include <iostream>
#include <unistd.h>

#include <boost/cstdint.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>
#include <boost/bind.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/thread.hpp>

using namespace std;

class test
{
public:
    test()
	:lock(helloMutex_),
	 jointData_(0),
	 MAXJOINTDATA(10000),
	 thread1(boost::bind(&test::sayHello, this)),
	 thread2(boost::bind(&test::sayHello, this)),
	 thread3(boost::bind(&test::sayHello, this))
	{ /* nothing */ }

    void startAndStopThreads(){
	lock.unlock();

	while(true){
	    boost::unique_lock<boost::mutex> local_lock(helloMutex_);

	    if(jointData_ >= MAXJOINTDATA){
		thread1.interrupt();
		thread2.interrupt();
		thread3.interrupt();

		break;
	    }
	}

	thread1.join();
	thread2.join();
	thread3.join();
    }

    void sayHello(){
	for(;;){
	    if(boost::this_thread::interruption_requested()) break;

	    boost::unique_lock<boost::mutex> local_lock(helloMutex_);
	    std::cout << "Hello world Nr. " << jointData_++
		      << " from thread " << boost::this_thread::get_id() << std::endl;
	}
    }

private:
    boost::mutex helloMutex_;
    boost::unique_lock<boost::mutex> lock;

    uint32_t jointData_;
    const uint32_t MAXJOINTDATA;

    boost::thread thread1, thread2, thread3;
};

main(){
    test Test;
    Test.startAndStopThreads();

    std::cout << "Done ..." << std::endl;
}
