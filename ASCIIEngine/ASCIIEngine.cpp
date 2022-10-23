#include "olcConsoleGameEngine.h"
#include <fstream>
#include <strstream>
#include <algorithm>
using namespace std;
struct vec3d {
	float x, y, z;
};
struct triangle {
	vec3d p[3];
	wchar_t sym;
	short col;
};
struct mesh {
	vector<triangle> tris;
	bool lobj(string filename) { // load object file 
		ifstream f(filename);
		if (!f.is_open()) {
			cout << "Couldn't load object file." << endl;
			return false;
		}
		vector<vec3d> verts;
		while (!f.eof())
		{
			char line[128];
			f.getline(line, 128);
			strstream s;
			s << line;
			char junk;
			if (line[0] == 'v') {
				vec3d v;
				s >> junk >> v.x >> v.y >> v.z;
				verts.push_back(v);
			}
			if (line[0] == 'f') {
				int f[3];
				s >> junk >> f[0] >> f[1] >> f[2];
				tris.push_back({ verts[f[0] - 1], verts[f[1] - 1], verts[f[2] - 1] });
			}
		}
		return true;
	}
};
struct mat4x4 {
	float m[4][4] = { 0 };
};
class ASCIIEngine : public ConsoleGameEngine {
public:
	ASCIIEngine() {
		m_sAppName = L"3D Engine";
	}
private:
	mesh meshCube;
	mat4x4 matp; // mat projection
	vec3d vc; // camera
	float t; // theta
	void mmv(vec3d &i, vec3d &o, mat4x4 &m) {
		o.x = i.x * m.m[0][0] + i.y * m.m[1][0] + i.z * m.m[2][0] + m.m[3][0];
		o.y = i.x * m.m[0][1] + i.y * m.m[1][1] + i.z * m.m[2][1] + m.m[3][1];
		o.z = i.x * m.m[0][2] + i.y * m.m[1][2] + i.z * m.m[2][2] + m.m[3][2];
		float w = i.x * m.m[0][3] + i.y * m.m[1][3] + i.z * m.m[2][3] + m.m[3][3];
		if (w != 0.0f) {
			o.x /= w; o.y /= w; o.z /= w;
		}
	}//multiply matrix vector
	CHAR_INFO GetColour(float lum)
	{
		short bg_col, fg_col;
		wchar_t sym;
		int pixel_bw = (int)(13.0f*lum);
		switch (pixel_bw)
		{
		case 0: bg_col = BG_BLACK; fg_col = FG_BLACK; sym = PIXEL_SOLID; break;

		case 1: bg_col = BG_BLACK; fg_col = FG_DARK_GREY; sym = PIXEL_QUARTER; break;
		case 2: bg_col = BG_BLACK; fg_col = FG_DARK_GREY; sym = PIXEL_HALF; break;
		case 3: bg_col = BG_BLACK; fg_col = FG_DARK_GREY; sym = PIXEL_THREEQUARTERS; break;
		case 4: bg_col = BG_BLACK; fg_col = FG_DARK_GREY; sym = PIXEL_SOLID; break;

		case 5: bg_col = BG_DARK_GREY; fg_col = FG_GREY; sym = PIXEL_QUARTER; break;
		case 6: bg_col = BG_DARK_GREY; fg_col = FG_GREY; sym = PIXEL_HALF; break;
		case 7: bg_col = BG_DARK_GREY; fg_col = FG_GREY; sym = PIXEL_THREEQUARTERS; break;
		case 8: bg_col = BG_DARK_GREY; fg_col = FG_GREY; sym = PIXEL_SOLID; break;

		case 9:  bg_col = BG_GREY; fg_col = FG_WHITE; sym = PIXEL_QUARTER; break;
		case 10: bg_col = BG_GREY; fg_col = FG_WHITE; sym = PIXEL_HALF; break;
		case 11: bg_col = BG_GREY; fg_col = FG_WHITE; sym = PIXEL_THREEQUARTERS; break;
		case 12: bg_col = BG_GREY; fg_col = FG_WHITE; sym = PIXEL_SOLID; break;
		default:
			bg_col = BG_BLACK; fg_col = FG_BLACK; sym = PIXEL_SOLID;
		}

		CHAR_INFO c;
		c.Attributes = bg_col | fg_col;
		c.Char.UnicodeChar = sym;
		return c;
	}
public:
	bool ouc() override {
		meshCube.lobj("objs/ball.obj"); // change this to whatever obj you want
		// list of objects:
		/*
		objs/sanik.obj
		objs/ball.obj
		objs/cube.obj
		objs/telephone.obj
		the default intensity is 0.5f
		if you are using cube.obj, go to the light intensity variable and change it to 8.0f (use the find tool in your IDE and search up intensity)
		if you are using telephone.obj, go to the light intensity variable and change it to 0.09f (use the find tool in your IDE and search up intensity)
		*/
		// TODO: maybe add more objects
		float n = 0.1f;// near
		float f = 1000.0f;//far
		float fov = 90.0f;
		float ar = (float)sh() / (float)sw();//im sorry there is no AR, its the aspect ratio
		float fovr = 1.0f / tanf(fov * 0.5f / 180.0f * 3.14159f);//tangent calculation
		matp.m[0][0] = ar * fovr;
		matp.m[1][1] = fovr;
		matp.m[2][2] = f / (f - n);
		matp.m[3][2] = (-f * n) / (f - n);
		matp.m[2][3] = 1.0f;
		matp.m[3][3] = 0.0f;
		return true;
	}
	bool oup(float fet) override{
		Fill(0, 0, sw(), sh(), PIXEL_SOLID, FG_BLACK);
		mat4x4 matRotZ, matRotX;
		t += 1.0f * fet;
		matRotZ.m[0][0] = cosf(t);
		matRotZ.m[0][1] = sinf(t);
		matRotZ.m[1][0] = -sinf(t);
		matRotZ.m[1][1] = cosf(t);
		matRotZ.m[2][2] = 1;
		matRotZ.m[3][3] = 1;
		matRotX.m[0][0] = 1;
		matRotX.m[1][1] = cosf(t * 0.5f);
		matRotX.m[1][2] = sinf(t * 0.5f);
		matRotX.m[2][1] = -sinf(t * 0.5f);
		matRotX.m[2][2] = cosf(t * 0.5f);
		matRotX.m[3][3] = 1;
		vector<triangle> ttr; //triangles to raster
		for (auto tri : meshCube.tris) {
			triangle trip, trit, triz, trizx; //triangle projected, triangle translated, triangle rotated Z and triangle rotated ZX
			mmv(tri.p[0], triz.p[0], matRotZ);
			mmv(tri.p[1], triz.p[1], matRotZ);
			mmv(tri.p[2], triz.p[2], matRotZ);
			mmv(triz.p[0], trizx.p[0], matRotX);
			mmv(triz.p[1], trizx.p[1], matRotX);
			mmv(triz.p[2], trizx.p[2], matRotX);
			trit = trizx;
			trit.p[0].z = trizx.p[0].z + 8.0f;
			trit.p[1].z = trizx.p[1].z + 8.0f;
			trit.p[2].z = trizx.p[2].z + 8.0f;
			vec3d normal, line1, line2;
			line1.x = trit.p[1].x - trit.p[0].x;
			line1.y = trit.p[1].y - trit.p[0].y;
			line1.z = trit.p[1].z - trit.p[0].z;
			line2.x = trit.p[2].x - trit.p[0].x;
			line2.y = trit.p[2].y - trit.p[0].y;
			line2.z = trit.p[2].z - trit.p[0].z;
			normal.x = line1.y * line2.z - line1.z * line2.y;
			normal.y = line1.z * line2.x - line1.x * line2.z;
			normal.z = line1.x * line2.y - line1.y * line2.x;
			float l = sqrtf(normal.x*normal.x + normal.y*normal.y + normal.z*normal.z);
			normal.x /= 1; normal.y /= 1; normal.z /= 1;
			if (normal.x*(trit.p[0].x - vc.x) + normal.y*(trit.p[0].y - vc.y)+normal.z*(trit.p[0].z - vc.z) < 0.0f){
				vec3d lightd = { 0.0f, 0.0f, -1.0f }; //light direction
				float l = sqrtf(lightd.x*lightd.x + lightd.y*lightd.y + lightd.z*lightd.z);
				float intensity = 0.5f; // light intensity variable
				lightd.x /= intensity; lightd.y /= intensity; lightd.z /= intensity;
				float dp = normal.x*lightd.x + normal.y*lightd.y + normal.z*lightd.z;
				CHAR_INFO c = GetColour(dp);
				trit.col = c.Attributes;
				trit.sym = c.Char.UnicodeChar;
				mmv(trit.p[0], trip.p[0], matp);
				mmv(trit.p[1], trip.p[1], matp);
				mmv(trit.p[2], trip.p[2], matp);
				trip.col = trit.col;
				trip.sym = trit.sym;
				trip.p[0].x += 1.0f; trip.p[0].y += 1.0f;
				trip.p[1].x += 1.0f; trip.p[1].y += 1.0f;
				trip.p[2].x += 1.0f; trip.p[2].y += 1.0f;
				trip.p[0].x *= 0.5f * (float)sw();
				trip.p[0].y *= 0.5f * (float)sh();
				trip.p[1].x *= 0.5f * (float)sw();
				trip.p[1].y *= 0.5f * (float)sh();
				trip.p[2].x *= 0.5f * (float)sw();
				trip.p[2].y *= 0.5f * (float)sh();
				ttr.push_back(trip);
			}
		}
		sort(ttr.begin(), ttr.end(), [](triangle &t1, triangle &t2){
			float z1 = (t1.p[0].z + t1.p[1].z + t1.p[2].z) / 3.0f;
			float z2 = (t2.p[0].z + t2.p[1].z + t2.p[2].z) / 3.0f;
			return z1 > z2;
		});
		for (auto &trip : ttr) {
			FillTriangle(trip.p[0].x, trip.p[0].y,
				trip.p[1].x, trip.p[1].y,
				trip.p[2].x, trip.p[2].y,
				trip.sym, trip.col);
			DrawTriangle(trip.p[0].x, trip.p[0].y, trip.p[1].x, trip.p[1].y, trip.p[2].x, trip.p[2].y, PIXEL_SOLID, FG_YELLOW); //comment this line to disable wireframes on the cube (wireframes are good for debugging)
		}
		return true;
	}
};
int main() {
	ASCIIEngine engine;
	if (engine.ConstructConsole(256, 240, 4, 4)) {
		engine.Start();
	}
	return 0;
}