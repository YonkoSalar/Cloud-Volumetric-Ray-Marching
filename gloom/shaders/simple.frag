#version 430 core

in layout(location = 0) vec3 normal;
in layout(location = 1) vec2 textureCoordinates;



out vec4 color;

uniform float u_time;



#define STEPS 20
#define OCTAVES 1
#define ZMAX 35
#define CAM_DISTANCE 35
#define SHADOW_STEPS 20
#define SHADOW_MAX 4
#define OPACITY 6
#define SHADOW_STRENGTH 4
#define AMBIENT_DENSITY 3

#define noise_type noise


vec3 cloud_color = vec3(1.0, 0.7, 0.8);
vec3 ambient_color = vec3(1.0, 0.2, 0.3);




////////////////////////////
//  VORONOI/WORLEY NOISE  //
////////////////////////////
// Source: https://github.com/libretro/glsl-shaders/blob/master/borders/resources/voronoi.glsl

vec2 rand2(in vec2 p)
{
        return fract(vec2(sin(p.x * 591.32 + p.y * 154.077), cos(p.x * 391.32 + p.y * 49.077)));
}

// voronoi distance noise, based on iq's articles
float voronoi(in vec2 x)
{
        vec2 p = floor(x);
        vec2 f = fract(x);
        
        vec2 res = vec2(8.0);
        for(int j = -1; j <= 1; j ++)
        {
                for(int i = -1; i <= 1; i ++)
                {
                        vec2 b = vec2(i, j);
                        vec2 r = vec2(b) - f + rand2(p + b);
                        
                        // chebyshev distance, one of many ways to do this
                        float d = max(abs(r.x), abs(r.y));
                        
                        if(d < res.x)
                        {
                                res.y = res.x;
                                res.x = d;
                        }
                        else if(d < res.y)
                        {
                                res.y = d;
                        }
                }
        }
        return res.y - res.x;
}


          

///////////////////////
//  NOISE FUNCTION  //
///////////////////////
// Source: https://gist.github.com/patriciogonzalezvivo/670c22f3966e662d2f83


float hash1( float n )
{
    return fract( n*17.0*fract( n*0.3183099 ) );
}


float noise( vec3 x )
{
    vec3 p = floor(x);
    vec3 w = fract(x);
    
    vec3 u = w*w*w*(w*(w*6.0-15.0)+10.0);
    
    float n = p.x + 317.0*p.y + 157.0*p.z;
    
    float a = hash1(n+0.0);
    float b = hash1(n+1.0);
    float c = hash1(n+317.0);
    float d = hash1(n+318.0);
    float e = hash1(n+157.0);
	float f = hash1(n+158.0);
    float g = hash1(n+474.0);
    float h = hash1(n+475.0);

    float k0 =   a;
    float k1 =   b - a;
    float k2 =   c - a;
    float k3 =   e - a;
    float k4 =   a - b - c + d;
    float k5 =   a - c - e + g;
    float k6 =   a - b - e + f;
    float k7 = - a + b + c - d + e - f - g + h;

    return -1.0+2.0*(k0 + k1*u.x + k2*u.y + k3*u.z + k4*u.x*u.y + k5*u.y*u.z + k6*u.z*u.x + k7*u.x*u.y*u.z);
}




//////////////////////////////////
//  FRACTIONAL BROWNIAN MOTION  //
//////////////////////////////////
// Source: https://www.iquilezles.org/www/articles/fbm/fbm.htm

float fbm( vec3 p )
{
    float f = 0.0;
    float gat = 0.0;
    
    for (float octave = 0.; octave < OCTAVES; ++octave)
    {
        float la = pow(2.0, octave);
        float ga = pow(0.5, octave + 1.);
        f += ga*noise_type( la * p); 
        gat += ga;
    }
    
    f = f/gat;
    
    return f;
}



/////////////////
//   DENSITY   //
/////////////////


/*
    This function move the noise throughout our scene by first adding the position 
    vector. Some of these numbers randomly picked t until it got a more detailed look.
*/

float map(vec3 pos) {
    pos += vec3(0.0, 0.0, -0.9);
    float f = fbm(pos * 0.2);
    return clamp( f-0.2 , 0.0, 1.0 );
}






////////////////////
//   RAY MARCH   //
///////////////////

/*
    Reference for creating volumetric ray marcher

    Inspiration:
    - https://www.shadertoy.com/view/MdyGzR
    - https://shaderbits.com/blog/creating-volumetric-ray-marcher/
*/


vec3 rayMarch(vec3 pos, vec3 dir)
{
    // Get step length
    float z_steps = ZMAX / float(STEPS);
    float inverse_step = 1.0 / float(STEPS);
    
    // Get shadow step
    float shadow_step = SHADOW_MAX / float(SHADOW_STEPS);
    float inverse_shadow_step = 1.0 / float(SHADOW_STEPS);
    

    // Computing light
    vec3 light_energy = vec3(0);
    float transmit = 1.0;
  

    // Create moving lights in our scene
    vec2 light_bulb_1 = vec2(-sin(u_time*0.9)*0.5 + cos(u_time*0.9)*0.3,-sin(u_time*0.9) + cos(u_time*0.4)*-0.2)*0.40 + 0.5;
    vec3 light_color_1 = vec3(1.0, 0.6, 0.9);
    float cloud_strength_1 = (1.0-(distance(textureCoordinates, light_bulb_1)));
    float light_strength_1 = 1.0/(5*distance(textureCoordinates,light_bulb_1));


    vec2 light_bulb_2 = vec2(sin(u_time*0.9)*0.5 + cos(u_time*0.9)*0.3,sin(u_time*0.9) + cos(u_time*0.4)*-0.2)*0.40 + 0.5;
    vec3 light_color_2 = vec3(0.0, 0.6, 0.9);
    float cloud_strength_2 = (1.0-(distance(textureCoordinates, light_bulb_2)));
    float light_strength_2 = 1.0/(5*distance(textureCoordinates,light_bulb_2));
    


    // Light source direction
    vec3 light_source_1 = normalize(light_color_1*light_strength_1 );
    vec3 light_source_2 = normalize(light_color_2*light_strength_2 );


 

    for (int i = 0; i < STEPS; i++)
    {  

             
        if (transmit < 0.1)
        {
            break;
        }
        
         
         // Get density of current position
         float density = map(pos);


         if (density > 0.001)
         {
            vec3 lpos = pos + light_energy;
            float shadow = 0.0;
            
            for (int s = 0; s < SHADOW_STEPS; s++)
            {
                lpos +=  light_source_1 * light_source_2 * shadow_step;
                shadow += map(lpos);
            }
                        
            float ds = density * inverse_step;
            float linearDensity = ds * OPACITY;
            linearDensity = clamp(linearDensity, 0.0, 1.0);
            
          
            float shadowterm = exp(-shadow * inverse_shadow_step * SHADOW_STRENGTH);
            
            
            float cloudDarkness = shadowterm * linearDensity * transmit;
            light_energy += cloud_color * cloudDarkness;
            

            // Out-scattering
            vec3 offset = pos + vec3(0.0, 0.25, 0.0);
            vec3 col = vec3(0.15, 0.45, 1.1);
            light_energy += col * (exp(-map(offset) * 0.2) * linearDensity * transmit) * (light_color_1*light_strength_1 + cloud_strength_1);
            light_energy += col * (exp(-map(offset) * 0.2) * linearDensity * transmit) * (light_color_2*light_strength_2 + cloud_strength_2);

            
            lpos = pos + vec3(0.0, 0.0, 0.05);
            float lsample = map(lpos);
            shadow += lsample;
            
           
            // Transmittance
            light_energy += ambient_color * (exp(-shadow *  AMBIENT_DENSITY) * linearDensity * transmit)  * (light_color_1*light_strength_1 + cloud_strength_1);
            light_energy += ambient_color * (exp(-shadow *  AMBIENT_DENSITY) * linearDensity * transmit)  * (light_color_2*light_strength_2 + cloud_strength_2);

          
            transmit *=  1.0 - linearDensity;
        }
        
        pos += dir * z_steps;
        

    }
    	

        // Sky color
        vec3 blue_color = mix(vec3(0.3, 0.6, 1.0), vec3(0.05, 0.35, 1.0), 1.0 );

        // Add the transmitted light with blue color to get a sky colour background
        light_energy += transmit * blue_color;

  
        return light_energy;
  
}



void main()
{

    // Get distance and calculate some movements to the camera
    vec3 position = CAM_DISTANCE * normalize(vec3(cos(u_time*0.5)+sin(u_time * 0.6), sin(u_time * 0.9) - tan(u_time* 0.2), -40));
    vec3 dir = normalize(vec3(textureCoordinates, 1.0));
    
    // Perform the ray march
    vec3 result = rayMarch(position, dir);

    // Mulitply by a lower value to get a darker scene
    float darkness = 0.4;
    color = vec4(result, 1.0) * darkness;
    
    

}


