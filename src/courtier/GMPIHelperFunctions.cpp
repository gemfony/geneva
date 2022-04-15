/********************************************************************************
 *
 * This file is part of the Geneva library collection. The following license
 * applies to this file:
 *
 * ------------------------------------------------------------------------------
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * ------------------------------------------------------------------------------
 *
 * Note that other files in the Geneva library collection may use a different
 * license. Please see the licensing information in each file.
 *
 ********************************************************************************
 *
 * Geneva was started by Dr. Rüdiger Berlich and was later maintained together
 * with Dr. Ariel Garcia under the auspices of Gemfony scientific. For further
 * information on Gemfony scientific, see http://www.gemfomy.eu .
 *
 * The majority of files in Geneva was released under the Apache license v2.0
 * in February 2020.
 *
 * See the NOTICE file in the top-level directory of the Geneva library
 * collection for a list of contributors and copyright information.
 *
 ********************************************************************************/

#include "courtier/GMPIHelperFunctions.hpp"

/******************************************************************************/
/**
 * Returns the message size of a completed request.
 * The return value is undefined for the status on not yet completed requests.
 * Therefore you should call MPI_Test first.
 *
 * @param status a reference to an MPI status object
 * @param dataType the datatype of the MPI message. Defaults to MPI_CHAR
 * @return the amount of objects of type dataType transferred by the operation
 */
int mpiGetCount(const MPI_Status &status, MPI_Datatype dataType) {
    int count{};
    MPI_Get_count(&status, dataType, &count);

    return count;
}

/**
 * Converts an MPI error code to the corresponding string which explains the error
 *
 * @param mpiError integer error code used by MPI and typically stored in an MPI_Status.status field
 * @return a string corresponding to the error code
 */
std::string mpiErrorString(int mpiError) {
    char errorMessage[MPI_MAX_ERROR_STRING];
    int messageLength;
    MPI_Error_string(mpiError, errorMessage, &messageLength);

    // the string in errorMessage is null terminated, which allows us to pass it to the std::string constructor
    return std::string{errorMessage};
}
