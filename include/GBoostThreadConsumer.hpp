/**
 * @file
 */

/* GBoostThreadConsumer.hpp
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

#include <boost/version.hpp>

#if BOOST_VERSION < 103600
#error "Error: Boost should at least have version 1.36 !"
#endif /* BOOST_VERSION */

#include <boost/cstdint.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/version.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/thread.hpp>
#include <boost/threadpool.hpp>

#ifndef GBOOSTTHREADCONSUMER_HPP_
#define GBOOSTTHREADCONSUMER_HPP_

#include "GIndividualBroker.hpp"
#include "GIndividual.hpp"
#include "GThreadGroup.hpp"

namespace Gem {
namespace GenEvA {

const boost::uint16_t DEFAULTGBTCMAXTHREADS = 4;

/***************************************************************/
/**
 * A derivative of GConsumer<carryer_type>, that processes items in separate
 * threads. This class is mainly intended for debugging purposes.
 * Use GBoostThreadPopulation instead, if you want to have a
 * multi-threaded population - it should have a much higher performance.
 *
 * This class could exist alongside a networked consumer, as the broker
 * accepts more than one consumer. Hence there might be real-life environments
 * where using this class makes sense.
 *
 * \todo Need to deal with id issue of GConsumer. get/put now work with this
 */
class GBoostThreadConsumer
	:public Gem::GenEvA::GConsumer<Gem::GenEvA::GIndividual>
{
public:
	/** @brief Standard constructor */
	GBoostThreadConsumer();
	/** @brief Standard destructor */
	virtual ~GBoostThreadConsumer();

	/** @brief Sets the maximum number of threads */
	void setMaxThreads(boost::uint16_t);
	/** @brief Retrieves the maximum number of threads */
	uint16_t getMaxThreads() const throw();

protected:
	/** @brief The actual business logic */
	virtual void customProcess();
	/** @brief To be called after customProcess() */
	virtual void finally();

private:
	GBoostThreadConsumer(const GBoostThreadConsumer&); ///< Intentionally left undefined
	const GBoostThreadConsumer& operator=(const GBoostThreadConsumer&); ///< Intentionally left undefined

	/** @brief Retrieves, processes and submits items */
	void processItems();

	boost::uint16_t maxThreads_; ///< The maxumum number of allowed threads in the pool
	Gem::Util::GThreadGroup gtg_;
};

/***************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */

#endif /* GBOOSTTHREADCONSUMER_HPP_ */
