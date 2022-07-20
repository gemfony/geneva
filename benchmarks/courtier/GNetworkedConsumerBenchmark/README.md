# Networked Consumer Benchmark

This is a benchmark which compares different consumer configurations regarding their speed.

`GNetworkedConsumerBenchmarkSubProgram/` contains a program, which runs multiple optimizations using the `GDelayIndividual`
and measures time for the execution of a given amount of iterations. It then writes a serialized version of the measurements to file.

The program `GNetworkedConsumerBenchmark` works with a config (`config/GNetworkedConsumerBenchmarkConfig.json`). It runs the
benchmark subprogram with the configured consumer configurations (also called competitors). It takes care of starting the 
competing configurations with the clients on the local machine. Then it combines all the measurements into nice graphs.