#ifndef SHADER_INC
#   define SHADER_INC

#define STRINGIFY(A) #A


const char* toonVS = STRINGIFY(
varying vec3 normal;            \n
void main()                     \n
{                               \n
    normal = gl_Normal;         \n
    gl_Position = ftransform(); \n
}                               \n
);

const char* toonFS = STRINGIFY(
uniform vec3 lightDir;          \n
varying vec3 normal;            \n
void main()                     \n
{                               \n
    float intensity = dot(normalize(lightDir), normalize(normal)); \n
    if ( intensity > 0.95 )     \n
        gl_FragColor = gl_FrontMaterial.diffuse; \n
    else if ( intensity > 0.5 ) \n
        gl_FragColor = gl_FrontMaterial.diffuse*0.9; \n
    else if ( intensity > 0.25 ) \n
        gl_FragColor = gl_FrontMaterial.diffuse*0.7; \n
    else                        \n
        gl_FragColor = gl_FrontMaterial.diffuse*0.4; \n
}                               \n
);

const char* basicFS = STRINGIFY(
void main( void )                     \n
{                                     \n
   gl_FragColor = gl_Color;           \n
}                                     \n
);

const char* gourFS = basicFS;
const char* blinnpFS = basicFS;

/*
Applied the equation from Prof Zheng's notes.
*/
const char *gourVS = STRINGIFY(
void main() {                                                                             \n
  vec3 N = gl_NormalMatrix * gl_Normal;  /* normal vector */                              \n
  vec3 V = vec3(- gl_Vertex + gl_ModelViewMatrix * vec4(0, 0, 0, 1) );  /* viewer dir*/   \n
  vec3 Lm = vec3(gl_LightSource[0].position);      /* vector toward light source */       \n
  N = normalize(N); V = normalize(V); Lm = normalize(Lm);                                 \n
  gl_FrontColor = gl_FrontMaterial.ambient * gl_LightSource[0].ambient;                   \n
  if (0.0 < dot(Lm,N)) { /* if we are looking from the front */                           \n
    gl_FrontColor += gl_FrontMaterial.diffuse * dot(Lm, N) * gl_LightSource[0].diffuse;   \n 
    vec3 Rm = reflect(Lm * -1.0, N);  /* reflected ray of light */                        \n
    Rm = normalize(Rm);                                                                   \n
    /* apply equation from notes */
    gl_FrontColor += gl_FrontMaterial.specular *                                          \n
      pow(dot(Rm, V), gl_FrontMaterial.shininess) * gl_LightSource[0].specular;           \n 
  }                                                                                       \n
  gl_Position = ftransform();                                                             \n
}
);

const char *blinnpVS = STRINGIFY(
void main() {                                                                             \n
  vec3 N = gl_NormalMatrix * gl_Normal;     /* normal vector */                           \n
  vec3 Lm = vec3(gl_LightSource[0].position);   /* vector toward light source */          \n
  N = normalize(N); Lm = normalize(Lm);                                                   \n
  gl_FrontColor = gl_FrontMaterial.ambient * gl_LightSource[0].ambient;                   \n
  if (0.0 < dot(Lm,N)) {/* if we are looking from the front */                            \n
    gl_FrontColor += gl_FrontMaterial.diffuse * dot(Lm, N) * gl_LightSource[0].diffuse;   \n 
    /* apply the modified phong reflection equation */
    gl_FrontColor += gl_FrontMaterial.specular *                                          \n
      pow(dot(N, vec3(gl_LightSource[0].halfVector)), gl_FrontMaterial.shininess)         \n
      * gl_LightSource[0].specular;                                                       \n 
  }                                                                                       \n
  gl_Position = ftransform();                                                             \n
}
);

const char* checkbpFS = STRINGIFY(
varying vec2 txt_c;                                                             \n
void main()                                                                     \n
{                                                                               \n
  gl_FragColor = gl_Color;                                                      \n
  /* Look at the texture coordinate, and assign it 
  to either dark or light accordingly. */                                       \n
  if (1.0 > mod((floor(txt_c.x*20.) + floor(txt_c.y*20.)), 2.))                 \n
      gl_FragColor *= 2.0;                                                      \n
}                                                                               \n
);

/*
Checker Blinn phong shader. Exactly the same as the Blinn-Phong shader except
for the last line.
*/
const char* checkbpVS = STRINGIFY(
varying vec2 txt_c;                                                                       \n
void main() {                                                                             \n
  vec3 N = gl_NormalMatrix * gl_Normal;     /* normal vector */                           \n
  vec3 Lm = vec3(gl_LightSource[0].position);   /* vector toward light source */          \n
  N = normalize(N); Lm = normalize(Lm);                                                   \n
  gl_FrontColor = gl_FrontMaterial.ambient * gl_LightSource[0].ambient;                   \n
  if (0.0 < dot(Lm,N)) {/* if we are looking from the front */                            \n
    gl_FrontColor += gl_FrontMaterial.diffuse * dot(Lm, N) * gl_LightSource[0].diffuse;   \n 
    /* apply the modified phong reflection equation */
    gl_FrontColor += gl_FrontMaterial.specular *                                          \n
      pow(dot(N, vec3(gl_LightSource[0].halfVector)), gl_FrontMaterial.shininess)         \n
      * gl_LightSource[0].specular;                                                       \n 
  }                                                                                       \n
  gl_Position = ftransform();                                                             \n
  txt_c = gl_MultiTexCoord0.st;                                                           \n
}
);

const char* textureFS = STRINGIFY(
varying vec2 txt_c;                                       \n
uniform sampler2D tex_2d;                                 \n
void main()                                               \n
{                                                         \n
    gl_FragColor = texture2D(tex_2d, txt_c);              \n
}                                                         \n
);

const char* textureVS = STRINGIFY(
varying vec2 txt_c;
void main()                                          \n
{                                                    \n
    txt_c = gl_MultiTexCoord0.st;                    \n
    gl_Position = ftransform();                      \n
}                                                    \n
);


const char* bonusFS = STRINGIFY(
  varying vec2 txt_c;                                                                     \n
  uniform float time;                                                                     \n
  void main() {                                                                           \n
    /* Use a mixture of random cos functions applied to the texture and fragment
    coordinates, with different coefficients to get more interesting results. */          \n
    float m = cos(gl_FragCoord.x/20. + txt_c.x) + cos(gl_FragCoord.y/20. + txt_c.y)       \n
      + cos((gl_FragCoord.x/20.+gl_FragCoord.y/20.+time/500.)/3.0);                       \n
    /* Apply a smoothing parameter to get the colors to be a larger distribution. */      \n
    float sinM = sin(3.1415926535897*m);                                                  \n
    /* Rotate plasma in a somewhat random way about x. */                                 \n
    vec3 plasma = vec3(1, sinM * cos(3.1415926535897*m), 1. - sinM);                      \n
    /* Apply random colors to the vector given */                                         \n
    gl_FragColor = vec4(0.6 + plasma *.6, 0.7);                                           \n
  }                                                                                       \n
);

/*
Same as the basic shader except it accepts the time parameter from main.cpp
*/
const char* bonusVS = STRINGIFY(
  varying vec2 txt_c;                                                             \n
  uniform float time;                                                             \n
  void main()                                                                     \n
  {                                                                               \n
      gl_Position = ftransform();                                                 \n
      txt_c = gl_MultiTexCoord0.st;                                               \n
  }                                                                               \n
  );
#endif
