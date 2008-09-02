/**
 * @file GEnum.hpp
 */

/* Copyright (C) 2004-2008 Dr. Ruediger Berlich
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

#ifndef GENUMS_H_
#define GENUMS_H_

namespace Gem {
namespace GenEvA {
  /**
   * This enumeration is needed because
   * vector<bool>[i] returns a helper class,
   * which breaks GenEvA
   */
  enum bit {G_FALSE=0, G_TRUE=1}; // 0,1

  /**
   * Return values for member functions.
   */
  enum GExitCode {ExitOK = 0, ExitBad = 1};

  /**
   * The serialization modes that are currently allowed
   */
  enum serializationMode {TEXTSERIALIZATION=0,XMLSERIALIZATION=1,BINARYSERIALIZATION=2,DEFAULTSERIALIZATION=3};

  /**
   * Currently three types of recombination schemes are supported:
   * - DEFAULTRECOMBINE defaults to RANDOMRECOMBINE
   * - RANDOMRECOMBINE chooses the parents to be replicated randomly from all parents
   * - VALUERECOMBINE prefers parents with a higher fitness
   */
  enum recoScheme {DEFAULTRECOMBINE=0, RANDOMRECOMBINE=1, VALUERECOMBINE=2};

  /**
   * The info function can be called in these three modes
   */
  enum infoMode {INFOINIT=0,INFOPROCESSING=1,INFOEND=2};

} /* namespace GenEvA */
} /* namespace Gem */

#endif /*GENUMS_H_*/
