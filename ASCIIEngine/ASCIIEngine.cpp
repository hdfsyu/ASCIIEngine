#include "olcConsoleGameEngine.h"
#include <fstream>
#include <strstream>
#include <algorithm>
using namespace std;
struct vec3d {
	float x = 0;
	float y = 0;
	float z = 0;
	float w = 1;
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
	vec3d mmv(mat4x4 &m, vec3d &i) {
		vec3d v;
		v.x = i.x * m.m[0][0] + i.y * m.m[1][0] + i.z * m.m[2][0] + i.w * m.m[3][0];
		v.y = i.x * m.m[0][1] + i.y * m.m[1][1] + i.z * m.m[2][1] + i.w * m.m[3][1];
		v.z = i.x * m.m[0][2] + i.y * m.m[1][2] + i.z * m.m[2][2] + i.w * m.m[3][2];
		v.w = i.x * m.m[0][3] + i.y * m.m[1][3] + i.z * m.m[2][3] + i.w * m.m[3][3];
		return v;
	}//multiply matrix vector
	mat4x4 mmi()
	{
		mat4x4 matrix;
		matrix.m[0][0] = 1.0f;
		matrix.m[1][1] = 1.0f;
		matrix.m[2][2] = 1.0f;
		matrix.m[3][3] = 1.0f;
		return matrix;
	}// make matrix identity

	mat4x4 mmx(float ar)
	{
		mat4x4 matrix;
		matrix.m[0][0] = 1.0f;
		matrix.m[1][1] = cosf(ar);
		matrix.m[1][2] = sinf(ar);
		matrix.m[2][1] = -sinf(ar);
		matrix.m[2][2] = cosf(ar);
		matrix.m[3][3] = 1.0f;
		return matrix;
	}//implements MMX TECHNOLOGY! im just kdding, make rotation matrix x pos

	mat4x4 mmy(float ar)
	{
		mat4x4 matrix;
		matrix.m[0][0] = cosf(ar);
		matrix.m[0][2] = sinf(ar);
		matrix.m[2][0] = -sinf(ar);
		matrix.m[1][1] = 1.0f;
		matrix.m[2][2] = cosf(ar);
		matrix.m[3][3] = 1.0f;
		return matrix;
	}//make rotation matrix y pos

	mat4x4 mmz(float ar)
	{
		mat4x4 matrix;
		matrix.m[0][0] = cosf(ar);
		matrix.m[0][1] = sinf(ar);
		matrix.m[1][0] = -sinf(ar);
		matrix.m[1][1] = cosf(ar);
		matrix.m[2][2] = 1.0f;
		matrix.m[3][3] = 1.0f;
		return matrix;
	}// make rotation matrix z pos

	mat4x4 mmt(float x, float y, float z)
	{
		mat4x4 matrix;
		matrix.m[0][0] = 1.0f;
		matrix.m[1][1] = 1.0f;
		matrix.m[2][2] = 1.0f;
		matrix.m[3][3] = 1.0f;
		matrix.m[3][0] = x;
		matrix.m[3][1] = y;
		matrix.m[3][2] = z;
		return matrix;
	}// make translation matrix

	mat4x4 mmp(float fFovDegrees, float fAspectRatio, float fNear, float fFar)
	{
		float fFovRad = 1.0f / tanf(fFovDegrees * 0.5f / 180.0f * 3.14159f);
		mat4x4 matrix;
		matrix.m[0][0] = fAspectRatio * fFovRad;
		matrix.m[1][1] = fFovRad;
		matrix.m[2][2] = fFar / (fFar - fNear);
		matrix.m[3][2] = (-fFar * fNear) / (fFar - fNear);
		matrix.m[2][3] = 1.0f;
		matrix.m[3][3] = 0.0f;
		return matrix;
	}//make projection matrix

	mat4x4 mmm(mat4x4 &m1, mat4x4 &m2)
	{
		mat4x4 matrix;
		for (int c = 0; c < 4; c++)
			for (int r = 0; r < 4; r++)
				matrix.m[r][c] = m1.m[r][0] * m2.m[0][c] + m1.m[r][1] * m2.m[1][c] + m1.m[r][2] * m2.m[2][c] + m1.m[r][3] * m2.m[3][c];
		return matrix;
	}//make matrix multiplication
	vec3d vadd(vec3d &v1, vec3d &v2) {
		return{ v1.x + v2.x, v1.y + v2.y, v1.z + v2.z };
	}//add
	vec3d vsub(vec3d &v1, vec3d &v2) {
		return{ v1.x - v2.x, v1.y - v2.y, v1.z - v2.z };
	}//subtract
	vec3d vmul(vec3d &v1, float k) {
		return{ v1.x * k, v1.y * k, v1.z * k };
	}//multiply
	vec3d vdiv(vec3d &v1, float k) {
		return{ v1.x / k, v1.y / k, v1.z / k };
	}//divide
	float vdp(vec3d &v1, vec3d &v2) {
		return v1.x*v2.x + v1.y*v2.y + v1.z * v2.z;
	}//dot product
	float vlength(vec3d &v) {
		return sqrtf(vdp(v, v));
	}// vector length
	vec3d vn(vec3d &v) {
		float l = vlength(v);
		return{ v.x / l,v.y / l,v.z / l };
	}//normalise
	vec3d vcp(vec3d &v1, vec3d &v2) {
		vec3d v;
		v.x = v1.y * v2.z - v1.z * v2.y;
		v.y = v1.z * v2.x - v1.x * v2.z;
		v.z = v1.x * v2.y - v1.y * v2.x;
		return v;
	}
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
		meshCube.lobj("objs/teapot.obj"); // change this to whatever obj you want
		// list of objects:
		/*
		objs/sanik.obj
		objs/ball.obj
		objs/cube.obj
		objs/telephone.obj
		objs/teapot.obj
		*/
		// TODO: maybe add more objects
		matp = mmp(90.0f, (float)sh() / (float)sw(), 0.1f, 1000.0f);
		return true;
	}
	bool oup(float fet) override{
		Fill(0, 0, sw(), sh(), PIXEL_SOLID, FG_BLACK);
		mat4x4 matRotZ, matRotX;
		t += 1.0f * fet;
		matRotZ = mmz(t*0.5f);
		matRotX = mmx(t);
		mat4x4 matTrans;
		matTrans = mmt(0.0f, 0.0f, 16.0f);
		mat4x4 world;
		world = mmi();
		world = mmm(matRotZ, matRotX);
		world = mmm(world, matTrans);
		vector<triangle> ttr; //triangles to raster
		for (auto tri : meshCube.tris) {
			triangle trip, trit; //triangle projected and triangle transformed
			trit.p[0] = mmv(world, tri.p[0]);
			trit.p[1] = mmv(world, tri.p[1]);
			trit.p[2] = mmv(world, tri.p[2]);
			vec3d normal, line1, line2;
			line1 = vsub(trit.p[1], trit.p[0]);
			line2 = vsub(trit.p[2], trit.p[0]);
			normal = vcp(line1, line2);
			normal = vn(normal);
			vec3d vcr = vsub(trit.p[0], vc); // adds vcr capabilities! im just kidding, this is the camera ray
			if (vdp(normal, vcr) < 0.0f){
				vec3d lightd = { 0.0f, 0.0f, -1.0f }; //light direction
				lightd = vn(lightd);
				float dp = max(0.5f, vdp(lightd, normal));
				CHAR_INFO c = GetColour(dp);
				trit.col = c.Attributes;
				trit.sym = c.Char.UnicodeChar;
				trip.p[0] = mmv(matp, trit.p[0]);
				trip.p[1] = mmv(matp, trit.p[1]);
				trip.p[2] = mmv(matp, trit.p[2]);
				trip.col = trit.col;
				trip.sym = trit.sym;
				trip.p[0] = vdiv(trip.p[0], trip.p[0].w);
				trip.p[1] = vdiv(trip.p[1], trip.p[1].w);
				trip.p[2] = vdiv(trip.p[2], trip.p[2].w);
				vec3d ov = { 1,1,0 }; //offset view
				trip.p[0] = vadd(trip.p[0], ov);
				trip.p[1] = vadd(trip.p[1], ov);
				trip.p[2] = vadd(trip.p[2], ov);
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
			//DrawTriangle(trip.p[0].x, trip.p[0].y, trip.p[1].x, trip.p[1].y, trip.p[2].x, trip.p[2].y, PIXEL_SOLID, FG_YELLOW); //comment this line to disable wireframes on the cube (wireframes are good for debugging)
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