# HairAirSimulation
<img src="https://github.com/JustLuoxi/HairAirSimulation/blob/master/interface.jpg" height="300">

Qt: 5.8.0    
Compiler: Desktop Qt 5.8.0 MSVC2015_64bit

# Introduction 

This is a series of assignments.    
In assignment 1, build a graphics program consists of animation controlling widgets to create key-frame animation.    
In assignment 2, simulate the hair motion with moving head.(I made it in 7~9 fps)   
In assignment 3, simulate the air flow(by Navier-Stokes equations), and handle the coupling cases of hair and air. 

# N-S equations

N-S equations are fundamental, interesting, and difficult to understand without a good background in physics and differential equations(as someone said). [BlainMaguire's work](https://github.com/BlainMaguire/3dfluid, "3d fluid") really helps me a lot.   

# System features

(1) A graphics program consists of animation controlling widgets to create key-frame animation("Add" -- add current frame as a key frame. slider -- change the frame. "run" -- after adding enough key-frames in different frame indices, you can click "run" to see the animation).    
(2) Interactive model control (Left -- translate, Right -- rotate ).     
(3) Hair generation and simulation.    
(4) Head-hair collision detection and solution (Bvh tree).    
(5) Basic rendering of hairs(Kajiya-Kay shading).     
(6) Add air flow interactively (W -- add air from bottom to top, D -- add air from left).    
(7) Add air flow in the simulation scenario to make hair flutter (V -- switch of showing air velocity field).    
(8) A coupling of air motion and air (H -- switch of hair -> air, J -- switch of air -> hair ).    

# What's more
If you are interesed in simulation and interaction. You may want to see the implement details in Assignment 2.pdf and Assignment 3.pdf. 

<img src="https://github.com/JustLuoxi/HairAirSimulation/blob/master/result.jpg" height="300">
