#version 330

//#define antialiasing 0.5

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
uniform float eyeShift = 0.0;
uniform float fovScale = 1;
uniform vec3 eyeOffset;

// --- COMMON RAY PARAMETERS ---
vec3 rayDirection;
vec3 rayOrigin;
vec3 rayPos = vec3(0,0,0);
float lambda;
int steps;

// --- COMMON SHADING PARAMETERS ---
vec3 N;
vec3 V;
vec4 orb;
vec4 albedo;

// --------------------------------------------------
// --- FRACTAL DEFINITION ---
// --------------------------------------------------

// --- FRACTAL UNIFORM PARAMETERS ---
uniform int iterations;

// --- FRACTAL INTERNAL PARAMETERS ---
float anim = 0.0;

uniform float scale;
uniform float boxFold = 1.0;
uniform float boxScale = 0.5;
uniform float sphereScale = 1.0;
uniform float fudgeFactor = 0.0;
float smoothing = 0.6;
float minRadius2 = boxScale * boxScale;    // Min radius
float fixedRadius2 = sphereScale * minRadius2;      // Fixed radius
vec2  scaleFactor = vec2(scale, abs(scale)) / minRadius2;

mat3 timeRotation;

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
        //p.xyz *= fractalRotation1;
        p.xyz *= timeRotation;
        
        // sphere fold:
        // if (dist2 < minRad2) {
        //   p /= minRad2;
        // } else if (dist2 < 1.0) {
        //   p /= dist2;
        // }
        float dist2 = dot(p.xyz, p.xyz);
        p.xyzw *= clamp(max(fixedRadius2 / dist2, minRadius2), 0.0, 1.0);  // sphere fold
        
        p.xyzw = p * scaleFactor.xxxy + p0;// + vec4(offset, 0.0);
        //p.xyz *= fractalRotation2;

        //if (i < colorIterations) {
            minDist2 = min(minDist2, dist2);
            c = p.xyz;
        //}
    }
    
	float distEstimate = ((length(p.xyz) - fudgeFactor) / p.w);

	orb = vec4(distEstimate, minDist2, 0.33 * log(dot(c, c)) + 1.0, 1);
    
	// Return dist2 estimate, min dist2, fractional iteration count
    return distEstimate * smoothing; 

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

	 timeRotation = rotX * rotY * rotZ;//mat3(1,0,0,0,1,0,0,0,1);//rotZ * rotY * rotZ;
	 
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

// --- fog parameters ---
uniform vec3 fogColorA		= vec3(0.5,0.6,0.7);
uniform vec3 fogColorB		= vec3(10.0,9,7);
uniform vec3  sunDirection	= vec3(0,1,0);
uniform float sunExponent	= 8.0;
uniform float fogIntensity  = 0.0;

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

vec4 transformAlbedo() {

	vec2 b = clamp(orb.yz,0.0, 1.0);
	vec3 rgb = mix(color0, mix(color1, color2, b.x), b.y);
	
	return vec4(rgb,1);
}

vec3 lighting(vec3 L) {

	vec3 H = normalize(L+V);
	float diff = max(0.0, dot(N,L));
	float spec = pow(max(0.0,dot(N,H)),shininess);
	if (diff == 0.0) spec = 0.0;

	float f = pow(1.0 - dot(V,N), 4.0);

	return albedo.rgb * diffuse * diff + (spec * specular + f * fresnel) * vec3(1,1,1);
}

vec3 applyFog( in vec3  rgb,      // original color of the pixel
               in float distance, // camera to point distance
               in vec3  rayDir,   // camera to point vector
               in vec3  sunDir )  // sun light direction
{
    float fogAmount = 1.0 - exp( -distance * fogIntensity);
    float sunAmount = max( dot( rayDir, sunDir ), 0.0 );
    vec3  fogColor  = mix( fogColorA, // bluish
                           fogColorB, // yellowish
                           pow(sunAmount, sunExponent) );
    return mix( rgb, fogColor, fogAmount );
}

uniform float aoSpread		= 3.4;
uniform float aoIntensity	= 0.2;
uniform int aoIterations	= 4;

float ambientOcclusion(vec3 p, vec3 n, float eps)
{
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


vec3 shading() {
	
    N = normalize(estimateNormal(rayPos));
	V = -normalize(rayDirection);
	
    // lighting
    vec3  L1 = normalize(keyLightDir);
    vec3  L2 = normalize(backLightDir);
	vec3  L3 = normalize(rayDirection);

    //float amb = (0.7+0.3*N.y);
    //float ao = pow( clamp( orb.w * 2.0, 0.0, 1.0), 1.2);
	//float ambientOcc = amb * ao;
    
	float ambientOcc = ambientOcclusion(rayPos, N, 0.01);
	
	// material		
    albedo = transformAlbedo();

    vec3 brdf = vec3(0.0); 
	brdf += ambientColor * ambientOcc;
    brdf += keyLightColor  * lighting(L1) * ambientOcc;
    brdf += keyLightColor  * lighting(L3) * ambientOcc;
    brdf += backLightColor * lighting(L2) * ambientOcc;

    // color
    vec3 col = brdf * albedo.rgb;

	col = applyFog(col,lambda,rayDirection,	sunDirection);

	return sqrt(col);
}

vec3 background() {

	return applyFog(vec3(0.0),tmax,rayDirection,vec3(sunDirection));

}

// ------------------------------------------------
// --- BASIC RAYMARCHING SHADER ----
// ------------------------------------------------

float trace(vec3 rayOrigin, vec3 rayDirection)
{

	bool hit = false;
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

#ifdef antialiasing

void main()
{

	initSpace();

	vec3 col = vec3(0.0);
	float n = 0.0;
    for (float x = 0.0; x < 1.0; x += float(antialiasing)) {
        for (float y = 0.0; y < 1.0; y += float(antialiasing)) {
            vec2 uv = vfUV + vec2(x,y)/screenResolution;
			rayOrigin = getEyePoint(matView);
			rayDirection = getRayDirection(uv);
			rayDirection.x += matProj[2].x; // asymmetric frustum in x
			rayDirection = normalize(rayDirection);

			rayOrigin *= mat3(matView);
			rayDirection *= mat3(matView);
			
			rayOrigin += eyeOffset;
			 
			col += render(rayOrigin, rayDirection).rgb;
            n += 1.0;
        }
    } 
	
	col /= n;

	vec3 linear = pow(col, vec3(2.2));
	FragColor = vec4(linear, 1);
	
}

#else 

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

#endif