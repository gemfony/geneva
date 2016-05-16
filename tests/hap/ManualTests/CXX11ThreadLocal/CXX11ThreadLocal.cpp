/**
 * @file CXX11ThreadLocal.cpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) eu
 *
 * This file is part of the Geneva library collection.
 *
 * Geneva was developed with kind support from Karlsruhe Institute of
 * Technology (KIT) and Steinbuch Centre for Computing (SCC). Further
 * information about KIT and SCC can be found at http://www.kit.edu/english
 * and http://scc.kit.edu .
 *
 * Geneva is free software: you can redistribute and/or modify it under
 * the terms of version 3 of the GNU Affero General Public License
 * as published by the Free Software Foundation.
 *
 * Geneva is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with the Geneva library. If not, see <http://www.gnu.org/licenses/>.
 *
 * For further information on Gemfony scientific and Geneva, visit
 * http://www.gemfony.eu .
 */

/**
 * This test creates an object to be stored in thread_local mode. It is
 * accessed by two std::thread in order to check, that the constructor and
 * destructor are called properly.
 */

// Standard header files
#include <iostream>
#include <thread>
#include <atomic>
#include <vector>

std::atomic<std::size_t> constructor(0);
std::atomic<std::size_t> destructor(0);

class tlsTest {
public:
	 tlsTest() {
		 constructor++;
	 }

	 ~tlsTest() {
		 destructor++;
	 }

	 decltype(std::this_thread::get_id()) getId() const { return std::this_thread::get_id(); }
};

thread_local tlsTest g_tlsTest;

const std::size_t NTHREADS=10;

void runner() {
	std::cout << "Thread " << std::this_thread::get_id() << " started." << std::endl;

	if(g_tlsTest.getId() != std::this_thread::get_id()) {
		std::cout << "Got invalid thread id from tls: " << g_tlsTest.getId() << " (expected " << std::this_thread::get_id() << ")" << std::endl;
	}
}

int main(int argc, char **argv) {
	std::vector<std::thread> v_thread;
	for(std::size_t t=0; t<NTHREADS; t++) {
		v_thread.emplace_back(std::thread(runner));
	}

	for(auto& t: v_thread) {
		t.join();
	}

	if(constructor.load() != NTHREADS || destructor.load() != NTHREADS) {
		std::cout
		<< "Error: Invalid number of constructor and destructor calls." << std::endl
		<< "Expected " << NTHREADS << " but got " << constructor.load() << " / " << destructor.load() << std::endl;

		return 1;
	} else {
		std::cout
		<< "Got the expected number of constructor and destructor calls: " << constructor.load() << " / " << destructor.load() << std::endl;

		return 0;
	}
}
