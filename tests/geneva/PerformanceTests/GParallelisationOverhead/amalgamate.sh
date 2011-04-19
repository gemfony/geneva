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
# This script will take two "short" results of delay measurements
# of equal length and produce a speedup graph. There are exactly
# two arguments: the files holding the serial and the parallel measurements.
####################################################################
# !/bin/bash
#

# Set some variables
RESULTFILE=result.C

# Check that the number of command line options is at exactly two 
# (the names of the files holding the serial and the parallel measurement).
if [ $# -ne 2 ]; then
	echo "Usage: ./amalgamate.sh <serial result file> <parallel result file>"
	exit
fi

# Check that both files exist
if [ ! -e $1 ]; then
    echo "Result file $1 with serial results cannot be opened. Leaving"
    exit
fi

if [ ! -e $2 ]; then
    echo "Result file $2 with parallel results cannot be opened. Leaving"
    exit
fi

# Read in the serial timings
mcnt=0
nMeasurements=0
while read -r measurement; do
  	delays[${mcnt}]=`echo $measurement | awk -F"/" '{ print $1 }'`
	serial[${mcnt}]=`echo $measurement | awk -F"/" '{ print $2 }'`
	let mcnt=${mcnt}+1
done < $1

# Extract the number of measurements
nMeasurements=${mcnt}

# Read in the parallel timings
mcnt=0
while read -r measurement; do
	parallel[${mcnt}]=`echo $measurement | awk -F"/" '{ print $2 }'`
	let mcnt=${mcnt}+1
done < $2

# Cross check the number of measurements
if [ ! ${nMeasurements} -eq ${mcnt} ]; then
    echo "Files $1 and $2 contain different numbers"
    echo "of measurements: ${nMeasurements}/${mcnt}." 
    echo "Leaving."
    exit
fi

####################################################################
# Emit the result file

# The header
echo "{" > ${RESULTFILE}
echo "  gStyle->SetOptTitle(0);" >> ${RESULTFILE}
echo "  TCanvas *cc = new TCanvas(\"cc\",\"cc\",0,0,800,600);" >> ${RESULTFILE}
echo >> ${RESULTFILE}
echo "  double delays[$nMeasurements];" >> ${RESULTFILE}
echo "  double serial[$nMeasurements];" >> ${RESULTFILE}
echo "  double parallel[$nMeasurements];" >> ${RESULTFILE}
echo "  double serial_error[$nMeasurements];" >> ${RESULTFILE}
echo "  double parallel_error[$nMeasurements];" >> ${RESULTFILE}
echo "  double speedup[$nMeasurements];" >> ${RESULTFILE}
echo >> ${RESULTFILE}

# Fill the arrays
dcnt=0
for delay in "${delays[@]}"; do
    echo "  delays[$dcnt]=double($delay);" >> ${RESULTFILE}
    let dcnt=$dcnt+1
done

scnt=0
for s in "${serial[@]}"; do
    echo "  serial[$scnt]=double($s);" >> ${RESULTFILE}
    let scnt=$scnt+1
done

pcnt=0
for p in "${parallel[@]}"; do
    echo "  parallel[$pcnt]=double($p);" >> ${RESULTFILE}
    echo "  speedup[$pcnt]=double(${serial[$pcnt]})/double(${parallel[$pcnt]});" >> ${RESULTFILE}
    let pcnt=$pcnt+1
done

echo >> ${RESULTFILE}
echo "  TGraph *evGraph = new TGraph($nMeasurements, delays, speedup); " >> ${RESULTFILE}
echo >> ${RESULTFILE}
echo "  evGraph->SetMarkerStyle(2);" >> ${RESULTFILE}
echo "  evGraph->SetMarkerSize(1.0);" >> ${RESULTFILE}
echo "  evGraph->Draw(\"ACP\");" >> ${RESULTFILE}
echo "  evGraph->GetXaxis()->SetTitle(\"Evaluation time/individual [s]\");" >> ${RESULTFILE}
echo "  evGraph->GetYaxis()->SetTitle(\"Total processing time/ [s]\");" >> ${RESULTFILE}
echo "}" >> ${RESULTFILE}
