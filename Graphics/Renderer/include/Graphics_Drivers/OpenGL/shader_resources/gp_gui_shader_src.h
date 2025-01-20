#ifndef GP_GUI_SHADER_SRC_H
#define GP_GUI_SHADER_SRC_H


namespace gridpro_gui {

namespace OpenGL_3_3
{
namespace ShaderSrc {

// Shader Name : Default Flat Shader [with pseudo lighting] [PER_GEOMETRY_COLOR]
static const char* BasicVertexShaderSource = R"(

    #version 330 core

    layout(location = 0) in vec3 VertexPos;

    uniform mat4 projection;
    uniform mat4 model; 
    uniform mat4 view; 

    void main()
    {    
       gl_Position = projection * view * model * vec4(VertexPos, 1.0); 
    }
)";

static const char* BasicFragmentShaderSource = R"(

    #version 330 core

    out vec4 FragColor;

 // uniform sampler2D textureSampler;

    uniform vec4 object_color;
 
    void main()
    {  
       vec3 mycolor = object_color.rgb;
       vec3 fragPos = gl_FragCoord.xyz;
       float depth  = gl_FragCoord.z;

     //vec3 dimming_factor = vec3(depth, depth, depth);
     //mycolor = mycolor - dimming_factor;
     //mycolor.r = clamp(mycolor.r, 0.1, 0.5);
     
       FragColor = object_color;
       
     //FragColor = texture(textureSampler, uv);
    }
)";


// // Shader Name : Default Flat Shader [with pseudo lighting] [PER_VERTEX_COLOR]
// static const char* PerVertexColorVertexShaderSource = R"(

//     #version 330 core

//     layout(location = 0) in vec3  VertexPos;
//     layout(location = 1) in uvec3 VertexColor;

//     out vec3 color;

//     uniform mat4 projection;
//     uniform mat4 model; 
//     uniform mat4 view; 

//     void main()
//     {    
//        color = normalize(vec3(VertexColor));
//        gl_Position = projection * view * model * vec4(VertexPos, 1.0); 
//     }
// )";

// static const char* PerVertexColorFragmentShaderSource = R"(

//     #version 330 core

//     in vec3 color;

//     out vec4 FragColor;

//     uniform vec4 object_color;

//     void main()
//     {        
//        FragColor = vec4(color, 1.0f);
//     }
// )";

static const char* PerVertexColorVertexShaderSource = R"(

    #version 430 core

    layout(location = 0) in vec3  VertexPos;
    layout(location = 1) in uvec3 VertexColor;

    out vec4 color;

    uniform mat4 projection;
    uniform mat4 model; 
    uniform mat4 view; 

    void main()
    {    
       gl_Position = projection * view * model * vec4(VertexPos, 1.0); 
       
       vec3 out_color = normalize(vec3(VertexColor));
       
       color = vec4(out_color, 1.0);
    }
)";

static const char* PerVertexColorFragmentShaderSource = R"(

    #version 430 core

    in vec4 color;

    out vec4 FragColor;

    void main()
    {        
       FragColor = color;
    }
)";




const char* PhongsLightingVertexShaderSource = R"(
#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;


out vec3 fragNormal;
out vec3 fragPosition;
out vec3 fragLightDir;
out vec4 vertexColor;



uniform vec4 object_color;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec3 lightPosition;



void main()
{
      
    // Transform vertex position and normal to world space
    vec4 worldPosition = model * vec4(position, 1.0);
    vec3 worldNormal = normalize(mat3(transpose(inverse(model))) * normal);
    
    // Compute the light direction
    fragLightDir = normalize(lightPosition - worldPosition.xyz);

    // Pass data to the fragment shader
    fragNormal = worldNormal;
    fragPosition = worldPosition.xyz;
     
    vertexColor = object_color;
     //  vertexColor = vec3(0.8f, 0.4f , 0.8f );
    

    // Transform the vertex position to clip space
    gl_Position = projection * view * worldPosition;
}
)";

//------------------------------------------On Screen rendering Fragment Shader----------------------------------------------------------------+

const char* PhongsLightingFragmentShaderSource = R"(

#version 330 core

in vec3 fragNormal;
in vec3 fragPosition;
in vec3 fragLightDir;
in vec4 vertexColor;


out vec4 fragColor;

uniform vec3 lightAmbient;
uniform vec3 lightDiffuse;
uniform vec3 lightSpecular;
uniform vec3 materialAmbient;
uniform vec3 materialDiffuse;
uniform vec3 materialSpecular;
uniform float materialShininess;



// Gamma correction parameter
const float gamma = 1.2;

void main()
{  
  
// Compute the normal and light direction in fragment space
vec3 normal = normalize(fragNormal);

if (!gl_FrontFacing) {
    normal = -normal; 
}

vec3 lightDir = normalize(fragLightDir);
vec3 viewDir = normalize(-fragPosition);


// Compute the diffuse color
float diffuseFactor = max(dot(normal, lightDir), 0.0);
vec3 diffuseColor = lightDiffuse * materialDiffuse * diffuseFactor * 3.5;

// Compute the half-angle vector (Blinn-Phong)
vec3 halfDir = normalize(lightDir + viewDir);
vec3 reflectDir = reflect(-lightDir, normal);


// Compute the specular color (Blinn-Phong)
float specularFactor = pow(max(dot(viewDir, reflectDir), 0.0), materialShininess);
//float specularFactor = pow(max(dot(normal, halfDir), 0.0), materialShininess);

vec3 specularColor = lightSpecular * materialSpecular * specularFactor * 6.0;

// Optional: Fresnel Effect to enhance realism (comment out if not needed)
float fresnelFactor = pow(1.0 - max(dot(viewDir, normal), 0.0), 3.0);
specularColor *= mix(1.0, fresnelFactor, 0.5);

// Compute the final color (ambient + diffuse + specular)
vec3 ambientColor = lightAmbient * materialAmbient * 2.0f;
vec3 finalColor = ambientColor + diffuseColor + specularColor;
finalColor = finalColor * vertexColor.xyz;

// Apply gamma correction for more realistic lighting perception
finalColor = pow(finalColor, vec3(1.0 / gamma));

fragColor = vec4(finalColor, 1.0f);
}

)";


// static const char *PhongsLightingVertexShaderSource = R"(
    
// #version 330 core

// layout(location = 0) in vec3 position;
// layout(location = 1) in vec3 normal;

// out vec3 fragNormal;
// out vec3 fragPosition;
// out vec3 fragLightDir;
// out vec3 vertexColor;
// out vec3 vertexAlpha;

// uniform mat4 projection;
// uniform mat4 model; 
// uniform mat4 view; 
// uniform vec3 lightPosition;

// void main()
// {
      
//     // Transform vertex position and normal to world space
//     vec4 worldPosition = model * vec4(position, 1.0);
//     vec3 worldNormal = mat3(transpose(inverse(model))) * normal;

//     // Compute the light direction
//     fragLightDir = normalize(lightPosition - worldPosition.xyz);

//     // Pass data to the fragment shader
//     fragNormal = worldNormal;
//     fragPosition = worldPosition.xyz;
     
//     // Transform the vertex position to clip space
//     gl_Position = projection * view * worldPosition;
// }
// )";

// //------------------------------------------On Screen rendering Fragment Shader----------------------------------------------------------------+

// static const char *PhongsLightingFragmentShaderSource = R"(

// #version 330 core

// in vec3 fragNormal;
// in vec3 fragPosition;
// in vec3 fragLightDir;
// in vec3 vertexColor;
// in vec3 vertexAlpha;

// out vec4 fragColor;

// uniform vec4 object_color;
// uniform vec3 lightAmbient;
// uniform vec3 lightDiffuse;
// uniform vec3 lightSpecular;
// uniform vec3 materialAmbient;
// uniform vec3 materialDiffuse;
// uniform vec3 materialSpecular;
// uniform float materialShininess;

// vec3 BlinnPhong(vec3 FragNormal, vec3 FragLightDir, vec3 FragPos, vec3 LightDiffuse, vec3 MaterialDiffuse , vec3 LightSpecular, vec3 MaterialSpecular, float MaterialShininess)
// {
//     // Compute the normal and light direction in fragment space
//     vec3 normal = normalize(FragNormal);
//     vec3 lightDir = normalize(FragLightDir);
//     vec3 viewDir = normalize(-FragPos);

//     // Compute the diffuse color
//     float diffuseFactor = max(dot(normal, lightDir), 0.0);
//     vec3 diffuseColor = LightDiffuse * MaterialDiffuse * diffuseFactor;
//     diffuseColor = diffuseColor + diffuseColor * 0.3f; 


//     // Compute the half-angle vector (Blinn-Phong)
//     vec3 halfDir = normalize(lightDir + viewDir);

//     // Compute the specular color (Blinn-Phong)
//     float specularFactor = pow(max(dot(normal, halfDir), 0.0), MaterialShininess);
//     vec3 specular = LightSpecular * MaterialSpecular * specularFactor;
//     vec3 finalColor = diffuseColor + specular;
//     return finalColor;
// }

// vec3 Phong(vec3 FragNormal, vec3 FragLightDir, vec3 FragPos, vec3 LightDiffuse, vec3 MaterialDiffuse , vec3 LightSpecular, vec3 MaterialSpecular, float MaterialShininess)
// {
//     // Compute the normal and light direction in fragment space
//     vec3 normal = normalize(FragNormal);
//     vec3 lightDir = normalize(FragLightDir);
//     vec3 viewDir = normalize(-FragPos);

//     // Compute the diffuse color
//     float diffuseFactor = max(dot(normal, lightDir), 0.0);
//     vec3 diffuseColor = LightDiffuse * MaterialDiffuse * diffuseFactor;
//     diffuseColor = diffuseColor;

//     // Compute the reflection vector (Phong)
//     vec3 reflectDir = reflect(-lightDir, normal);

//     // Compute the specular color (Phong)
//     float specularFactor = pow(max(dot(viewDir, reflectDir), 0.0), MaterialShininess);
//     vec3 specular = LightSpecular * MaterialSpecular * specularFactor;
    
//     // Compute the final color
//     vec3 finalColor = diffuseColor + specular;
//     return finalColor;
// }

// void main()
// {  

//     // Compute the color using the Blinn-Phong model
//     vec3 out_color = Phong(fragNormal, fragLightDir, fragPosition, lightDiffuse, materialDiffuse, lightSpecular, materialSpecular, materialShininess);

//     fragColor = vec4(out_color, 1.0);
// }

// )";

static const char* SelectPrimitiveVertexShaderSource = R"(

    #version 330 core

    layout(location = 0) in vec3 VertexPos;

    uniform mat4 projection;
    uniform mat4 model; 
    uniform mat4 view; 
    
    void main()
    {            
      gl_Position = projection * view * model * vec4(VertexPos, 1.0);
    }
)";

static const char* SelectPrimitiveFragmentShaderSource = R"(

    #version 410 core

    out vec4 FragColor;

    uniform int selection_init_id;
    
    void main()
    {  
      uint id = uint(selection_init_id);

      uint PrimID = gl_PrimitiveID + id;

      vec3 unique_color = vec3(1.0, 1.0, 1.0);

      unique_color.b = float((PrimID >> 16) & 0xFF) / 255.0;
      unique_color.g = float((PrimID >> 8)  & 0xFF) / 255.0;
      unique_color.r = float(PrimID & 0xFF) / 255.0;
 
      FragColor = vec4(unique_color, 1.0f);
    }
)";

static const char* SelectGeometryVertexShaderSource = R"(

    #version 330 core

    layout(location = 0) in vec3 VertexPos;

    uniform mat4 projection;
    uniform mat4 model; 
    uniform mat4 view; 
    
    void main()
    {            
      gl_Position = projection * view * model * vec4(VertexPos, 1.0);
    }
)";

static const char* SelectGeometryFragmentShaderSource = R"(

    #version 330 core
    
    out vec4 FragColor;

    uniform vec3 selection_init_id;
    
    void main()
    {  
      FragColor = vec4(selection_init_id, 1.0f);
    }
)";

}
} // namespace OpenGL_3_3
} // namespace gridpro_gui
#endif // GP_GUI_SHADER_SRC_H