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

#include "GMutable.hpp"
#include "GDoubleCollection.hpp"
#include "GRandom.hpp"

#include <iostream>
#include <cmath>

#ifndef GMINFUNCTION_H_
#define GMINFUNCTION_H_

using namespace std;

namespace GenEvA
{

  const int NDIMORIG = 4;
  const int NDIMTARGET = 2;
  const int NDATA = 2000;

  /******************************************************************************************/
  /**
   * This class calculates an n-dimensional representation of an m-dimensional space, 
   * where n<m .
   */
  class GMinFunction
    :public GDoubleCollection
  {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version){
      using boost::serialization::make_nvp;
      ar & make_nvp("GDoubleCollection", boost::serialization::base_object<GDoubleCollection>(*this));
      ar & make_nvp("myData",myData);
    }
    ///////////////////////////////////////////////////////////////////////

  public:
    GMinFunction(int nval, double min, double max, const vector<double>& values);
    GMinFunction(const GMinFunction &gmf);
    virtual ~GMinFunction(){/* nothing */}
	
    const GMinFunction& operator=(const GMinFunction& cp);
    const GObject& operator=(const GObject& cp);
	
    virtual double customFitness(void);

    virtual void load(const GObject *cp);
    virtual GObject *clone(void);

    void print(void);
	
  private:
    GMinFunction(void);
    vector<double> myData;
  };

  /******************************************************************************************/

} /* namespace GenEvA */

#endif /*GMINFUNCTION_H_*/
