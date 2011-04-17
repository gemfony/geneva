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
# This script will sync a local directory with another directory 
# on a list of remote hosts specified in a file. Note that it may be 
# useful to use ssh-agent or to avoid specifying a passphrase during 
# key generation in order not having to enter the passphrase for each 
# ssh call. You can call this script like this:
# ./rsyncList.sh -av -e ssh ./ myHostFile.hcfg:/home/user/somedir --delete-after
# Or in other words: Instead of specifying a remote host name, you
# provide the script with the name of a file holding host names.
# The host file should list one name of a remote host per line. Its
# extension MUST be ".hcfg".