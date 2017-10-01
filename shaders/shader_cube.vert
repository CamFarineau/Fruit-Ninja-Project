#version 120

uniform mat4 camera_projection;
uniform mat4 camera_modelview;
uniform mat4 normal_matrix;
uniform mat4 rotation;
uniform vec2 translation;
uniform float scaling;

varying vec4 position_3d_original;
varying vec4 position_3d_modelview;

varying vec3 normal;
varying vec4 color;




void main (void)
{
    vec4 p = gl_Vertex;
    p.xyz *= scaling;
    gl_Position = camera_projection*camera_modelview*rotation*p;
    gl_Position.xy += translation*gl_Position.w;

    position_3d_original = gl_Vertex;
    position_3d_modelview = camera_modelview*gl_Vertex;
    color = gl_Color;

    vec4 normal4d = normal_matrix*vec4(normalize(gl_Normal),0.0);
    normal = normal4d.xyz;

    gl_TexCoord[0]=gl_MultiTexCoord0;
}
