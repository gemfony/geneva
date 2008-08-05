/* threadStart.cpp
 *
 * Copyright (C) 2004-2008 Dr. Ruediger Berlich
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

/*
 * This example tries to find suitable ways of simultaneously 
 * starting execution of a number of Boost.Threads's main
 * execution function.
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
     startProcessing_(false),
     jointData_(0),
     thread1(boost::bind(&test::sayHello,this,1)),
     thread2(boost::bind(&test::sayHello,this,2)),
     thread3(boost::bind(&test::sayHello,this,3))
  { /* nothing */ }

  void startAndStopThreads(){
    startProcessing_ = true;
    lock.unlock();
    allReady_.notify_all();

    usleep(1000);

    thread1.interrupt();
    thread2.interrupt();
    thread2.interrupt();
    
    thread1.join();
    thread2.join();
    thread2.join();    
  }

  void sayHello(const uint8_t& threadNumber){
    for(;;){
      boost::unique_lock<boost::mutex> local_lock(helloMutex_);
      while(!startProcessing_){
	  allReady_.wait(local_lock);
      }

      if(boost::this_thread::interruption_requested()) break;

      std::cout << "Hello world Nr. " << jointData_++ 
		<< " from thread " << threadNumber << std::endl;
    }
  }

private:
  boost::mutex helloMutex_;
  boost::unique_lock<boost::mutex> lock;
  boost::condition_variable allReady_;

  bool startProcessing_;
  uint32_t jointData_;

  boost::thread thread1, thread2, thread3;
};

main(){
  std::cout << "Instantiating test class" << std::endl;
  test Test;
  std::cout << "Starting threads" << std::endl;

  Test.startAndStopThreads();

  std::cout << "Done ..." << std::endl;
}
