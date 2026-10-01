#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;
layout(location = 2) in vec2 a_TexCoord;
layout(location = 3) in float a_TexIndex;
layout(location = 4) in float a_TilingFactor;
layout(location = 5) in float a_EmissiveTexIndex; 
layout(location = 6) in float a_EmissiveIntensity;
layout(location = 7) in int a_EntityID;

layout(std140, binding = 0) uniform Camera
{
	mat4 u_ViewProjection;
};

out vec4 v_Color;
out vec2 v_TexCoord;
out flat float v_TexIndex;
out float v_TilingFactor;
out flat float v_EmissiveTexIndex;
out flat float v_EmissiveIntensity;
out flat int v_EntityID;

void main() 
{
	v_Color = a_Color;
	v_TexCoord = a_TexCoord;
	v_TexIndex = a_TexIndex;
	v_TilingFactor = a_TilingFactor;
	v_EmissiveTexIndex = a_EmissiveTexIndex;
	v_EmissiveIntensity = a_EmissiveIntensity;
	v_EntityID = a_EntityID;
	gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
}

#type fragment
#version 450 core

layout(location = 0) out vec4 color;
layout(location = 1) out int color2;

in vec4 v_Color;
in vec2 v_TexCoord;
in flat float v_TexIndex;
in float v_TilingFactor;
in flat float v_EmissiveTexIndex;
in flat float v_EmissiveIntensity;
in flat int v_EntityID;

uniform sampler2D u_Texture[32]; 

void main() 
{
	  vec4 texColor = texture(u_Texture[int(v_TexIndex)], v_TexCoord * v_TilingFactor) * v_Color;
    
    if (v_EmissiveTexIndex > 0.0) 
    {
        vec4 emissiveColor = texture(u_Texture[int(v_EmissiveTexIndex)], v_TexCoord * v_TilingFactor);
        texColor.rgb += emissiveColor.rgb * emissiveColor.a * v_EmissiveIntensity;
    }
    else 
    {
        texColor.rgb *= v_EmissiveIntensity;
    }

    if (texColor.a < 0.1)
        discard;

    color = texColor;
    color2 = v_EntityID;
}