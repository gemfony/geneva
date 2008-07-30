/**
 * \file
 */

/**
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

#include "GMemberCarrier.hpp"

// Included here so no conflicts occur. See explanation at
// http://www.boost.org/libs/serialization/doc/special.html#derivedpointers
#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(GenEvA::GMemberCarrier)

namespace GenEvA {

/*********************************************************************/
/**
 * Default constructor. This should usually only be called by
 * serialization functions.
 */
GMemberCarrier::GMemberCarrier(void) :
	parent_(false), generation_(0), command_(""), id_("") { /* nothing - we do not really want this constructor */
}

/*********************************************************************/
/**
 * Initializes the GMemberCarrier from a description contained in a string
 *
 * \param descr A string holding a description of this object
 */
GMemberCarrier::GMemberCarrier(const string& descr) {
	this->fromString(descr);
}

/*********************************************************************/
/**
 * Sets all information about the GMember object at once
 *
 * \param gmember The payload for this carrier
 * \param cmd The command associated with it
 * \param id An id associated with the payload (usually the id of its population)
 * \param generation The genereration the payload belongs to
 * \param isparent Indication of whether this is a parent or not
 */
GMemberCarrier::GMemberCarrier(const shared_ptr<GMember>& gmember,
		const string& cmd, const string& id, uint32_t generation, bool isparent) :
	parent_(isparent), generation_(generation), payload_(gmember),
			command_(cmd), id_(id) {
	// Check that the payload has a suitable value
	if (!payload_) {
		GException ge;
		ge << "In GMemberCarrier::GMemberCarrier(...) :" << endl
				<< "Error. Payload is empty!" << endl << raiseException;
	}

	// Check that we got the commands we expected
	if (command() != "evaluate" && command() != "mutate") {
		GException ge;
		ge << "In GMemberCarrier::GMemberCarrier(...) :" << endl
				<< "Error. Bad command given : " << command() << endl
				<< raiseException;
	}
}

/*********************************************************************/
/**
 * Sets all information about the GMember except the id object at once
 *
 * \param gmember The payload for this carrier
 * \param cmd The command associated with it
 * \param id An id associated with the payload (usually the id of its population)
 * \param generation The genereration the payload belongs to
 * \param isparent Indication of whether this is a parent or not
 */
GMemberCarrier::GMemberCarrier(const shared_ptr<GMember>& gmember,
		const string& cmd, uint32_t generation, bool isparent) :
	parent_(isparent), generation_(generation), payload_(gmember),
			command_(cmd), id_("") {
	// Check that we got the commands we expected
	if (command() != "evaluate" && command() != "mutate") {
		GException ge;
		ge << "In GMemberCarrier::GMemberCarrier(...) :" << endl
				<< "Error. Bad command given : " << command() << endl
				<< raiseException;
	}
}

/*********************************************************************/
/**
 * Standard destructor. We have no local, dynamically allocated data,
 * hence it is empty.
 */
GMemberCarrier::~GMemberCarrier() { /* nothing */
}

/*********************************************************************/
/**
 * Does the actual processing. This allows network clients to just call
 * this function after de-serialization - the GMemberCarrier object knows
 * itself what to do. This way clients can stay quite simple. Note that this
 * function is called by a thread. We thus need to catch all exceptions
 * that might occur in the functions we call.
 *
 * \todo We can put the "vocabulary" in a central place and let all
 * relevant functions check it when needed
 * \todo Need to catch all exceptions in this class - it is called from a thread
 */
void GMemberCarrier::process(void) {
	try {
		if (command() == "evaluate") {
			// We need to be able to trigger fitness calculation
			// for dirty individuals but cannot call customFitness() directly
			uint8_t currentEvaluationPermission =
					payload()->setEvaluationPermission(ENFORCEEVALUATION);
			payload()->fitness();
			payload()->setEvaluationPermission(currentEvaluationPermission); // restore
		} else if (command() == "mutate") {
			// We want to enforce fitness recalculation
			uint8_t currentEvaluationPermission =
					payload()->setEvaluationPermission(ENFORCEEVALUATION);
			payload()->mutate();
			payload()->setEvaluationPermission(currentEvaluationPermission); // restore
		}
	} catch (boost::thread_interrupted&) { // We have been asked to stop
		return;
	} catch (boost::exception& e) {
		GLogStreamer gls;
		gls << "In GConsumer::process(): Caught boost::exception with message"
				<< endl << e.diagnostic_information() << endl << logLevel(
				CRITICAL);

		abort();
	} catch (std::exception& e) {
		GLogStreamer gls;
		gls << "In GConsumer::process(): Caught std::exception with message"
				<< endl << e.what() << endl << logLevel(CRITICAL);

		abort();
	} catch (...) {
		GLogStreamer gls;
		gls << "In GConsumer::process(): Caught unknown exception." << endl
				<< logLevel(CRITICAL);

		abort();
	}
}

/*********************************************************************/
/**
 * Checks whether we own the last copy of the GMember
 *
 * \return A boolean indicating whether we hold the only copy of the GMember object
 */
bool GMemberCarrier::orphaned(void) {
	return payload().unique();
}

/*********************************************************************/
/**
 * Checks whether our payload is a parent
 *
 * \return A boolean indicating whether the payload is a parent
 */
bool GMemberCarrier::isParent(void) const {
	return parent_;
}

/*********************************************************************/
/**
 * Retrieves the generation the payload belongs to
 *
 * \return The generation the payload belongs to
 */
uint32_t GMemberCarrier::getGeneration(void) const {
	return generation_;
}

/*********************************************************************/
/**
 * Retrieves the payload of this class. Note that we return the payload
 * by value, not by reference. This is because thie object might quickly
 * get out of scope after delivery of the payload.
 *
 * \return The payload of this object
 */
shared_ptr<GMember> GMemberCarrier::payload(void) const {
	return payload_;
}

/********************************************************************/
/**
 * Retrieves the command associated with the payload
 *
 * \return The command associated with the payload
 */
string GMemberCarrier::command(void) const {
	return command_;
}

/********************************************************************/
/**
 * Sets the id associated with the payload. Usually this is the id
 * of the class the payload originated from.
 *
 * \param id An id associated with the payload
 */
void GMemberCarrier::setId(const string& id) {
	id_ = id;
}

/********************************************************************/
/**
 * Retrieves the id associated with the payload.
 *
 * \return An id associated with the payload
 */
const string& GMemberCarrier::getId(void) const {
	return id_;
}

/********************************************************************/
/**
 * Transforms the object, including its content, into a string.
 * This function uses Boost.Serialization.
 *
 * IMPORTANT: There are rumors that serialization is currently only
 * thread-safe when used with g++. If this is true, we need to
 * protect the serializationc alls below accordingly with a mutex.
 *
 * \return A string contain a description of this object
 */
string GMemberCarrier::toString(void) {
	ostringstream oarchive_stream;
	{
		boost::archive::text_oarchive oa(oarchive_stream);
		oa << boost::serialization::make_nvp("GMemberCarrier", *this);
	} // serialization will be completed at end of scope, oa will be deleted

	return oarchive_stream.str();
}

/********************************************************************/
/**
 * Initializes this object (including possible contents) from a
 * description in a string. Makes use of Boost.Serialization.
 *
 * IMPORTANT: There are rumors that serialization is currently only
 * thread-safe when used with g++. If this is true, we need to
 * protect the serializationc alls below accordingly with a mutex.
 *
 * \param descr A string containing a description of the object
 */
void GMemberCarrier::fromString(const string& descr) {
	istringstream istr(descr);
	{
		boost::archive::text_iarchive ia(istr);
		ia >> boost::serialization::make_nvp("GMemberCarrier", *this);;
	}
}

/********************************************************************/

} /* namespace GenEvA */
