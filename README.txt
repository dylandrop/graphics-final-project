Dylan Drop
dhd2110
=======

To run:

cd cmake && make && ./pa3

To use:

'a' - Toon shader
's' - Gourand shader (http://en.wikipedia.org/wiki/Phong_reflection_model)
'd' - Blinn-Phong (this is almost identical to Gourand, but uses a different
inner product [N dot M])
'f' - Checkerboard Blinn-Phong (this is almost identical to the previous, 
but has a FS that chooses a color modulo 2 according to frag position)
'g' - (Bonus) Plasma Frag Shader 

[Screenshots of each shader are in the parent directory in png format]

',' - Turn camera left
'.' - Turn camera right

Bonus explanation - I found out I could achieve a simple but interesting 
plasma effect if I mixed and matched the sin and cos equations applied to
a fragment's location + the glMultiTexCoord. Adding time into the equation
makes things more interesting, as you can see in the video.
