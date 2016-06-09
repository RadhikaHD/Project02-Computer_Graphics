/*
 * Kamana, Dhaipule, Stith
 * March 3, 2015
 * Vertex shader
 * Normalized Blinn-Phong Shading
 */


varying vec3 wc_normal, wc_position;

void main() {
	wc_normal = gl_NormalMatrix*gl_Normal;
	wc_position = gl_ModelViewMatrix*gl_Vertex;
	gl_Position = gl_ProjectionMatrix*gl_ModelViewMatrix*gl_Vertex;
}


