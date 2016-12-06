#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject{
	mat4 model;
	mat4 view;
	mat4 proj;
} UBO;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColour;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 fragColour;
layout(location = 1) out vec2 fragTexCoord;

out gl_PerVertex { vec4 gl_Position; };

void main() {
    gl_Position = UBO.proj * UBO.view * UBO.model * vec4(inPosition, 1.0);
    fragColour = inColour;
	fragTexCoord = inTexCoord;
}