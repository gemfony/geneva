/**
 * @file GExternalEvaluatorConsumer.cpp
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

#include "geneva/GExternalEvaluatorConsumer.hpp"

namespace Gem {
namespace Geneva {

/*******************************************************************/
/**
 * The default constructor
 */
GExternalEvaluatorConsumer::GExternalEvaluatorConsumer()
{ /* nothing */ }

/*******************************************************************/
/**
 * The destructor.
 */
GExternalEvaluatorConsumer::~GExternalEvaluatorConsumer()
{ /* nothing */ }

/*******************************************************************/
/**
 * An overloaded version of GBoostThreadConsumerT<>'s main processing
 * function. It converts the GIndividual object into a GExternalEvaluatorIndividual
 * object and hands it to a custom processing function, which needs to be
 * specified by derived classes.
 */
void GExternalEvaluatorConsumer::processItems(boost::shared_ptr<Gem::Geneva::GIndividual> p) {
	boost::shared_ptr<GExternalEvaluatorIndividual> p_conv;

#ifdef DEBUG
	p_conv = boost::dynamic_pointer_cast<GExternalEvaluatorIndividual>(p);
	if(!p_conv) {
		raiseException(
				"In GExternalEvaluatorConsumer::processItems(): Error!" << std::endl
				<< "Conversion to GExternalEvaluatorConsumer failed." << std::endl
		);
	}
#else
	p_conv = boost::static_pointer_cast<GExternalEvaluatorIndividual>(p);
#endif /* DEBUG */

	// Initiate the actual processing
	customProcessItems(p_conv);
}

/*******************************************************************/
/**
 * Returns a name for this consumer
 *
 * @return A unique identifier for this consumer
 */
std::string GExternalEvaluatorConsumer::getConsumerName() const {
	return std::string("GExternalEvaluatorConsumer");
}

/*******************************************************************/

} /* namespace Geneva */
} /* namespace Gem */
