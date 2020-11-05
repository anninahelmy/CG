//=============================================================================
//
//   Exercise code for the lecture "Introduction to Computer Graphics"
//     by Prof. Mario Botsch, Bielefeld University
//
//   Copyright (C) by Computer Graphics Group, Bielefeld University
//
//=============================================================================

#version 140

in vec3 v2f_normal;
in vec2 v2f_texcoord;
in vec3 v2f_light;
in vec3 v2f_view;

out vec4 f_color;

uniform sampler2D day_texture;
uniform sampler2D night_texture;
uniform sampler2D cloud_texture;
uniform sampler2D gloss_texture;
uniform bool greyscale;

const float shininess = 20.0;
const vec3  sunlight = vec3(1.0, 0.941, 0.898);

//The gloss and cloud textures are 1-channel (grayscale), whereas the day and night textures are 3-channel (RGB).
void main()
{
    //textures
    float clouds_tex = texture(cloud_texture,v2f_texcoord.st).r;
    float gloss = texture(gloss_texture, v2f_texcoord.st).r * (1-clouds_tex);
    vec3 day = texture(day_texture, v2f_texcoord.st).rgb;
    vec3 night_tex = texture(night_texture, v2f_texcoord.st).rgb;

    // normalize directions
    vec3 normal = normalize(v2f_normal);
    vec3 light = normalize(v2f_light);
    vec3 view = normalize(v2f_view);
    vec3 reflect = normalize(reflect(-light, normal));
    float ambient = 0.2;

    //I =I_a *m_a +I_l*m_d(n·l)+I_l*m_s(r·v)^s where:
    //The ambient and diffuse components of the material, ma and md, are given by the RGB
    //color sampled from the day texture, but the specular component ms should be pure white
    //(i.e. ms = [1, 1, 1]).

    // ambient light.
    day += ambient*sunlight*day;
    vec3 clouds = clouds_tex*sunlight*ambient;

    //diffuse
    if(dot(normal,light)>0){
        day += sunlight*day*dot(normal, light);
        vec3 clouds = sunlight*clouds_tex*dot(normal, light);
    }
    //specular
    if(dot(normal,light)>0 && dot(reflect, view)>0){
        float x = dot(reflect, view);
        float power = pow(x, shininess);
        day += gloss*sunlight*vec3(1,1,1)* power;
    }


    //Finally, mix the color from the Phong lighting calculation with the cloud color by linear interpolation
    //(using the cloud texture grayscale value as the interpolation weight) to obtain the day color.
    day = mix(day,clouds, clouds_tex);
    vec3 night= (1-clouds_tex)*night_tex;

    //mix day and night
    vec3 color = mix(night, day, dot(normal, light));

    if (greyscale) color = vec3(0.299*color.r+0.587*color.g+0.114*color.b);

    // add required alpha value
    f_color = vec4(color, 1.0);

}
