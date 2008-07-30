/**
 * \file
 */

/**
 * Copyright (C) 2004-2008 Dr. Ruediger Berlich
 * Copyright (C) 2007-2008 Forschungszentrum Karlsruhe GmbH
 *
 * This file is part of the test cases for Geneva, Gemfony
 * scientific's optimization library.
 *
 * Geneva and this test case are free software: you can redistribute it and/or
 * modify it under the terms of version 3 of the GNU Affero General Public License
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

#include <iostream>
#include <string>
#include <sstream>

#include "GBasePopulation.hpp"
#include "GBoostThreadPopulation.hpp"
#include "GDoubleGaussAdaptor.hpp"

#include "GMinFunction.hpp"

using namespace std;
using namespace GenEvA;

const uint16_t NPARENTS = 2;

void infoFunction(GBasePopulation * const gbp){
  GLogStreamer gls;
  gls << setprecision(15)
      << "h" << gbp << "->Fill(" << gbp->getGeneration() << ", " << gbp->at(0)->getMyCurrentFitness() << ");" << endl;
  if(gbp->at(0)->isDirty()){
    gls << "// Attention: object carries the dirty flag!" << endl;
  }
  gls << logLevel(TRACK);
}

void exceptionHandler(const string& msg){
  cout << "In handler. Received message " << msg << endl;
  exit(1);
}

int main(int argc, char **argv)
{
  LOGGER.addTarget(new GDiskLogger());
  LOGGER.addTarget(new GConsoleLogger());
  LOGGER.addLogLevelsUpTo(TRACK);
  LOGGER.registerExceptionHandler(&exceptionHandler);

  GRANDOMFACTORY.setNProducerThreads(12,8);

  // GBasePopulation pop;
  GBoostThreadPopulation pop;
  pop.setMaxThreads(8);

  // GException ge;
  // ge.forceException();
  // ge << "Hello World" << endl << raiseException;

  for(uint16_t i=0; i<NPARENTS; i++){
    GMinFunction *gmf = new GMinFunction(1000);
    // We try out parents with different sigma
    GDoubleGaussAdaptor *gdga =
      new GDoubleGaussAdaptor(double(i+1)/NPARENTS,0.1*double(i+1)/NPARENTS, 0.02,"GDoubleGaussAdaptor");
    gmf->addAdaptor(gdga);
    pop.appendMember(gmf);
  }

  pop.setMaxGeneration(2000);
  pop.setMaximize(false);
  pop.setPopulationSize(100,NPARENTS);
  pop.setReportGeneration(1);
  pop.setMaxTime(0,0,0,0);
  pop.registerInfoFunction(infoFunction);
  pop.setRecombinationMethod(VALUERECOMBINE);

  pop.optimize();

  cout << "done ..." << endl;

  return 0;
}
