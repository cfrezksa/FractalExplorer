

// ------------------------- MAIN SNIPPET  -----------------------------------
#version 330

// --- INPUT PARAMETERS ---
in vec2 vfUV;

// --- OUTPUT PARAMETERS ----
out vec4 FragColor;

// --- COMMON UNIFORM PARAMETERS ---
uniform vec2 screenResolution;
uniform mat4 matView; 
uniform mat4 matProj;
uniform float eyeShift = 0.0; 
uniform vec3 eyeOffset = vec3(0,0,0);
uniform float fovScale = 1.0;

vec3 rayDirection;
vec3 rayOrigin;

vec3 render(vec3 a, vec3 d);
void initSpace();

// --- snip ----

vec3 getEyePoint(mat4 matView)
{
    vec3 rayOrigin = -matView[3].xyz;
    rayOrigin.x += eyeShift; // apply stereo tweak
    return rayOrigin;
}

vec3 getRayDirection(vec2 uv)
{
    float aspect = screenResolution.x / screenResolution.y;
    vec3 dir = vec3(
        uv.x * fovScale * aspect,
        uv.y * fovScale,
        -1.);
    return normalize(dir);
}

void main()
{
	initSpace();

    vec2 uv = vfUV;
    rayOrigin = getEyePoint(matView);
    rayDirection = getRayDirection(uv);
    rayDirection.x += matProj[2].x; // asymmetric frustum in x
    rayDirection = normalize(rayDirection);

    rayOrigin *= mat3(matView);
    rayDirection *= mat3(matView);
	
	rayOrigin += eyeOffset;
	
	vec3 col = render(rayOrigin, rayDirection);

	vec3 linear = pow(col, vec3(2.2));
	FragColor = vec4(linear, 1);
}

// -------------------- END OF MAIN SNIPPET  -----------------------------------

// ------------------------- RENDER SNIPPET  -----------------------------------

float lambda;
bool hit = false;

float trace(vec3 r, vec3 d);
vec3 shading();
vec3 background();

vec3 render( in vec3 rayOrigin, in vec3 rayDirection)
{
    vec3 col = vec3(0.0);
    
	lambda = trace(rayOrigin, rayDirection);
    
	if (hit && (lambda > 0.0 )) {
		col = shading();
    } else {
		col = background();
	}

    return col;
}

// -------------------- END OF RENDER SNIPPET  -----------------------------------

// --------------------------TRACE SNIPPET  --------------------------------------

float sampleSpace(vec3 pos);

// --- COMMON RAYMARCHING PARAMETERS ---
uniform float tmax;
uniform float tmin;
uniform int maxSteps;
vec3 rayPos = vec3(0,0,0);
int steps;

float trace(vec3 rayOrigin, vec3 rayDirection)
{

    float t = tmin;
    
	for(int i = 0; i < maxSteps; i++ )
    {
		steps = i;
	    float precis = 0.001 * t;
         
		rayPos = rayOrigin + rayDirection * t;

	    float h = sampleSpace(rayPos);

		if ((h < precis) && hit) {
			steps--;
			break;
		}
		
		hit = false;

        if( h < precis || t > tmax ) {
			hit = true;
		}
        
		t += h;
    }

    if( t > tmax ) t = -1.0;
    return t;
}

// -------------------- END OF TRACE SNIPPET  -----------------------------------

// -------------------- AMBIENT_OCCLUSION SNIPPET  -----------------------------------

uniform float aoSpread		= 3.4;
uniform float aoIntensity	= 0.2;
uniform int aoIterations	= 4;

float ambientOcclusion(vec3 p, vec3 n, float eps)
{
	if (steps == 0) return 0.5;

    float o = 1.0;                  // Start at full output colour intensity
    eps *= aoSpread;                // Spread diffuses the effect
    float k = aoIntensity / eps;    // Set intensity factor
    float d = 2.0 * eps;            // Start ray a little off the surface
    
    for (int i = 0; i < aoIterations; ++i) {
        o -= (d - sampleSpace(p + n * d)) * k;
        d += eps;
        k *= 0.5;                   // AO contribution drops as we move further from the surface 
    }
    
    return clamp(o, 0.0, 1.0);
}

// -------------------- END OF AMBIENT_OCCLUSION SNIPPET  -----------------------------------


// -------------------------- SHADING SNIPPET  --------------------------------------

uniform vec3 backgroundColor = vec3(0,0,0);
uniform vec3 keyLightDir;
uniform vec3 keyLightColor;

uniform float fog = 0.5;
uniform float fogFalloff = 0.001;
uniform float fogPower = 1.0;

vec3 N;
vec3 V;
vec4 albedo;

vec3 estimateNormal(vec3 p);
vec3 lighting(vec3 p);
vec4 computeAlbedo();
void applyFog(inout vec3 col);
void applyInnerGlow(inout vec3 col);
void applyOuterGlow(inout vec3 col);

vec3 shading() {
	
    N = normalize(estimateNormal(rayPos));

	V = -normalize(rayDirection);
	
    vec3  L = normalize(keyLightDir);

	// material		
    albedo = computeAlbedo();

    vec3 col = keyLightColor * lighting(L);

	float ao = ambientOcclusion(rayPos, N, 0.001);
	
	applyInnerGlow(col);
	applyFog(col);

	return col * ao;
}

vec3 background() {

	vec3 col = backgroundColor;
	applyOuterGlow(col);
	applyFog(col);
	return col;
}

// -------------------- END OF SHADING SNIPPET  -----------------------------------

// ------------------------ FOG SNIPPET  -------------------------------------

void applyFog(inout vec3 col) {
	col = mix(backgroundColor, col, exp(-pow((lambda-tmin) * exp(fogFalloff), fogPower) * fog));
}

// -------------------- END OF FOG SNIPPET  -----------------------------------


// ------------------------ INNER GLOW SNIPPET  -------------------------------------

uniform vec3 innerGlowColor = vec3(0,0,4);
uniform float innerGlowIntensity = 0.05;

void applyInnerGlow(inout vec3 col) {

	float glowAmount = float(steps)/float(maxSteps);
    float glow = clamp(glowAmount * innerGlowIntensity * 3.0, 0.0, 1.0);

	col = mix(col, innerGlowColor, glow);
	
}

// -------------------- END OF INNER GLOW SNIPPET  -----------------------------------

// ------------------------ OUTER GLOW SNIPPET  -------------------------------------

uniform vec3 outerGlowColor = vec3(1,0,0);
uniform float outerGlowIntensity = 0.02;

void applyOuterGlow(inout vec3 col) {

	float glowAmount = float(steps)/float(maxSteps);
    float glow = clamp(glowAmount * outerGlowIntensity * 3.0, 0.0, 1.0);

	col.rgb = mix(col.rgb, outerGlowColor, glow);
}

// -------------------- END OF OUTER GLOW SNIPPET  -----------------------------------


// ------------------------ LIGHTING SNIPPET  -------------------------------------

uniform float diffuse;
uniform float specular;
uniform float shininess;
uniform float fresnel;

vec3 lighting(vec3 L) {

	vec3 H = normalize(L+V);
	float diff = max(0.0, dot(N,L));
	float spec = pow(max(0.0,dot(N,H)),shininess);
	if (diff == 0.0) spec = 0.0;

	float f = pow(1.0 - dot(V,N), 4.0);

	return albedo.rgb * diffuse * diff + (spec * specular * f) * vec3(1,1,1);
}

// -------------------- END OF LIGHTING SNIPPET  ----------------------------------

// ------------------------ ESTIMATE NORMAL SNIPPET  -------------------------------------

float epsilon = 0.001;

vec3 estimateNormal( in vec3 pos)
{
	if (steps == 0) return -rayDirection;

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

// ------------------------ END OF ESTIMATE NORMAL SNIPPET  ----------------------------

// ------------------------ FRACTAL SNIPPET  ----------------------------


// --- FRACTAL INTERNAL PARAMETERS ---
float anim = 0.0;

uniform int iterations = 6;
uniform int colorIterations = 4;
uniform float smoothing = 0.95;

uniform vec3 color0;
uniform vec3 color1;
uniform vec3 color2;

uniform float scale;
uniform float boxFold = 1.0;
uniform float boxScale = 0.5;
uniform float sphereScale = 1.0;
uniform float fudgeFactor = 0.0;
uniform vec3 offset = vec3(0.2,0.3,0.1);
uniform mat3  fractalRotation1 = mat3(1,0,0, 0,1,0, 0,0,1);
uniform mat3  fractalRotation2 = mat3(1,0,0, 0,1,0, 0,0,1);
mat3  timeRotation = mat3(1,0,0, 0,1,0, 0,0,1);

float minRadius2 = boxScale * boxScale;    // Min radius
float fixedRadius2 = sphereScale * minRadius2;      // Fixed radius
vec2  scaleFactor = vec2(scale, abs(scale)) / minRadius2;

uniform float time;

vec4 orb;

float sampleSpace( vec3 vecPos)
{
	float minDist2 = 1000.0;
    vec3 c = vecPos;
    
	orb = vec4(10000.0,1,0,1);

    // dist2 estimate
    vec4 p = vec4(vecPos, 1.0);
	vec4 p0 = p;  // p.w is knighty's DEfactor
    
    for (int i = 0; i < iterations; i++) {
        // box fold:
        // if (p > 1.0) {
        //   p = 2.0 - p;
        // } else if (p < -1.0) {
        //   p = -2.0 - p;
        // }
        p.xyz = clamp(p.xyz, -boxFold, boxFold) * 2.0 * boxFold - p.xyz;  // box fold
        p.xyz *= fractalRotation1;
        p.xyz *= timeRotation;
        
		  // sphere fold:
        // if (dist2 < minRad2) {
        //   p /= minRad2;
        // } else if (dist2 < 1.0) {
        //   p /= dist2;
        // }
        float dist2 = dot(p.xyz, p.xyz);
        p.xyzw *= clamp(max(fixedRadius2 / dist2, minRadius2), 0.0, 1.0);  // sphere fold
        
        p.xyzw = p * scaleFactor.xxxy + p0 + vec4(offset, 0.0);
        p.xyz *= fractalRotation2;

        if (i < colorIterations) {
            minDist2 = min(minDist2, dist2);
            c = p.xyz;
        }
    }
    
	float distEstimate = ((length(p.xyz) - fudgeFactor) / p.w);

	orb = vec4(distEstimate, minDist2, 0.33 * log(dot(c, c)) + 1.0, 1);
    
	// Return dist2 estimate, min dist2, fractional iteration count
    return distEstimate * smoothing; 

}

vec4 computeAlbedo() {

	vec2 b = clamp(orb.yz, 0.0, 1.0);
	vec3 rgb = mix(color0, mix(color1, color2, b.x), b.y);
	
	return vec4(rgb,1);
}

void initSpace() {
	
	 float tx = -0.032 * time + 0.139;
	 float cx = cos(tx);
	 float sx = sin(tx);
	 mat3 rotX = mat3( cx, 0, -sx, 0,  1,  0,  sx, 0,  cx);
	 
	 float ty = 0.123 * time + 1.932;
	 
	 float cy = cos(ty);
	 float sy = sin(ty);
	 mat3 rotY = mat3( cy, 0, -sy, 0,  1,  0,  sy, 0,  cy);
	 
	 float tz = 0.092 * time - 0.971;
	 float cz = cos(tz);
	 float sz = sin(tz);
	 mat3 rotZ = mat3(cz, -sz, 0, sz, cz, 0, 0, 0, 1);
	 
	 timeRotation = rotX * rotY * rotZ;
	
}
