#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;
layout(location = 2) in vec2 a_TexCoord;
layout(location = 3) in float a_TexIndex;
layout(location = 4) in float a_TilingFactor;
layout(location = 5) in int a_EntityID;

layout(std140, binding = 0) uniform Camera
{
	mat4 u_ViewProjection;
};

out vec4 v_Color;
out vec2 v_TexCoord;
out float v_Falloff;
out flat int v_EntityID;

void main() 
{
	v_Color = a_Color;
	v_TexCoord = a_TexCoord;
	v_Falloff = a_TilingFactor; // Hack: Usem el TilingFactor per passar el Falloff
	v_EntityID = a_EntityID;
	
	gl_Position = u_ViewProjection * vec4(a_Position, 1);
}

#type fragment
#version 450 core

layout(location = 0) out vec4 color;
layout(location = 1) out int color2;

in vec4 v_Color;
in vec2 v_TexCoord;
in float v_Falloff;
in flat int v_EntityID;

void main() 
{
	// 1. Les coordenades de textura van de 0 a 1. 
	// Ho convertim a un espai de -1 a 1 perquè el centre (0,0) estigui al mig del Quad.
	vec2 localPos = v_TexCoord * 2.0 - 1.0;
	
	// 2. Calculem la distància fins al centre (Teorema de Pitàgores automàtic)
	float distance = length(localPos);
	
	// 3. Atenuació de la llum. Si la distància és > 1, és 0 (negre/transparent). 
	// El Falloff controla com de suau és la transició.
	float attenuation = clamp(1.0 - distance, 0.0, 1.0);
	attenuation = pow(attenuation, v_Falloff); // Modifiquem la corba de caiguda
	
	// 4. Color final: Color RGB * Intensitat (guardada a l'Alpha) * Atenuació
	vec3 lightColor = v_Color.rgb;
	float intensity = v_Color.a;
	
	vec4 finalColor = vec4(lightColor * intensity * attenuation, attenuation);
	
	if (finalColor.a < 0.01)
		discard; // No dibuixem els píxels de les cantonades on no hi ha llum
		
	color = finalColor;
	color2 = v_EntityID; // Per l'Editor (poder fer clic a la llum)
}