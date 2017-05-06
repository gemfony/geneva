#!/usr/bin/env bash

#!/bin/bash

####################################################################
#
# Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
#
# See the AUTHORS file in the top-level directory for a list of authors.
#
# Contact: contact [at] gemfony (dot) eu
#
# This file is part of the Geneva library collection.
#
# Geneva was developed with kind support from Karlsruhe Institute of
# Technology (KIT) and Steinbuch Centre for Computing (SCC). Further
# information about KIT and SCC can be found at http://www.kit.edu/english
# and http://scc.kit.edu .
#
# Geneva is free software: you can redistribute and/or modify it under
# the terms of version 3 of the GNU Affero General Public License
# as published by the Free Software Foundation.
#
# Geneva is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with the Geneva library. If not, see <http://www.gnu.org/licenses/>.
#
# For further information on Gemfony scientific and Geneva, visit
# http://www.gemfony.eu .
#
####################################################################
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

