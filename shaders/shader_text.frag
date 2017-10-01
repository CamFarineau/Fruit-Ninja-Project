#version 120

uniform sampler2D texture;



void main (void)
{
    vec2 tex_coord     = gl_TexCoord[0].xy;
    vec4 color_texture = texture2D(texture,tex_coord);

    //couleur finale
    gl_FragColor = color_texture;

}
