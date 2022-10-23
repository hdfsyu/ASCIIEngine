#include "olcConsoleGameEngine.h"
using namespace std;
struct vec3d {
	float x, y, z;
};
struct triangle {
	vec3d p[3];
};
struct mesh {
	vector<triangle> tris;
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
public:
	bool ouc() override {
		meshCube.tris = {
		// south
		{ 0.0f, 0.0f, 0.0f,    0.0f, 1.0f, 0.0f,    1.0f, 1.0f, 0.0f },
		{ 0.0f, 0.0f, 0.0f,    1.0f, 1.0f, 0.0f,    1.0f, 0.0f, 0.0f },
		// east
		{ 1.0f, 0.0f, 0.0f,    1.0f, 1.0f, 0.0f,    1.0f, 1.0f, 1.0f },
		{ 1.0f, 0.0f, 0.0f,    1.0f, 1.0f, 1.0f,    1.0f, 0.0f, 1.0f },
		// north
		{ 1.0f, 0.0f, 1.0f,    1.0f, 1.0f, 1.0f,    0.0f, 1.0f, 1.0f },
		{ 1.0f, 0.0f, 1.0f,    0.0f, 1.0f, 1.0f,    0.0f, 0.0f, 1.0f },
		// west
		{ 0.0f, 0.0f, 1.0f,    0.0f, 1.0f, 1.0f,    0.0f, 1.0f, 0.0f },
		{ 0.0f, 0.0f, 1.0f,    0.0f, 1.0f, 0.0f,    0.0f, 0.0f, 0.0f },
		// top
		{ 0.0f, 1.0f, 0.0f,    0.0f, 1.0f, 1.0f,    1.0f, 1.0f, 1.0f },
		{ 0.0f, 1.0f, 0.0f,    1.0f, 1.0f, 1.0f,    1.0f, 1.0f, 0.0f },
		// bottom
		{ 1.0f, 0.0f, 1.0f,    0.0f, 0.0f, 1.0f,    0.0f, 0.0f, 0.0f },
		{ 1.0f, 0.0f, 1.0f,    0.0f, 0.0f, 0.0f,    1.0f, 0.0f, 0.0f },
		};
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
		for (auto tri : meshCube.tris) {
			triangle trip, trit, triz, trizx; //triangle projected, triangle translated, triangle rotated Z and triangle rotated ZX
			mmv(tri.p[0], triz.p[0], matRotZ);
			mmv(tri.p[1], triz.p[1], matRotZ);
			mmv(tri.p[2], triz.p[2], matRotZ);
			mmv(triz.p[0], trizx.p[0], matRotX);
			mmv(triz.p[1], trizx.p[1], matRotX);
			mmv(triz.p[2], trizx.p[2], matRotX);
			trit = trizx;
			trit.p[0].z = trizx.p[0].z + 3.0f;
			trit.p[1].z = trizx.p[1].z + 3.0f;
			trit.p[2].z = trizx.p[2].z + 3.0f;
			mmv(trit.p[0], trip.p[0], matp);
			mmv(trit.p[1], trip.p[1], matp);
			mmv(trit.p[2], trip.p[2], matp);
			trip.p[0].x += 1.0f; trip.p[0].y += 1.0f;
			trip.p[1].x += 1.0f; trip.p[1].y += 1.0f;
			trip.p[2].x += 1.0f; trip.p[2].y += 1.0f;
			trip.p[0].x *= 0.5f * (float)sw();
			trip.p[0].y *= 0.5f * (float)sh();
			trip.p[1].x *= 0.5f * (float)sw();
			trip.p[1].y *= 0.5f * (float)sh();
			trip.p[2].x *= 0.5f * (float)sw();
			trip.p[2].y *= 0.5f * (float)sh();
			DrawTriangle(trip.p[0].x, trip.p[0].y,
				trip.p[1].x, trip.p[1].y,
				trip.p[2].x, trip.p[2].y,
				PIXEL_SOLID, FG_WHITE);
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