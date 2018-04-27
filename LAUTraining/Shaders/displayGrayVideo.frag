#version 330 core

uniform sampler2D qt_texture;      // THIS TEXTURE HOLDS THE XYZ+TEXTURE COORDINATES
uniform float     qt_scaleFactor;  // TELLS US HOW MUCH TO SCALE THE INPUT PIXELS SO THAT WE CAN SEE THEM ON SCREEN
in           vec2 qt_coordinate;   // HOLDS THE TEXTURE COORDINATE FROM THE VERTEX SHADER

layout(location = 0, index = 0) out vec4 qt_fragColor;

void main()
{
    // GET THE PIXEL COORDINATE OF THE CURRENT FRAGMENT
    qt_fragColor.rgb = qt_scaleFactor * texture(qt_texture, qt_coordinate, 0).rrr;
    qt_fragColor.a = 1.0;
}
