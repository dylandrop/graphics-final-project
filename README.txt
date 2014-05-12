Dylan Drop, Dobri Dobrev
dhd2110 dmd2119
=======

To run:

mkdir gcc-build && cd gcc-build
cp ../storm.jpg .
cmake .. && make && ./pa3

To use:

',' - Turn camera left
'.' - Turn camera right
'o' - Rotate view up
'l' - Rotate view down
'b' - Pull ball under water (release to see the ball fly up!)

(Additionally:
'a' - Toon shader on ball
's' - Gourand shader (http://en.wikipedia.org/wiki/Phong_reflection_model)
'd' - Blinn-Phong 
'g' - (Bonus) Plasma Frag Shader
)

For our project we rendered an ocean scene with a ball that you can bounce in the middle. This ball is buoyant, in that it bobs up and down. You can hold the ball under water and simulate the buoyancy force when it's released. Notice that a ripple will appear when the ball hits the water again. Also notice the sky image in the background.

Several techniques were employed in this project, some of which we have seen in the class, and others that we haven't seen. Of the techniques we have seen, we used the following:

• Blinn-Phong shading (on the ball)
• Texture mapping (the sky in the background)
• Various techniques regarding the representation of vertices as triangles

In addition, we also used Heightfield approximations for our water body, as explaned in this paper:

http://www.cs.ubc.ca/~rbridson/fluidsimulation/fluids_notes.pdf

Basically, this uses the idea of having certain excited points which can ripple out to the points around it by applying the equation

this.height = sum(neighbors.height) - this.height * 4

Thus, you can easily see how on each iteration, the height of a specific point would flow out to its neighboring points. All that was left was to set certain points as "jump" points, in other words points that started with an increased height and bounced up and down according to a sine wave function.

We also used the SOIL image library in creating this project. This allows us to do image mapping. To help us with SOIL, we used the SOIL image tutorial (http://www.lonesock.net/soil.html).
