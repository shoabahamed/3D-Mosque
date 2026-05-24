#version 330 core
out vec4 FragColor;
uniform vec3 objectColor;

// No texture samplers needed

void main()
{
    FragColor = vec4(objectColor, 1.0f); // Just output an orange color
}