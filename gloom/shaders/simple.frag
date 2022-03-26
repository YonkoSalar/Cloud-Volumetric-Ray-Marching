#version 430 core

in layout(location = 0) vec3 normal;
in layout(location = 1) vec2 textureCoordinates;
in vec3 FragPos;



out vec4 color;

void main()
{
    color = vec4(1.0f, 1.0f, 1.0f, 1.0f);
}
