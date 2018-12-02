uniform mat4 ProjectionMatrix;
attribute vec2 vertex;
attribute vec2 uv0;
attribute vec4 colour;
varying vec2 Texcoord;
varying vec4 col;
void main()
{
    gl_Position = ProjectionMatrix* vec4(vertex.xy, 0., 1.);
    Texcoord  = uv0;
    col = colour;
}