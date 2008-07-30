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

  /******************************************************************************************/

  GMinFunction::GMinFunction(void) :GDoubleCollection()
  { /* nothing */ }

  /******************************************************************************************/

  GMinFunction::GMinFunction(int nval, double min, double max, const vector<double>& values)
    :GDoubleCollection(nval, min, max)
  {
    myData.clear();
    vector<double>::const_iterator it;
    for(it=values.begin(); it!=values.end(); ++it) myData.push_back(*it); 
  }

  /******************************************************************************************/

  GMinFunction::GMinFunction(const GMinFunction &gmf)
    :GDoubleCollection(gmf)
  {
    myData.clear();
    vector<double>::const_iterator it;
    for(it=gmf.myData.begin(); it!=gmf.myData.end(); ++it) myData.push_back(*it); 
  }

  /******************************************************************************************/
  /**
   * This calculation follows the formula on page 11 of Evolutionaere Algorithmen by 
   * Ingrid Gerdes, Frank Klawonn and Rudolf Krause (Vieweg Verlag). It extends this
   * function to arbitrary target dimensions (smaller or equal the original dimension
   * of the data).
   */
  double GMinFunction::customFitness(void){
    int i,j,k;
    double enumerator = 0.;
    double denominator = 0.;
	
    for(i=0; i<NDATA; i++){
      for(j=i+1; j<NDATA; j++){
	double targetVal = 0;
	double origVal = 0;

	for(k=0; k<NDIMTARGET; k++){
	  targetVal += pow(this->at(i*NDIMTARGET+k) - this->at(j*NDIMTARGET+k),2);
	}
			
	for(k=0; k<NDIMORIG; k++){
	  origVal += pow(myData.at(i*NDIMORIG+k) - myData.at(j*NDIMORIG+k),2);
	}

	denominator += origVal;
			
	targetVal = sqrt(targetVal);
	origVal = sqrt(origVal);
			
	enumerator += pow(targetVal-origVal, 2);
      }
    }
	
    return enumerator/std::max(denominator,0.000000000000001); 
  }

  /******************************************************************************************/

  GObject *GMinFunction::clone(void){
    return new GMinFunction(*this);
  }

  /******************************************************************************************/

  void GMinFunction::load(const GObject *cp){
    // Check that this object is not accidently assigned to itself. 
    // As we do not actually do any calls with this pointer, we
    // can use the faster static_cast<>
    if(static_cast<const GMinFunction *>(cp) == this){
      GException ge;
      ge << "In GMinFunction::load(): Error!" << endl
	 << "Tried to assign an object to itself." << endl
	 << raiseException;
    }
	
    GDoubleCollection::load(cp);

    const GMinFunction *gmf = dynamic_cast<const GMinFunction *>(cp);
    if(!gmf){
      GException ge;
      ge << "In GMinFunction::load() : Conversion error!" << endl
	 << raiseException;
    }

    myData.clear();
    vector<double>::const_iterator it;
    for(it=gmf->myData.begin(); it!=gmf->myData.end(); ++it) myData.push_back(*it); 
  }

  /******************************************************************************************/

  const GMinFunction& GMinFunction::operator=(const GMinFunction& cp){
    GMinFunction::load(&cp);
    return *this;
  }

  /******************************************************************************************/

  const GObject& GMinFunction::operator=(const GObject& cp){
    GMinFunction::load(&cp);
    return *this;
  }

  /******************************************************************************************/

  void GMinFunction::print(void){
    cout << "{" << endl
	 << "TH2F *h2 = new TH2F(\"h2\",\"h\",100,-4,4,100,-4,4);" << endl;
	
    for(int i=0; i<NDATA; i++){
      cout << "h2->Fill(" << this->at(i*NDIMTARGET) << "," << this->at(i*NDIMTARGET + 1) << ");" << endl;
    }
	
    cout << "h2->Draw();" << endl
	 << "}" << endl;
  }

  /******************************************************************************************/

} /* namespace GenEvA */
