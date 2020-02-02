#!/usr/bin/env bash

################################################################################
#
# This file is part of the Geneva library collection. The following license
# applies to this file:
#
# ------------------------------------------------------------------------------
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# ------------------------------------------------------------------------------
#
# Note that other files in the Geneva library collection may use a different
# license. Please see the licensing information in each file.
#
################################################################################
#
# Geneva was started by Dr. Rüdiger Berlich and was later maintained together
# with Dr. Ariel Garcia under the auspices of Gemfony scientific. For further
# information on Gemfony scientific, see http://www.gemfomy.eu .
#
# The majority of files in Geneva was released under the Apache license v2.0
# in February 2020.
#
# See the NOTICE file in the top-level directory of the Geneva library
# collection for a list of contributors and copyright information.
#
################################################################################
# This script will start a given Geneva program in client mode a
# predefined number of times. No server will be started.
# This can be used to test networked execution of a Geneva program under
# real-world conditions.
####################################################################

# Check the number of command line arguments (should be exactly 4)
if [ ! $# -eq 4 ]; then
    echo "Usage: ./startClients.sh <program name> <number of clients> <ip/hostname> <port>"
    exit 1
fi

# Read in the command line arguments
PROGNAME=$1
NCLIENTS=$2
IP=$3
PORT=$4

# Check that the program exists
if [ ! -e ${PROGNAME} ]; then
    echo "Error: Program file ${PROGNAME} does not exist."
    exit
fi

# Check that the desired number of clients is integral and >= 0
if [ ! $(echo "${NCLIENTS}" | grep -E "^[0-9]+$") ]; then
    echo "Error: Number of clients Number of clients \"${NCLIENTS}\" is not a valid integer. Leaving."
    exit
fi
if [ ! ${NCLIENTS} -gt 0 ];     then
    echo "Error: \"${NCLIENTS}\" should at least be 1. Leaving"
    exit
fi

# Check that the port number is integral and >= 1000
if [ ! $(echo "${PORT}" | grep -E "^[0-9]+$") ]; then
    echo "Error: Port \"${PORT}\" is not a valid integer. Leaving."
    exit
fi
if [ ${PORT} -le 1000 ];     then
    echo "Error: Port \"${PORT}\" should at least be 1001. Leaving"
    exit
fi

# Create an output directory
if [ ! -d ./output ]; then
    mkdir ./output
fi

# Start the clients
for i in `seq 1 $2`; do
    (./${PROGNAME} -e 2 -c tcpc --client --ip=${IP} --port=${PORT} >& ./output/output_client_$i) &
done

