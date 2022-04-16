#version 430 core

in layout(location = 0) vec3 position;
in layout(location = 1) vec3 normal_in;
in layout(location = 2) vec2 textureCoordinates_in;
in layout(location = 3) vec3 tangent;
in layout(location = 4) vec3 bitangent;


uniform layout(location = 3) mat4 MVP;
uniform layout(location = 4) mat4 MV;
uniform layout(location = 12) mat3 normal_mat;



out layout(location = 0) vec3 normal_out;
out layout(location = 1) vec2 textureCoordinates_out;




out vec3 FragPos;


struct Ray 
{
 vec3 origin;
 vec3 dir;
};

out Ray outRay;


void main()
{
    
    
    textureCoordinates_out = textureCoordinates_in;
    normal_out =  normalize(vec3(MV * vec4(normal_in,0.0)));

    
    // Get current position of fragment
   //gl_Position = vec4(MV * vec4(position, 1.0f));

   vec3 cameraPosition = vec3(0.0, 0.0, -2.0);


   outRay.dir = position- cameraPosition;
   outRay.origin = cameraPosition + vec3(0.5);

    gl_Position = MVP * vec4(position, 1.0f);
    
}
