grapedetector
=============

Simple Arduino code for a machine designed to detect an object moving through a pipe and use an air blower to 
push it into one or another bucket (i.e. a physical sort).

The project uses an unsigned long long to make a 64-bit boolean queue modeling the path of the object through
the system. While not an ideal solution, it was certainly ideal given the constraints of having no documentation, no
Internet access, no previous experience with Arduino, and having only 20 minutes to write the code. 

See the code file for an explanation of what this is all about-- it was for a very specific science/art project
modelling a process in RNA repair for a biochemist.
