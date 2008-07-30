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

#include "GDoubleCollection.hpp"
#include "GRandom.hpp"

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/tracking.hpp>
#include <boost/serialization/split_member.hpp>

#include <iostream>
#include <cmath>

#ifndef GMINFUNCTION_H_
#define GMINFUNCTION_H_

using namespace std;
using namespace GenEvA;

namespace GenEvA
{

  class GMinFunction
    :public GDoubleCollection
  {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version){
      using boost::serialization::make_nvp;
      ar & make_nvp("GMFGDoubleCollection", boost::serialization::base_object<GDoubleCollection>(*this));
    }
    ///////////////////////////////////////////////////////////////////////
	
  public:
    GMinFunction(void);
    GMinFunction(int nval);
    GMinFunction(const GMinFunction &gmf);
    virtual ~GMinFunction(){/* nothing */}
    virtual double customFitness(void);
    virtual void customMutate(void);

    virtual GObject *clone(void);
  };

} /* namespace GenEvA */

#endif /*GMINFUNCTION_H_*/
