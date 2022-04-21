# GMPIConsumerT example

This example shows how to use the GMPIConsumerT of the geneva library connection to process work items in a cluster
communicating via MPI.

## Run the example:

The example is best started with a runner program like `mpirun` like so:

```
mpirun -np 4 ./GMPIConsumer
```

The above command will run the example in 4 processes. One of the processes will run the master node which distributes
the work items. The other three processed will function as workers.

TODO: mention usage with the Go2 class (yet to be implemented)
