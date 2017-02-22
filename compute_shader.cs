#version 430

uniform float minx; 
uniform float miny;
uniform float dy;
uniform float dx;
uniform float resx;
uniform float resy;

layout(binding = 0, rgba32f) writeonly uniform image2D destTex;

layout (local_size_x = 8, local_size_y = 8) in;

void main() {
	ivec2 pix = ivec2(gl_GlobalInvocationID.xy);
	ivec2 size = imageSize(destTex);
	if (pix.x >= size.x || pix.y >= size.y)
		return;

	float x, y, cx ,cy;
    int iter;
    cx = minx + pix.x*dx/resx;
    cy = miny + pix.y*dy/resy;
    x = 0, y = 0;
    iter = 0;
    
    int maxiter = 256;
    while (x*x + y*y < 4.0 && iter < maxiter) {
        float tmp = x*x - y*y + cx; 
        y = 2*x*y + cy;
        x = tmp;
        iter++;
    }
    vec3 color;
    if (iter == maxiter) {
        color = vec3(0,0,0);
    } else {
    	float R = (iter & 31)/31.0;
    	float G = (iter & 63)/63.0;
    	float B = (iter & 15)/15.0;
        color = vec3(R, G, B);
    }


	imageStore(destTex, pix, vec4(color, 1.0));
}