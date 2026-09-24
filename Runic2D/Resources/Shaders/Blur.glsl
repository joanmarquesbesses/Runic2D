#type vertex
#version 450 core

layout(location = 0) in vec2 a_Position;
layout(location = 1) in vec2 a_TexCoord;

out vec2 v_TexCoord;

void main()
{
    v_TexCoord = a_TexCoord;
    gl_Position = vec4(a_Position, 0.0, 1.0);
}

#type fragment
#version 450 core

layout(location = 0) out vec4 color;

in vec2 v_TexCoord;

uniform sampler2D u_Image;
uniform bool u_Horizontal;
uniform float u_Threshold;

const float weight[5] = float[](0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

vec3 ExtractHDR(vec3 c) {
    return max(c - vec3(u_Threshold), vec3(0.0));
}

void main()
{
    vec2 tex_offset = 1.0 / textureSize(u_Image, 0); 
    
    vec3 result = ExtractHDR(texture(u_Image, v_TexCoord).rgb) * weight[0];
    
    if(u_Horizontal)
    {
        for(int i = 1; i < 5; ++i) {
            result += ExtractHDR(texture(u_Image, v_TexCoord + vec2(tex_offset.x * i, 0.0)).rgb) * weight[i];
            result += ExtractHDR(texture(u_Image, v_TexCoord - vec2(tex_offset.x * i, 0.0)).rgb) * weight[i];
        }
    }
    else
    {
        for(int i = 1; i < 5; ++i) {
            result += ExtractHDR(texture(u_Image, v_TexCoord + vec2(0.0, tex_offset.y * i)).rgb) * weight[i];
            result += ExtractHDR(texture(u_Image, v_TexCoord - vec2(0.0, tex_offset.y * i)).rgb) * weight[i];
        }
    }
    
    color = vec4(result, 1.0);
}