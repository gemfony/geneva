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

const unsigned int NPARENTS=1;

void infoFunction(GBasePopulation * const gbp){
  GLogStreamer gls;
  gls << setprecision(15) << "h" << gbp << "->Fill(" << gbp->getGeneration() << ", " << gbp->at(0)->getMyCurrentFitness() << ");" << endl;
  if(gbp->at(0)->isDirty()){
    gls << "// Attention: object carries the dirty flag!" << endl;
  }
  gls << logLevel(TRACK);
}

void createData(vector<double> &values, int nValues){
  int i,j;
  GRandom gr;

  for(i=0; i<nValues; i++){
    for(j=0; j<NDIMORIG; j++){ // NDIMORIG is defined in GMinFunction.h
      values.push_back(gr.evenRandom(-2.,2.));
    }
  }
}

int main(int argc, char **argv)
{
  LOGGER.addTarget(new GDiskLogger());
  LOGGER.addTarget(new GConsoleLogger());
  LOGGER.addLogLevel(EXCEPTION);
  LOGGER.addLogLevel(CRITICAL);
  LOGGER.addLogLevel(UNCRITICAL);
  LOGGER.addLogLevel(TRACK);
 
  vector<double> values;
  createData(values,NDATA); // NDATA is defined in GMinFunction.h
	
  GBoostThreadConsumer gc;
  gc.setMaxThreads(5);
		
  GTransferPopulation pop;	
  // GBoostThreadPopulation pop;
  // pop.setMaxThreads(5);
	
  for(unsigned int i=0; i<NPARENTS; i++){
    GMinFunction *gmf = new GMinFunction(NDIMTARGET*NDATA,-10,10,values); // NDIMTARGET is defined in GMinFunction.h
    // We try out parents with different sigma
    GDoubleGaussAdaptor *gdga = 
      new GDoubleGaussAdaptor(0.05*double(i+1)/NPARENTS, 0.005*double(i+1)/NPARENTS, 0.0001,"GDoubleGaussAdaptor");
    gmf->addAdaptor(gdga);
        
    pop.appendMember(gmf);
  }
        	
  pop.setMaxGeneration(2000);
  pop.setMaximize(false);
  pop.setPopulationSize(10,NPARENTS);
  pop.setReportGeneration(1);
  pop.setMaxTime(0,0,0,0);	
  pop.registerInfoFunction(infoFunction);

  pop.optimize();
    
  GMinFunction *g = static_cast<GMinFunction *>(pop.at(0).get());
  g->print();
    
  return 0;
}
