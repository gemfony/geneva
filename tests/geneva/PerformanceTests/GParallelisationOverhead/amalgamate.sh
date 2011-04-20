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
# of equal length and produce a speedup graph. There are three
# arguments: the files holding the serial and the parallel measurements,
# and the number of parallel processing units --this determines the
# maximum theoretical speedup.
####################################################################
# !/bin/bash
#

# Set some variables
RESULTFILE=speedup.C

# Check that the number of command line options is at exactly three 
# (the names of the files holding the serial and the parallel measurement,
# plus the number of parallel processing units
if [ $# -ne 3 ]; then
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

# Check that the number of parallel processing units is integral and > 0
if [ ! $(echo "$3" | grep -E "^[0-9]+$") ]; then
    echo "Error: $3 is not a valid integer. Leaving."
    exit
fi
if [ ! $3 -gt 0 ];     then
    echo "Error: $3 should at least be 1. Leaving"
    exit
fi

# Assign the number of processing units
nProcUnits=$3

# Read in the serial timings
mcnt=0
nMeasurements=0
while read -r measurement; do
  	delays[${mcnt}]=`echo $measurement | awk -F"/" '{ print $1 }'`
	serial[${mcnt}]=`echo $measurement | awk -F"/" '{ print $2 }'`
	sdSerial[${mcnt}]=`echo $measurement | awk -F"/" '{ print $3 }'` # The standard deviation
	let mcnt=${mcnt}+1
done < $1

# Extract the number of measurements
nMeasurements=${mcnt}

# Read in the parallel timings
mcnt=0
while read -r measurement; do
	parallel[${mcnt}]=`echo $measurement | awk -F"/" '{ print $2 }'`
	sdParallel[${mcnt}]=`echo $measurement | awk -F"/" '{ print $3 }'`
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
echo "  double speedup_error[$nMeasurements];" >> ${RESULTFILE}
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

secnt=0 # Serial error counter
for se in "${sdSerial[@]}"; do
    echo "  serial_error[$secnt]=double($se);" >> ${RESULTFILE}
    let secnt=$secnt+1
done

pcnt=0
for p in "${parallel[@]}"; do
    echo "  parallel[$pcnt]=double($p);" >> ${RESULTFILE}
    echo "  speedup[$pcnt]=double(${serial[$pcnt]})/double(${parallel[$pcnt]});" >> ${RESULTFILE}
    echo "  speedup[$pcnt] /= double(${nProcUnits});" >> ${RESULTFILE}
    let pcnt=$pcnt+1
done

pecnt=0 # Parallel error counter
for pe in "${sdParallel[@]}"; do
    echo "  parallel_error[$pecnt]=double($pe);" >> ${RESULTFILE}
    let pecnt=$pecnt+1	
done

let upperLimit=${nMeasurements}-1
for spe in `seq 1 ${upperLimit}`; do
	echo "  speedup_error[$spe] = sqrt(pow(${serial[$spe]}*${sdParallel[$spe]}/pow(${parallel[$spe]}, 2), 2) + pow(${sdSerial[$spe]}/${parallel[$spe]}, 2));" >> ${RESULTFILE}
	echo "  speedup_error[$spe] /= double(${nProcUnits});" >> ${RESULTFILE}
done

# Emit the footer
echo >> ${RESULTFILE}
echo "  TGraphErrors *evGraph = new TGraphErrors($nMeasurements, delays, speedup, 0, speedup_error); " >> ${RESULTFILE}
echo >> ${RESULTFILE}
echo "  evGraph->SetMarkerStyle(2);" >> ${RESULTFILE}
echo "  evGraph->SetMarkerSize(1.0);" >> ${RESULTFILE}
echo "  evGraph->Draw(\"AC\");" >> ${RESULTFILE}
echo "  TLine *tl1 = new TLine(0.,1.,132, 1.);" >> ${RESULTFILE}
echo "  tl1->SetLineStyle(2);" >> ${RESULTFILE}
echo "  tl1->Draw();" >> ${RESULTFILE}
echo "  TLatex *tt1 = new TLatex(20, 1.01, \"Speedup normalized to number of workers \#rightarrow upper limit is 1\");" >> ${RESULTFILE}
echo "  tt1->SetTextSize(0.03);" >> ${RESULTFILE}
echo "  tt1->Draw();" >> ${RESULTFILE}
echo "  evGraph->GetXaxis()->SetTitle(\"Evaluation time/individual [s]\");" >> ${RESULTFILE}
echo "  evGraph->GetYaxis()->SetTitle(\"Speedup for ${nProcUnits} parallel workers\");" >> ${RESULTFILE}
echo "}" >> ${RESULTFILE}
