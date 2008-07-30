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
#include "GTransferPopulation.hpp"
#include "GMemberBroker.hpp"
#include "GBoostThreadConsumer.hpp"
#include "GMinFunction.hpp"
#include "GDoubleGaussAdaptor.hpp"
#include "GAsioTCPConsumer.hpp"
#include "GAsioTCPClient.hpp"

using namespace std;
using namespace GenEvA;

const unsigned int NPARENTSSUPER = 2;
const unsigned int NPARENTSSUB = 1;

void infoFunction(GBasePopulation * const gbp){
  GLogStreamer gls;
  gls << setprecision(15) << "h" << gbp << "->Fill(" << gbp->getGeneration() << ", " << gbp->at(0)->getMyCurrentFitness() << ");" << endl;
  if(gbp->at(0)->isDirty()){
    gls << "// Attention: object carries the dirty flag!" << endl;
  }
  gls << logLevel(TRACK);
}

int main(int argc, char **argv)
{
  LOGGER.addTarget(new GDiskLogger());
  LOGGER.addTarget(new GConsoleLogger());
  LOGGER.addLogLevel(EXCEPTION);
  LOGGER.addLogLevel(CRITICAL);
  LOGGER.addLogLevel(UNCRITICAL);
  LOGGER.addLogLevel(TRACK);
    	
  GBoostThreadConsumer gc;
  gc.setMaxThreads(5);

  GBoostThreadPopulation superPop;
  superPop.setMaxThreads(20);
	
  for(unsigned int i=0; i<NPARENTSSUPER; i++){
    GTransferPopulation *subPop = new GTransferPopulation();	
 
    for(unsigned int j=0; j<NPARENTSSUB; j++){
      GMinFunction *gmf = new GMinFunction(1000);
      // We try out parents with different sigma
      GDoubleGaussAdaptor *gdga = 
	new GDoubleGaussAdaptor(double(j+1)/NPARENTSSUB, 0.1*double(j+1)/NPARENTSSUB, 0.2,"GDoubleGaussAdaptor");
      gmf->addAdaptor(gdga);
      subPop->appendMember(gmf);
    }
        	
    subPop->setMaxGeneration(10);
    subPop->setMaximize(false);
    subPop->setPopulationSize(100,NPARENTSSUB);
    subPop->setReportGeneration(5);
    subPop->setMaxTime(0,0,0,0);	
    subPop->registerInfoFunction(infoFunction);
    	
    superPop.appendMember(subPop);
  }
    	
  superPop.setMaxGeneration(100);
  superPop.setMaximize(false);
  superPop.setPopulationSize(5,NPARENTSSUPER);
  superPop.setReportGeneration(1);
  superPop.setMaxTime(0,0,0,0);	    	
  superPop.registerInfoFunction(infoFunction);
	
  superPop.optimize();

  return 0;
}
