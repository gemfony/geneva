/*
 * This file is part of the Geneva library collection.
 *
 * See the NOTICE file in the top-level directory of the Geneva library
 * collection for a list of contributors and copyright information.
 *
 * The following license applies to the code IN THIS FILE:
 *
 * ***************************************************************************
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * ***************************************************************************
 *
 * NOTE THAT THE BOOST-LICENSE DOES NOT APPLY TO ANY OTHER FILES OF THE
 * GENEVA LIBRARY, UNLESS THIS IS EXPLICITLY STATED IN THE CORRESPONDING FILE!
 */

#pragma once

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard header files go here
#include <string>
#include <sstream>
#include <stdexcept>

// Boost header files go here

// Geneva header files go here

/******************************************************************************/
// Exceptions and related definitions. Note that we do not use the Gem::Common
// namespace here for convenience reasons.

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * General exception class to be thrown in the case of severe errors
 * in the Geneva library collection.
 */
class gemfony_exception : public std::runtime_error
{
public:
	using std::runtime_error::runtime_error;
};

/******************************************************************************/
/** @brief This function allows to output a gemfony_exception to a stream */
G_API_COMMON std::ostream& operator<<(std::ostream&, const gemfony_exception&);

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * An exception to be thrown in case of an expectation violation. This is used
 * in the compare infrastructure, but listed here to resolve a circular dependency.
 */
class g_expectation_violation : public gemfony_exception
{
public:
	 using gemfony_exception::gemfony_exception;
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This define allows easy access to throwing exceptions.
 */
#define raiseException(E)                                                                                \
  {                                                                                                      \
    std::ostringstream error;                                                                            \
    error                                                                                                \
       << std::endl                                                                                      \
       << "================================================" << std::endl                                \
       << "ERROR" << std::endl                                                                           \
       << "in file " << __FILE__ << std::endl                                                            \
       << "near line " << __LINE__ << " with description:" << std::endl                                  \
       << std::endl                                                                                      \
       << E << std::endl                                                                                 \
       << std::endl                                                                                      \
       << "If you suspect that this error is due to Geneva," << std::endl                                \
       << "then please consider filing a bug via" << std::endl                                           \
       << "http://www.gemfony.eu (link \"Bug Reports\") or" << std::endl                                 \
       << "through http://www.launchpad.net/geneva" << std::endl                                         \
       << std::endl                                                                                      \
       << "We appreciate your help!" << std::endl                                                        \
       << "The Geneva team" << std::endl                                                                 \
       << "================================================" << std::endl;                               \
    throw(gemfony_exception(error.str()));                                            \
  }

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
