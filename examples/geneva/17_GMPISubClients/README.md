# MPI Sub-Clients


This example shows how to use the GMPISubClientOptimizer.
It offers all functionality of Go2 but additionally adds the functionality of
creating MPI-sub-clients.

You can set the size of the sub-client networks via the `subClientGroupSize` parameter in the
`GMPISubClientOptimizer.json` config file.


For example to run this example with 1 server, 4 clients and 3 sub-clients for each of the clients use:

``````
    mpirun --oversubscribe -np 17 ./GMPISubClients --consumer mpi 
``````

The amount of subgroups will automatically be calculated from the number of processes started with
mpirun and the size of the subgroups set through the configuration file.