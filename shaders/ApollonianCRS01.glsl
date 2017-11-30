#version 330

// --- INPUT PARAMETERS ---
in vec2 vfUV;

// --- OUTPUT PARAMETERS ----
out vec4 FragColor;

// --- COMMON UNIFORM PARAMETERS ---
uniform vec2 screenResolution;
uniform mat4 matView;
uniform mat4 matProj;
uniform float time;

// --- COMMON RAYMARCHING PARAMETERS ---
uniform float tmax;
uniform float tmin;
uniform int maxSteps;

// --- COMMON VIEWING PARAMETERS ---
float u_eyeballCenterTweak = 0.0;
float u_fov_y_scale = 1;

// --- COMMON RAY PARAMETERS ---
vec3 rayDirection;
vec3 rayOrigin;
vec3 rayPos;
float lambda;

// --- COMMON SHADING PARAMETERS ---
vec3 N;
vec3 V;
vec4 orb;
vec4 albedo;

// --------------------------------------------------
// --- FRACTAL DEFINITION ---
// --------------------------------------------------

// --- FRACTAL UNIFORM PARAMETERS ---
uniform float transformExponent;
uniform float transformSpeed;
uniform vec3 shift;
uniform int iterations;

// --- FRACTAL INTERNAL PARAMETERS ---
float anim = 0.0;


float lnorm(vec3 vec) {
	float R = (sin(time/5.0) + 1.0)/2.0;
	vec3 other = vec3(0.2 * R * cos(time), R, 0.7 * R * sin(time));
	return dot(vec, other);

}
 
float sampleSpace( vec3 vecPos)
{
	
	float scale = 1.0;

	float sineWave = sin(time * transformSpeed) * 0.5 + 0.5;
	float spikyWave = 1.0-pow(1.0-sineWave, transformExponent);
    float tt = 1.1;// + spikyWave * 0.65;
    
	orb = vec4(1000.0); 
	
	vec3 vecPos1 = vecPos;
	for( int i=0; i < iterations; i++ )
	{
		vecPos1 = -1.0 + 2.0 * pow(fract(0.5 * vecPos1 + 0.5),vec3(1.05));
		float r2 = dot(vecPos1,vecPos1);        
        r2 = r2 * tt + lnorm(vecPos1) * (1.0 - tt);        
        orb = min( orb, vec4(abs(vecPos1),r2) );        
		float k = anim/r2;
		vecPos1 += mix(vec3(0.0),shift, (1.0 + sin(time/2.0))/2.0);
		vecPos1 *= k;
		scale *= k;


	}
	
	return 0.125*abs(vecPos.y)/scale;
}

float timeWarp() {
	return 1.1;//(1.1 + 0.5 * smoothstep( -0.3, 0.3, cos(time) ));
}

void initSpace() {
	anim = timeWarp();
}

// --- SHADING UNIFORM PARAMETERS ---

uniform vec3 color0;
uniform vec3 color1;
uniform vec3 color2;
uniform vec3 keyLightDir;
uniform vec3 backLightDir;
uniform vec3 ambientColor;
uniform vec3 keyLightColor;
uniform vec3 backLightColor;
uniform float diffuse;
uniform float specular;
uniform float fresnel;
uniform float shininess;

float epsilon = 0.001;
vec3 estimateNormal( in vec3 pos)
{
	vec3 gradMin = vec3(
		sampleSpace(pos - vec3(epsilon, 0, 0)),
		sampleSpace(pos - vec3(0, epsilon, 0)),
		sampleSpace(pos - vec3(0, 0, epsilon)));
	

	vec3 gradMax = vec3(
		sampleSpace(pos + vec3(epsilon, 0, 0)),
		sampleSpace(pos + vec3(0, epsilon, 0)),
		sampleSpace(pos + vec3(0, 0, epsilon)));
	

	return normalize(gradMax-gradMin);
}

vec4 transformAlbedo() {
	
	vec3 rgb = color0;
	rgb = mix( rgb, color1, clamp(6.0 * orb.y,0.0,1.0) );
	rgb = mix( rgb, color2, pow(clamp(1.0 - 2.0 * orb.z, 0.0, 1.0), 100.0));
	return vec4(rgb, 1);
	
	///vec3 s = normalize(orb.rgb);
	///vec3 rgb = color0 * orb.x + color1 * orb.y + color2 * orb.z;
	///
	///return vec4(orb.bbb,1);
}

vec3 lighting(vec3 L) {

	vec3 H = normalize(L+V);
	float diff = max(0.0, dot(N,L));
	float spec = pow(max(0.0,dot(N,H)),shininess);
	if (diff == 0.0) spec = 0.0;

	float f = pow(1.0 - dot(V,N), 4.0);

	return albedo.rgb * diffuse * diff + (spec * specular + f * fresnel) * vec3(1,1,1);
}


vec3 shading() {
		
    N = normalize(estimateNormal(rayPos));
	V = -normalize(rayDirection);
	
    // lighting
    vec3  L1 = normalize(keyLightDir);
    vec3  L2 = normalize(backLightDir);
	vec3  L3 = normalize(rayDirection);

    float amb = (0.7+0.3*N.y);
    float ao = pow( clamp( orb.w * 2.0, 0.0, 1.0), 1.2);
	
    // material		
    albedo = transformAlbedo();

    vec3 brdf = vec3(0.0); 
	brdf += ambientColor * amb * ao;
    brdf += keyLightColor  * lighting(L1) * ao * amb;
    brdf += keyLightColor  * lighting(L3) * ao * amb;
    brdf += backLightColor * lighting(L2) * ao * amb;

    // color
    vec3 col = brdf * albedo.rgb * exp(-3.0 * lambda * lambda * lambda);

	return sqrt(col);
}

vec3 background() {
	return vec3(0.0);
}

// ------------------------------------------------
// --- BASIC RAYMARCHING SHADER ----
// ------------------------------------------------

float trace(vec3 rayOrigin, vec3 rayDirection)
{

    float t = tmin;
    
	for(int i = 0; i < maxSteps; i++ )
    {
	    float precis = 0.001 * t;
         
		rayPos = rayOrigin + rayDirection * t;

	    float h = sampleSpace(rayPos);

        if( h < precis || t > tmax ) break;
        
		t += h;
    }

    if( t > tmax ) t = -1.0;
    return t;
}


vec3 render( in vec3 rayOrigin, in vec3 rayDirection)
{
    vec3 col = vec3(0.0);
    
	lambda = trace(rayOrigin, rayDirection);
    
	if (lambda > 0.0 ) {
		col = shading();
    } else {
		col = background();
	}

    return col;
}

vec3 getEyePoint(mat4 matView)
{
    vec3 rayOrigin = -matView[3].xyz;
    rayOrigin.x += u_eyeballCenterTweak; // apply stereo tweak
    return rayOrigin;
}

vec3 getRayDirection(vec2 uv)
{
    float aspect = screenResolution.x / screenResolution.y;
    vec3 dir = vec3(
        uv.x * u_fov_y_scale * aspect,
        uv.y * u_fov_y_scale,
        -1.);
    return normalize(dir);
}


void main()
{

    vec2 uv = vfUV;
    rayOrigin = getEyePoint(matView);
    rayDirection = getRayDirection(uv);
    rayDirection.x += matProj[2].x; // asymmetric frustum in x
    rayDirection = normalize(rayDirection);

    rayOrigin *= mat3(matView);
    rayDirection *= mat3(matView);
	
	initSpace();
	 
	vec3 col = render(rayOrigin, rayDirection);

	vec3 linear = pow(col, vec3(2.2));
	FragColor = vec4(linear, 1);
}