/*
 * Kamana, Dhaipule, Stith
 * March 3, 2015
 * Fragment shader
 * Normalized Blinn-Phong Shading
 */

varying vec3 wc_normal, wc_position;

void main() {
	int i;
	for( i=0; i<3; i++ ) {
		vec3 P, N, L, V, H;
		vec4 diffuse_color = gl_FrontMaterial.diffuse; 
		vec4 specular_color = gl_FrontMaterial.specular; 
		float shininess = gl_FrontMaterial.shininess;

		P = wc_position;
		N = normalize(wc_normal);
		L = normalize(gl_LightSource[i].position - P);
		V = normalize(-P);
		H = normalize(L+V);
		
		diffuse_color *= max(dot(N,L),0.0);
		
		diffuse_color.x *= gl_LightSource[i].diffuse.x;
		diffuse_color.y *= gl_LightSource[i].diffuse.y;
		diffuse_color.z *= gl_LightSource[i].diffuse.z;

		specular_color.x *= gl_LightSource[i].specular.x;
		specular_color.y *= gl_LightSource[i].specular.y;
		specular_color.z *= gl_LightSource[i].specular.z;

		specular_color *= pow(max(dot(H,N),0.0),shininess);
		
		// Normalize
		float normalize = ( shininess + 2.0 ) / (8.0*3.14159);
		specular_color *= normalize;
		gl_FragColor += diffuse_color + specular_color;
	}	
}



