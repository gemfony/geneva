#/bin/bash

GENEVATESTS=( 
	"GAdaptorT_test" 
	"GBasePopulation_test" 
	"GBooleanCollection_test" 
	"GBoostThreadPopulation_test" 
	"GBoundedDouble_test" 
	"GBoundedInt32_test" 
	"GBrokerPopulation_test" 
	"GDataExchange_test" 
	"GDoubleCollection_test" 
	"GGaussAdaptorT_test" 
	"GInt32Collection_test" 
	"GIntFlipAdaptorT_test" 
	"GNumCollectionT_test" 
	"GObject_test" 
	"GParameterSet_test" 
	"GParameterT_test" 
	"GParameterTCollectionT_test" 
	"GRandom_test" 
	"SampleIndividuals_test" 
)

let lastElement="${#GENEVATESTS[*]} -1"
for test in `seq 0 $lastElement`; do
	echo "==================================="
	echo "Running test ${GENEVATESTS[test]}"
	./${GENEVATESTS[test]}
done
