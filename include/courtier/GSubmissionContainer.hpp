/**
 * @file GSubmissionContainer.hpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) com
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
 * http://www.gemfony.com .
 */

// Standard headers go here

// Includes check for correct Boost version(s)
#include "common/GGlobalDefines.hpp"

// Boost headers go here

#ifndef GSUBMISSIONCONTAINERBASE_HPP_
#define GSUBMISSIONCONTAINERBASE_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif

// Geneva headers go here
#include "courtier/GCourtierEnums.hpp"


namespace Gem {
namespace Courtier {

/**********************************************************************************************/
/**
 * This class can serve as a base class for items to be submitted through the broker. You need to
 * re-implement the purely virtual functions in derived classes. Note that it is mandatory for
 * derived classes to be serializable in order to benefit from networked execution.
 */
class GSubmissionContainer {
	// Add serialization code here

public:
	GSubmissionContainer();
	GSubmissionContainer(const GSubmissionContainer&);
	virtual ~GSubmissionContainer();

	virtual void process() = 0;

	/** @brief Allows the courtier library to associate an id with the container */
	void setCourtierId(const std::pair<Gem::Courtier::ID_TYPE_1, Gem::Courtier::ID_TYPE_2>&);
	/** @brief Allows to retrieve the courtier-id associated with this container */
	std::pair<Gem::Courtier::ID_TYPE_1, Gem::Courtier::ID_TYPE_2> getCourtierId() const;

private:
    /** @brief A two-part id that can be assigned to this container object */
    std::pair<Gem::Courtier::ID_TYPE_1, Gem::Courtier::ID_TYPE_2> id_;
};


} /* Courtier */
} /* Gem */

#endif /* GSUBMISSIONCONTAINERBASE_HPP_ */
