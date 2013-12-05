//
// template-rt.cpp
//

#define _CRT_SECURE_NO_WARNINGS
#include "matm.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstring>
#include <algorithm>
#include <vector>
using namespace std;

// Program Definitions
#define MAX_LIGHTS 5
#define MAX_SPHERES 5

// Debugging function. If defined, output the DEBUG calls to
// std err.  If not, get rid of them for release builds.
#define DEBUG_BUILD 1
#ifdef DEBUG_BUILD
#	define DEBUG(x) do { std::cerr << x; } while (0)
#else
#	define DEBUG(x) do {} while (0)
#endif

// Structure Definitions
struct Ray
{
    vec4 origin;
    vec4 dir;
};

struct Sphere
{
	string name;
	vec4 position;
	vec3 scale;
	vec4 color;
	float Ka;
	float Kd;
	float Ks;
	float Kr;
	float n;
};

struct Light
{
	string name;
	vec4 position;
	vec4 color;
};

enum IntersectionT
{
	LIGHT_SOURCE, NO_INTERSECTION, SPHERE, HOLLOW_SPHERE
};

struct Intersection
{
	Intersection() {};
	~Intersection() {};

	IntersectionT type;
	union {
		Light *light;
		Sphere *sphere;
	};
};

// Global Variables ///////////////////////////////////////////////////////////

vector<vec4> g_colors; //Actual pixel colors?

// Everything we need to load from files goes in here
Light g_lights[MAX_LIGHTS]; // Array of light info (d)
Sphere g_spheres[MAX_SPHERES]; // Array of sphere info (c)
int g_width;    // Resolution width  (b)
int g_height;   // Resolution height (b)
float g_left;   // Left plane    (a)
float g_right;  // Right plane   (a)
float g_top;    // Top plane     (a)
float g_bottom; // Bottom plane  (a)
float g_near;   // Near plane    (a)
vec4 g_bgcolor;// Background color (e)
vec4 g_ambient;// Ambient intensity  (f)
string g_output;// Output file name (g)

// A few interators to track where we are in the program
int iL = 0;
int iS = 0;


// -------------------------------------------------------------------
// Input file parsing

vec4 toVec4(const string& s1, const string& s2, const string& s3)
{
    stringstream ss(s1 + " " + s2 + " " + s3);
    vec4 result;
    ss >> result.x >> result.y >> result.z;
    result.w = 1.0f;
    return result;
}

vec3 toVec3(const string& s1, const string& s2, const string& s3)
{
	stringstream ss(s1 + " " + s2 + " " + s3);
	vec3 result;
	ss >> result.x >> result.y >> result.z;
	return result;
}

float toFloat(const string& s)
{
    stringstream ss(s);
    float f;
    ss >> f;
    return f;
}

void parseLine(const vector<string>& vs)
{
    //TODO: add parsing of SPHERE, LIGHT, BACK, AMBIENT, OUTPUT.
    if (vs[0] == "RES") 
	{
        g_width = (int)toFloat(vs[1]);
        g_height = (int)toFloat(vs[2]);
        g_colors.resize(g_width * g_height);
    }
	else if (vs[0] == "NEAR")
		g_near = toFloat(vs[1]);
	else if (vs[0] == "LEFT")
		g_left = toFloat(vs[1]);
	else if (vs[0] == "RIGHT")
		g_right = toFloat(vs[1]);
	else if (vs[0] == "BOTTOM")
		g_bottom = toFloat(vs[1]);
	else if (vs[0] == "TOP")
		g_top = toFloat(vs[1]);
	else if (vs[0] == "SPHERE") 
	{
		g_spheres[iS].name = vs[1];
		g_spheres[iS].position = toVec4(vs[2],vs[3],vs[4]);
		g_spheres[iS].scale = toVec3(vs[5],vs[6],vs[7]);
		g_spheres[iS].color = toVec4(vs[8], vs[9], vs[10]);
		g_spheres[iS].Ka = toFloat(vs[11]);
		g_spheres[iS].Kd = toFloat(vs[12]);
		g_spheres[iS].Ks = toFloat(vs[13]);
		g_spheres[iS].Kr = toFloat(vs[14]);
		g_spheres[iS].n = toFloat(vs[15]);

		iS++;
	}
	else if (vs[0] == "LIGHT")
	{
		g_lights[iL].name = vs[1];
		g_lights[iL].position = toVec4(vs[2],vs[3],vs[4]);
		g_lights[iL].color = toVec4(vs[5], vs[6], vs[7]);

		iL++;
	}
	else if (vs[0] == "BACK")
		g_bgcolor = toVec4(vs[1], vs[2], vs[3]);
	else if (vs[0] == "AMBIENT")
		g_ambient = toVec4(vs[1], vs[2], vs[3]);
	else if (vs[0] == "OUTPUT")
		g_output = vs[1];
}

void loadFile(const char* filename)
{
    ifstream is(filename);
    if (is.fail())
    {
        cout << "Could not open file " << filename << endl;
        exit(1);
    }
    string s;
    vector<string> vs;
    while(!is.eof())
    {
        vs.clear();
        getline(is, s);
        istringstream iss(s);
        while (!iss.eof())
        {
            string sub;
            iss >> sub;
            vs.push_back(sub);
        }
        parseLine(vs);
    }
}


// -------------------------------------------------------------------
// Utilities

void setColor(int ix, int iy, const vec4& color)
{
    int iy2 = g_height - iy - 1; // Invert iy coordinate.
    g_colors[iy2 * g_width + ix] = color;
}

void loadedSceneInfo()
{
	cout << "NEAR: " << g_near << endl;
	cout << "LEFT: " << g_left << endl;
	cout << "RIGHT: "<< g_right << endl;
	cout << "BOTTOM: " << g_bottom << endl;
	cout << "TOP: " << g_top << endl;
	cout << "RES: " << g_width << "x" << g_height << endl;

	int i;
	for (i = 0; i < iS; i++)
	{
		cout << "SPHERE # " << i << endl;
		cout << "   NAME: " << g_spheres[i].name << endl;
		cout << "   POS: " << g_spheres[i].position.x << ",";
		cout << g_spheres[i].position.y << ",";
		cout << g_spheres[i].position.z << endl;
		cout << "   SCALE: " << g_spheres[i].scale.x << ",";
		cout << g_spheres[i].scale.y << ",";
		cout << g_spheres[i].scale.z << endl;
		cout << "   COLOR: " << g_spheres[i].color.x << ",";
		cout << g_spheres[i].color.y << ",";
		cout << g_spheres[i].color.z << endl;
		cout << "   Ka: " << g_spheres[i].Ka << endl;
		cout << "   Kd: " << g_spheres[i].Kd << endl;
		cout << "   Ks: " << g_spheres[i].Ks << endl;
		cout << "   Kr: " << g_spheres[i].Kr << endl;
		cout << "   N: " << g_spheres[i].n << endl;
	}
	for (i = 0; i < iL; i++)
	{
		cout << "LIGHT # " << i << endl;
		cout << "   NAME: " << g_lights[i].name << endl;
		cout << "   POS: " << g_lights[i].position.x << ",";
		cout << g_lights[i].position.y << ",";
		cout << g_lights[i].position.z << endl;
		cout << "   COLOR: " << g_lights[i].color.x << ",";
		cout << g_lights[i].color.y << ",";
		cout << g_lights[i].color.z << endl;
	}

	cout << "BACK: " << g_bgcolor.x << ",";
	cout << g_bgcolor.y << ",";
	cout << g_bgcolor.z << endl;

	cout << "AMBIENT: " << g_ambient.x << ",";
	cout << g_ambient.y << ",";
	cout << g_ambient.z << endl;

	cout << "OUTPUT: " << g_output << endl;

}

bool colorOOR(const vec4& color)
{
	if ( 0.0 > color.x || color.x > 255.0
	  || 0.0 > color.y || color.y >= 255.0
	  || 0.0 > color.z || color.z > 255.0
	  || 1.0 == color.w )
		return true;
	else
		return false;
}

vec4 normal(const vec4& q, const Sphere& sphere)
{
	vec4 normal = q - sphere.position;
	mat4 trans = Scale(sphere.scale);
	mat4 invTrans;
	InvertMatrix(transpose(trans), invTrans);
	normal.w = 0;
	return normalize(invTrans*invTrans*normal);
}


// -------------------------------------------------------------------
// Intersection routine

vec4 intersect(const Ray& ray, Intersection& status)
{
	float distance = 1000.0;
	vec4 intPoint;

	status.type = NO_INTERSECTION;

	int i;
	for (i = 0; i < iS; i++)
	{
		vec4 toSphere = g_spheres[i].position - ray.origin;

		mat4 invTrans;
		InvertMatrix( Scale(g_spheres[i].scale), invTrans);

		vec4 S = invTrans * toSphere;
		vec4 C = invTrans * ray.dir;

		float a = dot(C, C);
		float b = dot(S, C);
		float c = dot(S, S) - 1;

		if( (b*b - a*c) < 0 ) continue;

		float root = sqrtf(b*b - a*c)/a;
		float soln1 = b/a - root;
		float soln2 = b/a + root;

		if ( (soln1 > 1.0f) && (soln1 < soln2) && (soln1 < distance) )
		{
			distance = soln1;
			intPoint = ray.origin + distance*ray.dir;
			status.type = SPHERE;
			status.sphere = &g_spheres[i];
		}
		else if ( (soln2 > 1.0f) && (soln2 < soln1) && (soln2 < distance) )
		{
			distance = soln2;
			intPoint = ray.origin + distance*ray.dir;
			status.type = SPHERE;
			status.sphere = &g_spheres[i];
		}
		else if ( (soln1 < 0.625f) && (soln1 > 0.0f) && (soln1 < soln2)
		       && (soln1 < distance) )
		{
			distance = soln1;
			intPoint = ray.origin + distance*ray.dir;
			status.type = HOLLOW_SPHERE;
			status.sphere = &g_spheres[i];
		}
		else if ( (soln2 < 0.625f) && (soln2 > 0.0f) && (soln2 < soln1)
		       && (soln2 < distance) )
		{
			distance = soln2;
			intPoint = ray.origin + distance*ray.dir;
			status.type = HOLLOW_SPHERE;
			status.sphere = &g_spheres[i];
		}
	}

	if ( status.type != NO_INTERSECTION ) return intPoint;
	else return vec4(0.0,0.0,0.0,1.0);
}

// -------------------------------------------------------------------
// Ray tracing

vec4 trace(Ray& ray, int step)
{
	vec4 point_q;
	vec4 normal_n;
	vec4 pixelColor;

	int max = 4;
	if ( step > max ) return (g_bgcolor);

	Intersection status;
	point_q = intersect(ray, status);

	if ( status.type == LIGHT_SOURCE ) return(status.light->color);
	if ( status.type == NO_INTERSECTION ) return(g_bgcolor);
	
	pixelColor = status.sphere->color * status.sphere->Ka * g_ambient;
	
	normal_n = normal(point_q, *status.sphere);

	vec4 diffuse = vec4( 0.0, 0.0, 0.0, 0.0);
	vec4 specular = vec4( 0.0, 0.0, 0.0, 0.0);

	int i;
	for (i = 0; i < iL; i++)
	{
		Ray light_ray;
		light_ray.origin = point_q;
		light_ray.dir = normalize(g_lights[i].position - point_q);

		Intersection l_status;
		intersect(light_ray, l_status);

		if ( l_status.type == NO_INTERSECTION )
		{
			float light_proj = dot(normal_n, light_ray.dir);
			if ( light_proj <= 0.0f ) continue;

			diffuse += dot(normal_n, light_ray.dir) * g_lights[i].color
			         * status.sphere->color;


			float view_proj = dot(normal_n, ray.dir);
			vec4 blinn = light_ray.dir - ray.dir;
			float spec_f = dot( blinn, blinn);
			if ( spec_f != 0.0 )
			{
				spec_f = 1.0/sqrtf(spec_f)*fmax(light_proj-view_proj,0.0f);
				spec_f = powf(spec_f, status.sphere->n);
				specular += spec_f * spec_f * spec_f * g_lights[i].color;
			}
		}
	}

	pixelColor += diffuse*status.sphere->Kd + specular*status.sphere->Ks;

	float reflect_f = 2.0f * dot(normal_n, ray.dir);
	ray.origin = point_q;
	ray.dir = normalize(ray.dir - reflect_f*normal_n);
	ray.origin = ray.origin + 0.0*ray.dir;
	vec4 reflection = trace(ray,step+1);
	if (!colorOOR(pixelColor+reflection*status.sphere->Kr))
		pixelColor += reflection*status.sphere->Kr;
	
	return (pixelColor);
}

vec4 getDir(int ix, int iy)
{
    // Return the direction from the origin to pixel (ix, iy), normalized.
	float x = g_left + ( ((float) ix)/g_width) * (g_right-g_left);
	float y = g_bottom + ( ((float) iy)/g_height) * (g_top-g_bottom);
    vec4 dir = vec4(x, y, -g_near, 0.0f);
    return normalize(dir);
}

void renderPixel(int ix, int iy)
{
    Ray ray;
    ray.origin = vec4(0.0f, 0.0f, 0.0f, 1.0f);
    ray.dir = getDir(ix, iy);
    vec4 color = trace(ray, 1);
    setColor(ix, iy, color);
}

void render()
{
    for (int iy = 0; iy < g_height; iy++)
        for (int ix = 0; ix < g_width; ix++)
            renderPixel(ix, iy);
}


// -------------------------------------------------------------------
// PPM saving

void savePPM(int Width, int Height, char* fname, unsigned char* pixels) 
{
    FILE *fp;
    const int maxVal=255;

    printf("Saving image %s: %d x %d\n", fname, Width, Height);
    fp = fopen(fname,"wb");
    if (!fp) {
        printf("Unable to open file '%s'\n", fname);
        return;
    }
    fprintf(fp, "P6\n");
    fprintf(fp, "%d %d\n", Width, Height);
    fprintf(fp, "%d\n", maxVal);

    for(int j = 0; j < Height; j++) {
        fwrite(&pixels[j*Width*3], 3, Width, fp);
    }

    fclose(fp);
}

void saveFile()
{
    // Convert color components from floats to unsigned chars.
    unsigned char* buf = new unsigned char[g_width * g_height * 3];
    for (int y = 0; y < g_height; y++)
        for (int x = 0; x < g_width; x++)
            for (int i = 0; i < 3; i++)
                buf[y*g_width*3+x*3+i] = (unsigned char)(fmin(((float*)g_colors[y*g_width+x])[i], 1.0f) * 255.9f);

	char *filename = new char[g_output.length() + 1];
	strcpy(filename, g_output.c_str());
    savePPM(g_width, g_height, filename, buf);
	delete[] filename;
    delete[] buf;
}


// -------------------------------------------------------------------
// Main

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        cout << "Usage: template-rt <input_file.txt>" << endl;
        exit(1);
    }
    loadFile(argv[1]);
	loadedSceneInfo();
    render();
    saveFile();
	return 0;
}

