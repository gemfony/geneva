/* SynchronousThreadStart.cpp
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
 * This example tries to make several threads start after a certain condition was
 * met and let them stop after a number of counts. This works nicely except for the
 * fact that the controlling thread, which is supposed to stop the other threads,
 * is often only assigned CPU time after a while. Thus the other threads perform 
 * more work than they are supposed to do.
 */


#include <iostream>

#include <boost/cstdint.hpp>
#include <boost/bind.hpp>
#include <boost/exception.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/thread.hpp>

#include <boost/date_time.hpp>

using namespace std;

class test
{
public:
  test()
    :jointData_(0),
     MAXJOINTDATA(10),
     go_(false),
     thread1(boost::bind(&test::sayHello, this, 1)),
     thread2(boost::bind(&test::sayHello, this, 2)),
     thread3(boost::bind(&test::sayHello, this, 3))
  { /* nothing */ }

  void startAndStopThreads(){
    std::cout << "Going to sleep for 2 seconds in startAndStopThreads()" << std::endl;
    boost::this_thread::sleep(boost::posix_time::seconds(2));

    boost::unique_lock<boost::mutex> lock(helloMutex_);
    go_=true;
    lock.unlock();
    readyToGo_.notify_all();

    while(true){
      boost::unique_lock<boost::mutex> lock2(helloMutex_);

      if(jointData_ >= MAXJOINTDATA){
	go_=false;

	std::cout << "Sending interrupt" << std::endl;
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

  void sayHello(const uint16_t threadNumber){
    while(true){	    
      boost::unique_lock<boost::mutex> lock(helloMutex_);

      while(!go_) {
	try{
	  readyToGo_.wait(lock);
	}
	catch(boost::thread_interrupted&){
	  std::cout << "Received interrupt in thread " << threadNumber << std::endl;
	  return;
	}
      }

      std::cout << "Hello world Nr. " << jointData_++ 
		<< " from thread " << threadNumber << std::endl;

      boost::this_thread::yield();
    }
  }

private:
  volatile uint32_t jointData_;
  const uint32_t MAXJOINTDATA;
  bool go_;

  boost::mutex helloMutex_;
  boost::condition_variable readyToGo_;
  boost::thread thread1, thread2, thread3;
};

main(){
  test Test;

  std::cout << "Starting threads" << std::endl;
  // Test.startAndStopThreads();
  boost::thread thread4(boost::bind(&test::startAndStopThreads,&Test));
  thread4.join();

  std::cout << "Done ..." << std::endl;
}
