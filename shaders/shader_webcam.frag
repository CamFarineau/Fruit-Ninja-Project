#version 120

uniform sampler2D texture;

void main (void)
{
    vec2 tex_coord=gl_TexCoord[0].xy;
    gl_FragColor=texture2D(texture,tex_coord);
}
