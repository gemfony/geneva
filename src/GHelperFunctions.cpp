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

#include "GHelperFunctions.hpp"

namespace GenEvA
{

  /**********************************************************************************/
  /**
   * Emits a number of white space characters. Used for indention of log output
   *
   * \param indention The number of white spaces to emit
   * \return A string with a given number of white spaces
   */
  string ws(uint16_t indention)
  {
    register uint16_t i;
    string result = "";
    for(i=0; i<indention; i++) result += " ";
    return result;
  }

  /**********************************************************************************/
  /**
   * Converts a double value into a string of 0s and 1s. Note that this function
   * is deprectaed, as conversion is now done mostly with the boost::lexical_cast
   * function.
   * 
   * \param value The value to be converted
   * \return A string representation of the argument
   */
  string d2s(double value){
    int16_t varSize=sizeof(double);
    string result(varSize*8,' ');

    // does "unsigned char" have the correct size ?
    if(sizeof(unsigned char) != 1){
      GLogStreamer gls;
      gls << "In string d2s(double): Error!" << endl
	  << "unsigned char has invalid size : " << sizeof(unsigned char) << endl
	  << logLevel(CRITICAL);

      exit(1);
    }

    unsigned char *probe;
    int16_t i,j;

    // attach probe to beginning of 'value'
    probe=(unsigned char *)(&value);
    for(i=0; i<varSize; i++){
      for(j=0; j<8; j++){
	if((*probe) & (1 << j)) result.at(i*8+j)='1';
	else result.at(i*8+j)='0';
      }

      probe++;
    }

    return result;
  }

  /**********************************************************************************/
  /**
   * Converts a string of 0s and 1s into a double value.
   * 
   * \param str The string to be converted into a double
   * \return The double created from the string.
   */
  double s2d(string str)
  {
    int16_t i, j, size_str=str.size(), varSize=sizeof(double);
    unsigned char *probe;

    double result=0;

    // check size of string - must be suitable for conversion into double
    if(size_str != sizeof(double)*8){
      GLogStreamer gls;
      gls << "In double s2d(string): Error!" << endl
	  << "string has invalid size : " << str << " " << size_str << endl
	  << logLevel(CRITICAL);

      exit(1);
    }

    // does "unsigned char" have the correct size ?
    if(sizeof(unsigned char) != 1){
      GLogStreamer gls;
      gls << "In double s2d(string): Error!" << endl
	  << "unsigned char has invalid size : " << sizeof(unsigned char) << endl
	  << logLevel(CRITICAL);

      exit(1);	  
    }

    // attach probe to the beginning of result
    probe=(unsigned char *)(&result);
    // make sure all bits are set to 0
    *probe=0;

    for(i=0; i<varSize; i++){
      for(j=0; j<8; j++){
	if(str.at(i*8+j)=='1') (*probe) |= (1 << j); // switch on all bits that are needed
      }
      probe++;
      *probe=0;
    }

    return result;
  }

  /**********************************************************************************/
  /**
   * Find the smallest double for which x+getMinDouble(x) > x
   * Attention : This implementation seems to be close to the smallest
   * distinguishable double, but apparently still returns slightly too large values.
   * 
   * \param val double value for which to search for smallest larger neighbour
   * \return Smallest distinguishable double (for "val").
   */

  double getMinDouble(double val){
    double result;
    int exponent; // we cannot use int16_t here - frexp complains ...

    frexp(val,&exponent);
    result=pow(2.,exponent-DBL_MANT_DIG);

    return result;
  }

  /**********************************************************************************/
  /**
   * This function compares the last s2.size() characters of s1 with s2 .
   * 
   * \param s1 first comparison string
   * \param s2 second comparison string
   * \return true if s1 ends with s2
   */
  bool endsWith(string s1, string s2){
    if(s1.find(s2,s1.size()-s2.size())==string::npos) return false;
    return true;
  }

  /**********************************************************************************/
  /**
   * This function calculates the checksum of a string and returns it as a string
   * of size "checksum_length" (an enum from the header file). This function uses 
   * Boost's CRC implementation.
   * 
   * \param str The string for which the checksum should be calculated
   * \return The checksum in string format
   */
  /*
  string checksum(const string& str){
    const uint16_t buffer_size=1024;
    boost::crc_32_type  result;
    std::istringstream  iss(str);
    
    string cs = "";

    if ( iss ) {
      do {
	char  buffer[ buffer_size ];	    
	iss.read( buffer, buffer_size );
	result.process_bytes( buffer, iss.gcount() );
      } while ( iss );

      std::ostringstream oss;
      oss << std::setw(checksum_length) << std::hex << result.checksum();
      cs = oss.str();

      if(cs.size() != checksum_length){
	GLogStreamer gls;
	gls << "In checksum() : Bad checksum size" << "Bad checksum size " << cs.size() << " " << cs << endl
	    << logLevel(CRITICAL);

	exit(1);
      }
    }

    return cs;
  }
  */

  /**********************************************************************************/
  /**
   * Assembles a query string from a given command, emitting a string of a given size.
   * This function is used in conjunction with Boost::Asio .
   * 
   * \param str The string from which the size should be extracted
   * \param sz Resulting size of the query string
   * \return The query string
   */
  string assembleQueryString(const string& query, uint16_t sz){
    std::ostringstream query_stream;
    query_stream << std::setw(sz) << query;
    return query_stream.str();
  }

  /**********************************************************************************/
  /**
   * Extracts the size of ASIO's data section from a C string. See GAsioTCPClient .
   * Used in conjunction with Boost::Asio.
   * 
   * \param ds The data string holding the data size
   * \return The size if the data
   */
  size_t extractDataSize(const char* ds){
    istringstream is(string(ds, header_length));
    size_t inboundDataSize = 0;
    if (!(is >> std::hex >> inboundDataSize)) {
      // Header doesn't seem to be valid. Inform the caller.
      GLogStreamer gls;
      gls << "In extractDataSize: Got invalid header!" << endl
	  << logLevel(CRITICAL);

      exit(1);
    }

    return inboundDataSize;
  }

  /**********************************************************************************/

} /* namespace GenEvA */
