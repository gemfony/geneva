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

#include "GLowBitsAdaptor.hpp"

namespace GenEvA
{

  /***************************************************************************/

  GLowBitsAdaptor::GLowBitsAdaptor()
    :GTemplateAdaptor<bit>("GLowBitsAdaptor")
  {
  }

  /***************************************************************************/

  GLowBitsAdaptor::GLowBitsAdaptor(string nm)
    :GTemplateAdaptor<bit>(nm)
  {
  }

  /***************************************************************************/

  GLowBitsAdaptor::GLowBitsAdaptor(double prob)
    :GTemplateAdaptor<bit>("GLowBitsAdaptor")
  {
  }

  /***************************************************************************/

  GLowBitsAdaptor::GLowBitsAdaptor(const GLowBitsAdaptor& cp)
    :GTemplateAdaptor<bit>("GLowBitsAdaptor")
  {
    GLowBitsAdaptor::load(&cp);
  }

  /***************************************************************************/

  GLowBitsAdaptor::~GLowBitsAdaptor()
  {
  }

  /***************************************************************************/

  void GLowBitsAdaptor::reset(void)
  {
  }

  /***************************************************************************/

  void GLowBitsAdaptor::load(const GObject *gb)
  {
  }

  /***************************************************************************/

  GObject *GLowBitsAdaptor::clone(void)
  {
    return new GLowBitsAdaptor(*this);
  }

  /***************************************************************************/

  void GLowBitsAdaptor::customMutate(bit& val)
  {
  }


  /***************************************************************************/

  void GLowBitsAdaptor::setLowProbability(double val)
  {
  }

  /***************************************************************************/

  void GLowBitsAdaptor::setHighProbability(double val)
  {
  }

  /***************************************************************************/

}
