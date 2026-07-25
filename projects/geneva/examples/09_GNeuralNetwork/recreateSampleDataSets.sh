#!/bin/bash
echo "Creating hyper_cube.dat"
./GNeuralNetwork --trainingDataFile ./DataSets/hyper_cube.dat   --traininDataType 1 --nDataSets 5000
echo "Creating hyper_sphere.dat"
./GNeuralNetwork --trainingDataFile ./DataSets/hyper_sphere.dat --traininDataType 2 --nDataSets 5000
echo "Creating axis_centric.dat"
./GNeuralNetwork --trainingDataFile ./DataSets/axis_centric.dat --traininDataType 3 --nDataSets 5000
echo "Creating sinus.dat"
./GNeuralNetwork --trainingDataFile ./DataSets/sinus.dat        --traininDataType 4 --nDataSets 5000
echo "done"

