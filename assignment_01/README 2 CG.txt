# README 2 CG 
*Gruppe: R. Stern, A.Helmy, B.Helmy* 

## Phong Lighting Model and Shadows
We started off with implementing a for loop for the light sources, there we added the diffuse and specular contribution. 
Diffuse and Specular Contribution were implemented just like the formulas given in the lecture. 
Diffusion: I_l*m_d(n\dot l) 
		* I_l is the intensity of light source,
		*  m_d is materials diffuse reflection coefficient.
		*  light direction l and normalized vector n
Specular: I_l*m_s(r\dotv) 
		* I_l is the intensity of light source,
		*  m_s the materials specular reflection coefficient
		*  r reflected ray = 2n(n\dot l) - l
		*  v refracted ray
With only these two computed „values“ output_1 was generated. Obviously the shadows were missing, but it wasn’t 100% clear to me, why nothing usable was actually rendered. 
When implementing the shadows and running the raytracer output_2 was rendered. It does actually look a lot better! 
But still, not quite right.  Maybe the order is wrong in the code ? As first the diffusion, then the check for rays and then the specular contributions were implemented… 
So first of all, we checked, whether object is in shadow and discarded diffuse and specular contribution if light source is blocked by another object. But then output_3 was rendered. So apparently only the ambience is rendered - but where did the specular and diffusion contribution go? 
So maybe it is because we didn’t filter whether the scalar products n*l, r*v are bigger than zero? But implementing this constraint returned the same picture as before. 
With changing some code output4.tga was outputted - we remembered having this problem discussed in the lecture, so an offset and discarding secondary intersection points were being added, output5.tga was returned. This does look a lot better! 
The only thing which still didn’t work, were the shadows. It was absolutely unclear to us, why not - the code itself does look correct… 

## Reflections
We then implemented the reflections. The code is quite straightforward. The if-clause checks the maximal depth of the iteration and also, whether the material is shiny and therefore reflective or not. The reflected vector describes the direction of the reflected ray, which is actually the reflection at the surface normal of the incoming viewing ray. Then the reflected ray, again with an added shadow ray offset is defined. 
The reflectiveness of the material of the object is being defined and with the given formula for the color with linear interpolation, the color is being outputted.  The reflected_color must be recursive - so here the recursion happens!“ /recursively tracing a ray reflected at the intersection point/„
Implementing this part was actually far easier than the phong lightning… 

Going back to the problem with the shadows, after some hours we realized that we defined the shadow ray wrong. We had `Ray shadowRay = (_point + l_direction*shadow_ray_offset, -l_direction);`in our definition, but the `=` sign is wrong, therefore the shadowRay was never initialized. 
Output6.tga is the correct one. 
