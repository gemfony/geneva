#!/usr/bin/python

################################################################################
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
################################################################################

import argparse
import sys
from os import path
from textwrap import dedent
from datetime import datetime
import xml.etree.ElementTree as ET


def init():
    """
    Perform initialization work, for instance to set up the optimization environment.
    """
    print("Initializing the optimization environment...")

def finalize():
    """
    Perform finalization work, for instance to clean up the optimization environment.
    """
    print("Cleaning up the optimization environment...")

def setup(setup_file):
    """
    Provide the problem setup data inside a file of the given name.
    """
    header = """\
    <!--
        Problem setup file created by """ + command_name() + """
        on """ + current_time() + """
    -->
    <parameterset>
        <nResultsExpected>1</nResultsExpected>
        <nvar>4</nvar>
        <type>GParameterSet</type>
        <var0>
            <nvar>1</nvar>
            <type>GConstrainedDoubleObject</type>
            <baseType>double</baseType>
            <isLeaf>true</isLeaf>
            <value0>-10.0</value0>
            <lowerBoundary>-10.0</lowerBoundary>
            <upperBoundary>10.0</upperBoundary>
            <initRandom>true</initRandom>
        </var0>
        <var1>
            <nvar>1</nvar>
            <type>GConstrainedDoubleObject</type>
            <baseType>double</baseType>
            <isLeaf>true</isLeaf>
            <value0>-10.0</value0>
            <lowerBoundary>-10.0</lowerBoundary>
            <upperBoundary>10.0</upperBoundary>
            <initRandom>true</initRandom>
        </var1>
        <var2>
            <nvar>1</nvar>
            <type>GConstrainedDoubleObject</type>
            <baseType>double</baseType>
            <isLeaf>true</isLeaf>
            <value0>-10.0</value0>
            <lowerBoundary>-10.0</lowerBoundary>
            <upperBoundary>10.0</upperBoundary>
            <initRandom>true</initRandom>
        </var2>
        <var3>
            <nvar>1</nvar>
            <type>GConstrainedDoubleObject</type>
            <baseType>double</baseType>
            <isLeaf>true</isLeaf>
            <value0>-10.0</value0>
            <lowerBoundary>-10.0</lowerBoundary>
            <upperBoundary>10.0</upperBoundary>
            <initRandom>true</initRandom>
        </var3>
    </parameterset>
    """
    setup_file.write(dedent(header))
    setup_file.close()

def evaluate_result(in_file, out_file):
    """
    Evaluate the result for the given input parameters, and
    generate an output file.
    """
    (iteration, result) = evaluate(in_file)
    write_output(out_file, iteration, result)

def evaluate_archive(in_file):
    """
    Evaluate the result for the given input parameters, and
    archive the results.
    """
    (iteration, result) = evaluate(in_file)
    archive_output(iteration, result)

def evaluate(in_file):
    """
    Evaluate the result for the given input parameters.
    """
    # print("Evaluating result for the given parameters...")
    (iteration, x, y, z, w) = read_input(in_file)

    # ATM we just calculate a parabola...
    result = x**2 + y**2 + w**2 + z**2

    #cmd = 'exe' + _params 
    #os.system(cmd)

    return (iteration, result)

def write_output(out_file, iteration, result):
    """
    Write the output file containing the calculated results.
    """
    # print("Generating the output file...")
    header = """\
    <!--
        Results file created by """ + command_name() + """
        on """ + current_time() + """
        for iteration #""" + str(iteration) + """
    -->
    <results>
        <isUseful>true</isUseful>
        <nresult>1</nresult>
        <result0>""" + str(result) + """</result0>
    </results>
    """
    out_file.write(dedent(header))
    out_file.close()

def archive_output(iteration, result):
    """
    Archive the calculated results.
    """
    print("Archiving the results...")
    # Nothing to do at the moment

def read_input(in_file):
    """
    Read the input file and extract the parameters for the run.
    """

    xmltree = None
    try:
        xmltree = ET.parse(in_file)
    except ET.ParseError as e:
        sys.exit("\nERROR: parsing error on input file '"
                 + in_file.name + "':\n\t" + str(e))

    root = xmltree.getroot()
    #
    # Error handling needed here
    #
    iteration = int(root.find("./iteration").text)
    # nr_results = int(root.find("./nResultsExpected").text)
    nr_params = int(root.find("./nvar").text)

    # if nr_results != 1:
    #    sys.exit("\nERROR: unexpected parameter: nResultsExpected=" + str(nr_results) + " (expected 1)!")
    if nr_params != 4:
        sys.exit("\nERROR: unexpected parameter: nvar=" + str(nr_params) + " (expected 4)!")

    params = root.findall(".//*[isLeaf]")
    if len(params) != nr_params:
        sys.exit("\nERROR: inconsistent data in input file: nvar=" + str(nr_params)
                 + ", but found " + str(len(params)) + " parameters!")

    x = float(root.find("./var0").find("value0").text)
    y = float(root.find("./var1").find("value0").text)
    z = float(root.find("./var2").find("value0").text)
    w = float(root.find("./var3").find("value0").text)

    return (iteration, x, y, z, w)


def main(argv):
    """
    This script encapsulates all the problem-specific information for an optimization
    problem. It is called by the optimizer to evaluate some parameter set, and provides
    the results in an output file.
    """

    parser = argparse.ArgumentParser(description="External evaluator")

    group = parser.add_mutually_exclusive_group(required=True)
    group.add_argument("--init", help="Perform initialization work",
                       action="store_true")
    group.add_argument("--finalize", help="Perform finalization work",
                       action="store_true")
    group.add_argument("--setup", help="Provide initialization data to the caller",
                       nargs='?', metavar='SETUP_FILE', const="setupFile.xml",
                       type=argparse.FileType('w'))
    group.add_argument("--evaluate", help="Evaluate the parameters from the input file"
                       " (requires --result or --archive)",
                       nargs='?', metavar='IN_FILE', const="parameters.xml",
                       type=argparse.FileType('r'))

    group_eval = parser.add_mutually_exclusive_group()
    group_eval.add_argument("--result", help="Write the results to the given file",
                            nargs='?', metavar='OUT_FILE', const="results.xml",
                            type=argparse.FileType('w'))
    group_eval.add_argument("--archive", help="Archive the results",
                            action="store_true")

    args = parser.parse_args()

    # These options checks are not supported by argparse (in Python 2.7 at least)
    if args.result or args.archive:
        if args.evaluate is None:
            sys.exit('--result and --archive can only be used with --evaluate! Aborting...')
    else:
        if args.evaluate:
            sys.exit('--evaluate requires either --result or --archive! Aborting...')

    # Now do our job
    if args.init:
        init()
    elif args.finalize:
        finalize()
    elif args.setup:
        setup(args.setup)
    elif args.evaluate:
        if args.result:
            evaluate_result(args.evaluate, args.result)
        elif args.archive:
            evaluate_archive(args.evaluate)
        else:
            sys.exit("ERROR: inconsistent options! why are we here?")
    else:
        sys.exit("ERROR: inconsistent options! why are we here?")


def command_name():
    """Return a cleaned up name of this program."""
    return path.basename(sys.argv[0])

def current_time():
    """Return the current date/time as a string."""
    return str(datetime.now())

if __name__ == "__main__":
    main(sys.argv[1:])
