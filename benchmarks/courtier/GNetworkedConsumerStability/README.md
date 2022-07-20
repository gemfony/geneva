# Networked Consumer Stability Test

This test investigates the stability of different consumer configurations.

The program in `GNetworkedConsumerStabilitySubProgram/` runs an optimization using `Go2`, `GDelayIndividual` and the
evolutionary algorithm in an endless loop i.e. the optimization continues until the program is stopped from the outside.

The program `GNetworkedConsumerStability` works with a configuration file (`config/GNetworkedConsumerStabilityConfig.json`)
and starts the subprogram with the different configured competing consumer configurations.
For each consumer configuration the user of this test can specify a message which will be emitted by
a consumer once it shuts down and a message which will be emitted when it has issues connecting to the server.
`GNetworkedConsumerStability` will read the standard output of all processes in real-time using asynchronous IO and detect connection losses and
client terminations by searching for the given substrings. Once all tests have been finished plots will be drawn 
which show the two measured qualities over time.