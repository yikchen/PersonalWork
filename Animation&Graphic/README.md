## Character&Animation
The program would take in skel file and create joints. Each joints should have its only world matrix will indicate its location. The program would also take in skin file and its will create triangle mesh on top of the corresponding skel. If no skel file is provide the skin will be in deformat state. The program does smooth skinning to attach the skin to the skel. Finally, the program take in keyframe and produce animation using interpolation and extrapolation.

## Cloth Simulation
This projecct does a cloth simulation using 100 particles (10 x 10). Each particle is apply with physic (spring force, spring damper, wind, gravity, collision with ground). 

## Quaternion
In this project, I implement my own quaternion class to represent the object that exist in the space. Each box have its own quaternion representation, thus interpolation between quaternion can be use to create smooth animation. 
