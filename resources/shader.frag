varying vec2 Texcoord;
varying vec4 col;
uniform sampler2D sampler0;
void main()
{
    gl_FragColor = col * texture2D(sampler0, Texcoord);
}