/**
 * @file GBoostThreadPopulation.hpp
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

// Standard headers go here

// Boost headers go here

#include <boost/version.hpp>

#if BOOST_VERSION < 103600
#error "Error: Boost should at least have version 1.36 !"
#endif /* BOOST_VERSION */

#include <boost/cstdint.hpp>
#include <boost/bind.hpp>
#include <boost/utility.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/xtime.hpp>
#include <boost/threadpool.hpp>

#ifndef GBOOSTTHREADPOPULATION_HPP_
#define GBOOSTTHREADPOPULATION_HPP_

// GenEvA headers go here

#include "GBasePopulation.hpp"
#include "GIndividual.hpp"
#include "GObject.hpp"
#include "GLogger.hpp"
#include "GenevaExceptions.hpp"

namespace Gem {
namespace GenEvA {
/** @brief The default number of threads for parallelization with boost */
const boost::uint16_t DEFAULTBOOSTTHREADS = 2;

/********************************************************************/
/**
 * A multi-threaded population based on GBasePopulation. This version
 * uses the Boost.Threads library and a thread-pool library from
 * http://threadpool.sf.net .
 */
class GBoostThreadPopulation
	: public GenEvA::GBasePopulation
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int version) {
		using boost::serialization::make_nvp;
		ar & make_nvp("GBTGBasePopulation",
				boost::serialization::base_object<GBasePopulation>(*this));
		ar & make_nvp("nThreads_", nThreads_);
	}
	///////////////////////////////////////////////////////////////////////

public:
	/** @brief The default constructor */
	GBoostThreadPopulation();
	/** @brief A standard copy constructor */
	GBoostThreadPopulation(const GBoostThreadPopulation&);
	/** @brief The standard destructor */
	virtual ~GBoostThreadPopulation();

	/** @brief Assignment operator */
	const GBoostThreadPopulation& operator=(const GBoostThreadPopulation&);

	/** @brief Loads data from another object */
	virtual void load(const GObject *);
	/** @brief Creates a deep clone of this object */
	virtual GObject *clone();

	/** @brief Overloaded from GBasePopulation::optimize() */
	virtual void optimize();

	/** @brief Sets the maximum number of threads */
	void setNThreads(const boost::uint8_t&);
	/** @brief Retrieves the maximum number of threads */
	uint8_t getNThreads() const throw();

protected:
	/** @brief Overloaded version from GBasePopulation,
	 * core of the Boost-thread implementation */
	virtual void mutateChildren();

private:
	boost::uint8_t nThreads_; ///< The number of threads
	boost::threadpool::pool tp_; ///< A thread pool
};

/********************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */

#endif /* GBOOSTTHREADPOPULATION_HPP_ */
