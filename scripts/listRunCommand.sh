####################################################################
#
# Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
#
# See the AUTHORS file in the top-level directory for a list of authors.
#
# Contact: contact [at] gemfony (dot) com
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
# http://www.gemfony.com .
#
####################################################################
# This script will read a list of hosts from the file specified with
# the first command line option. It will then call the ssh program for
# all hosts specified there. All other command line arguments given to 
# this script will be passed verbatim the each ssh call. 
# Note that it may be useful to use ssh-agent or not to specify
# any passphrase at all during key generation in order not having to
# enter the passphrase for each ssh call. 
# You can call this script like this:
# ./listRunCommand.sh ./myHostsFile -p 24 -l myUserName myCommand
# The host file should list one name of a remote host per line.
# The script will stop execution when one of the ssh calls fails.
####################################################################
# !/bin/bash
#
# Check that the number of command line options is at least two 
# (the name of the hosts file and the command to be executed remotely).
if [ $# -lt 2 ]; then
	echo "Usage: ./listRunCommand.sh <hostFile> <command>"
fi

# Check that the hosts file exists
if [ ! -e $1 ]; then
	echo "Error: File $1 does not exist"
fi

# Read the host names into a list

HOSTS[0]="141.52.7.30"

# Assemble the command string and execute it for each host in the list
for host in "${HOSTS[@]}"; do
	COMMANDSTRING="ssh "
done
