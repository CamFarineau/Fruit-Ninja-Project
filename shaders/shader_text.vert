#version 120

uniform vec2 size;

//Un Vertex Shader encore plus minimaliste
void main (void)
{
    //position dans l'espace ecran
    gl_Position = vec4(     gl_Vertex.x / size.x * 2.0 - 1.0,
                       1.0 -gl_Vertex.y / size.y * 2.0,0.0,1.0);

    //coordonnees de textures
    gl_TexCoord[0]=gl_MultiTexCoord0;
}
