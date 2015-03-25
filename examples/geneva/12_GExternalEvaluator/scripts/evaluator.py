#!/usr/bin/python

################################################################################
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
################################################################################

import argparse
import os
import sys
import xml.etree.ElementTree as ET

from datetime import datetime
from textwrap import dedent


def init():
    """
    Perform initialization work, for instance to set up the optimization environment.
    """
    print("Initializing the optimization environment...")
    # Nothing to do...

def finalize():
    """
    Perform finalization work, for instance to clean up the optimization environment.
    """
    print("Cleaning up the optimization environment...")
    # Nothing to do...

def setup(setup_file, initial_values):
    """
    Provide the problem setup data inside a file of the given name.
    """

    # We write the XML "by-hand", as ElementTree does not support "pretty-printing"
    header = """\
    <!--
        Problem setup file created by """ + command_name() + """
        on """ + current_time() + """
    -->
    <batch>
      <dataType>setup_data</dataType>
      <runID>0</runID>
      <nIndividuals>1</nIndividuals>
      <individuals>
        <individual0>
          <type>GParameterSet</type>
          <nVars>4</nVars>
          <vars>\n"""

    content = header
    for i in range(4):
        content += "            <var" + str(i) + ">\n"
        content += "              <name>coord_" + str(i) + "</name>\n"
        content += "              <type>GConstrainedDoubleObject</type>\n"
        content += "              <baseType>double</baseType>\n"
        content += "              <isLeaf>true</isLeaf>\n"
        content += "              <nVals>1</nVals>\n"
        if initial_values == "min":
            content += "              <values>\n"
            content += "                <value0>-10.0</value0>\n"
            content += "              </values>\n"
            content += "              <lowerBoundary>-10.0</lowerBoundary>\n"
            content += "              <upperBoundary>10.0</upperBoundary>\n"
            content += "              <initRandom>false</initRandom>\n"
        elif initial_values == "max":
            content += "              <values>\n"
            content += "                <value0>10.0</value0>\n"
            content += "              </values>\n"
            content += "              <lowerBoundary>-10.0</lowerBoundary>\n"
            content += "              <upperBoundary>10.0</upperBoundary>\n"
            content += "              <initRandom>false</initRandom>\n"
        else:
            content += "              <values>\n"
            content += "                <value0>0.0</value0>\n"
            content += "              </values>\n"
            content += "              <lowerBoundary>-10.0</lowerBoundary>\n"
            content += "              <upperBoundary>10.0</upperBoundary>\n"
            content += "              <initRandom>true</initRandom>\n"
        content += "          </var" + str(i) + ">\n"

    content += """\
          </vars>
          <nBounds>0</nBounds>
          <nResults>1</nResults>
        </individual0>
      </individuals>
    </batch>
    """

    with open(setup_file, 'w') as infile:
        infile.write(dedent(content))

def evaluate(in_file, out_file):
    """
    Evaluate the result for the given input parameters, and
    generate an output file.
    """
    (iteration, it_id, result) = process_input(in_file)
    write_output(out_file, iteration, it_id, result)

def archive(in_file):
    """
    Evaluate the result for the given input parameters, and
    archive the results.
    """
    (iteration, it_id, result) = process_input(in_file)
    archive_output(iteration, it_id, result)

def process_input(in_file):
    """
    Read the input data and process it to get the results.
    """
    (iteration, it_id, x, y, z, w) = read_input(in_file)

    # This is an example, we just calculate a 4D parabola...
    result = x**2 + y**2 + w**2 + z**2

    return (iteration, it_id, result)

def write_output(out_file, iteration, it_id, result):
    """
    Write the output file containing the calculated results.
    """
    # We write the XML "by-hand", as ElementTree does not support "pretty-printing"
    header = """\
    <!--
        Results file created by """ + command_name() + """
        on """ + current_time() + """
        for iteration """ + str(iteration) + """
    -->
    <batch>
      <dataType>run_results</dataType>
      <runID>0</runID>
      <nIndividuals>1</nIndividuals>
      <individuals>
        <individual0>
          <iteration>""" + str(iteration) + """</iteration>
          <id>""" + str(it_id) + """</id>
          <isValid>true</isValid>
          <isDirty>false</isDirty>
          <nResults>1</nResults>
          <results>
            <rawResult0>""" + str(result) + """</rawResult0>
          </results>
        </individual0>
      </individuals>
    </batch>
    """
    with open(out_file, 'w') as ofile:
        ofile.write(dedent(header))

def archive_output(iteration, it_id, result):
    """
    Archive the calculated results (for instance, this could mean
    to store the results in a DB).
    """
    print("Archiving the results...")
    # Nothing to do at the moment

def read_input(in_file):
    """
    Read the input file and extract the parameters for the run.
    """

    xmltree = None
    try:
        with open(in_file) as ifile:
            xmltree = ET.parse(ifile)
    except ET.ParseError as e:
        sys.exit("\nERROR: parsing error on input file '"
                 + in_file.name + "':\n\t" + str(e))

    root = xmltree.getroot()
    #
    # More error handling needed here, in case the XML file
    # doesn't have the expected structure
    #
    ind = root.find("./individuals/individual0")
    try:
        iteration = int(ind.find("./iteration").text)
    except:
        iteration = -1

    try:
        it_id = ind.find("./id").text
    except:
        it_id = "UNKNOWN_ID"

    # nr_results = int(root.find("./nResults").text)
    nr_params = int(ind.find("./nVars").text)

    # if nr_results != 1:
    #    sys.exit("\nERROR: unexpected parameter: nResults=" + str(nr_results) + " (expected 1)!")
    if nr_params != 4:
        sys.exit("\nERROR: unexpected parameter: nVars=" + str(nr_params) + " (expected 4)!")

    params = ind.findall(".//*[isLeaf]")
    if len(params) != nr_params:
        sys.exit("\nERROR: inconsistent data in input file: nVars=" + str(nr_params)
                 + ", but found " + str(len(params)) + " parameters!")

    x = float(ind.find("./vars/var0").find("values/value0").text)
    y = float(ind.find("./vars/var1").find("values/value0").text)
    z = float(ind.find("./vars/var2").find("values/value0").text)
    w = float(ind.find("./vars/var3").find("values/value0").text)

    return (iteration, it_id, x, y, z, w)


def main(argv):
    """
    This script encapsulates all the problem-specific information for an optimization
    problem. It is called by the optimizer to evaluate some parameter set, and provides
    the results in an output file.
    """

    parser = argparse.ArgumentParser(description="External evaluator")

    parser.add_argument("--initvalues", help="Start with the given initial values,"
                        " either 'min' or 'max' (default: 'random')")

    group = parser.add_mutually_exclusive_group(required=True)
    group.add_argument("--init", help="Perform initialization work",
                       action="store_true")
    group.add_argument("--finalize", help="Perform finalization work",
                       action="store_true")
    group.add_argument("--setup", help="Provide initialization data to the caller",
                       action="store_true")
    group.add_argument("--evaluate", help="Evaluate the parameters from the input file",
                       action="store_true")
    group.add_argument("--archive", help="Archive the results",
                       action="store_true")

    parser.add_argument("--data", help="Read the input data from the given file"
                        " (default: 'data.xml')",
                        metavar='IN_FILE', default="data.xml")
    parser.add_argument("--result", help="Write the results to the given file"
                        " (default: 'output.xml')",
                        metavar='OUT_FILE', default="output.xml")

    args = parser.parse_args()

    # These options checks are not supported by argparse (in Python 2.7 at least)
    if args.initvalues and not args.setup:
        sys.exit('The option --initvalues may only be used with --setup! Aborting...')

    initial_values = args.initvalues or "random"
    if initial_values not in set(["min", "max", "random"]):
        sys.exit("Wrong --initvalues value, use either 'min' or 'max'! Aborting...")

    # Now do our job
    if args.init:
        init()
    elif args.finalize:
        finalize()
    elif args.setup:
        setup(args.result, initial_values)
    elif args.evaluate:
        evaluate(args.data, args.result)
    elif args.archive:
        archive(args.data)
    else:
        sys.exit("ERROR: inconsistent options! why are we here?")


def command_name():
    """Return a cleaned up name of this program."""
    return os.path.basename(sys.argv[0])

def current_time():
    """Return the current date/time as a string."""
    return str(datetime.now())

if __name__ == "__main__":
    main(sys.argv[1:])
