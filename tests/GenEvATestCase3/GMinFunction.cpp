/**
 * \file
 */

/**
 * Copyright (C) 2004-2008 Dr. Ruediger Berlich
 * Copyright (C) 2007-2008 Forschungszentrum Karlsruhe GmbH
 * 
 * This file is part of the test cases for Geneva, Gemfony 
 * scientific's optimization library.
 * 
 * Geneva and this test case are free software: you can redistribute it and/or 
 * modify it under the terms of version 3 of the GNU Affero General Public License 
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

#include "GMinFunction.hpp"

// Included here so no conflicts occur. See explanation at
// http://www.boost.org/libs/serialization/doc/special.html#derivedpointers
#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(GenEvA::GMinFunction)

namespace GenEvA
{

  /**
   * A default constructor is required for the
   * serialization process.
   */
  GMinFunction::GMinFunction(void){
    /* nothing */
  }

  GMinFunction::GMinFunction(int nval)
    :GDoubleCollection(nval)
  { /* nothing */ }

  GMinFunction::GMinFunction(const GMinFunction &gmf)
    :GDoubleCollection(gmf)
  { /* nothing */ }

  double GMinFunction::customFitness(void){
    double result = 0.;
    vector<double>::iterator it;

    for(it=this->begin(); it!=this->end(); ++it){
      result += pow(*it,2);
    }

    return result;
  }

  void GMinFunction::customMutate(void){
    GDoubleCollection::customMutate();

    // cout << "h" << getParentPopGeneration() << "->Fill(" << this->at(0) << "," << this->at(1) << ");" << endl;
  }

  GObject *GMinFunction::clone(void){
    return new GMinFunction(*this);
  }

} /* namespace GenEvA */
