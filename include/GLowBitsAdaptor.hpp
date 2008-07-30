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

#ifndef GLOWBITSADAPTOR_H_
#define GLOWBITSADAPTOR_H_

using namespace std;

#include "GObject.hpp"
#include "GTemplateAdaptor.hpp"
#include "GBitFlipAdaptor.hpp"
#include "GEnums.hpp"

namespace GenEvA
{

  /***************************************************************************/
  /**
   * Basic idea:
   * This class mutates lower bits with higher probability than
   * higher bits, thus favouring small value changes.
   *
   * Note: This is not likely to work easily for doubles due to the IEEE format,
   * (exponent vs. mantissa).
   *
   * We thus implement this later.
   */

  class GLowBitsAdaptor
    :public GTemplateAdaptor<bit>
  {
    //////
    // need to work on serialization, once this class is finished
    //////
  public:
    GLowBitsAdaptor();
    explicit GLowBitsAdaptor(string nm);
    explicit GLowBitsAdaptor(double prob);
    GLowBitsAdaptor(const GLowBitsAdaptor& cp);
    virtual ~GLowBitsAdaptor();

    virtual void reset(void);
    virtual void load(const GObject *gb);
    virtual GObject *clone(void);

    void setLowProbability(double val);
    void setHighProbability(double val);

  protected:
    virtual void customMutate(bit& val);
	
  private:
    GBitFlipAdaptor *gba;
  };

  /***************************************************************************/

}

#endif /*GLOWBITSADAPTOR_H_*/
