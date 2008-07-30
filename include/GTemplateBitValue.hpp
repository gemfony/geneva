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

#include <boost/version.hpp>

#if BOOST_VERSION < 103600
#error "Error: Boost should at least have version 1.36 !"
#endif /* BOOST_VERSION */

#include <boost/cstdint.hpp>

#ifndef GTEMPLATEBITVALUE_H_
#define GTEMPLATEBITVALUE_H_

using namespace std;

#include "GBitCollection.hpp"

namespace GenEvA
{

  /************************************************************************/

  template <class T>
  class GTemplateBitValue
    :public GenEvA::GBitCollection
  {
    ///////
    // Need to work on serialization, once this class is finished
    ///////
  public:
    GTemplateBitValue();
    explicit GTemplateBitValue(T val);
    GTemplateBitValue(const GTemplateBitValue& cp);
    virtual ~GTemplateBitValue();

    void operator=(const GTemplateBitValue& cp);

    virtual void reset(void);
    virtual void load(const GObject * gm);
    virtual GObject *clone(void);

    virtual void setValue(const T& val); // ??? wird benötigt ?
    virtual const T getValue(void);
    const T& getInternalValue(void);
    virtual void setInternalValue(const T& val);

  protected:
    virtual double customFitness(void);
    virtual void customMutate(void);

  };

  /************************************************************************/

  /************************************************************************/

  /************************************************************************/

  /************************************************************************/

  /************************************************************************/

}

#endif /*GTEMPLATEBITVALUE_H_*/
