#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
#include <fstream>
#include <strstream>
#include <algorithm>
struct vec3d {
	float x = 0;
	float y = 0;
	float z = 0;
	float w = 1;
};
struct triangle{
	vec3d p[3];
	olc::Pixel col;
};
struct mesh {
	std::vector<triangle> tris;
	bool lobj(std::string filename) { // load object file 
		std::ifstream f(filename);
		if (!f.is_open()) {
			return false;
		}
		std::vector<vec3d> verts;
		while (!f.eof())
		{
			char line[128];
			f.getline(line, 128);
			std::strstream s;
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
class ASCIIEngine : public olc::PixelGameEngine {
public:
	ASCIIEngine() {
		sAppName = "3D Engine";
	}
private:
	mesh meshCube;
	mat4x4 matp; // mat projection
	vec3d vc; // camera
	vec3d ld; //look direction
	float t; // theta
	float fy;//camera rotation in XZ plane
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
		float fr = 1.0f / tanf(fFovDegrees * 0.5f / 180.0f * 3.14159f);
		mat4x4 matrix;
		matrix.m[0][0] = fAspectRatio * fr;
		matrix.m[1][1] = fr;
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
	mat4x4 mpa(vec3d &pos, vec3d &target, vec3d &up) {
		vec3d nf = vsub(target, pos);// new forward direction
		nf = vn(nf);
		vec3d a = vmul(nf, vdp(up, nf));
		vec3d np = vsub(up, a); // new up direction
		np = vn(np);
		vec3d nr = vcp(np, nf); //new right direction
		mat4x4 matrix;
		matrix.m[0][0] = nr.x;	matrix.m[0][1] = nr.y;	matrix.m[0][2] = nr.z;	matrix.m[0][3] = 0.0f;
		matrix.m[1][0] = np.x;		matrix.m[1][1] = np.y;		matrix.m[1][2] = np.z;		matrix.m[1][3] = 0.0f;
		matrix.m[2][0] = nf.x;	matrix.m[2][1] = nf.y;	matrix.m[2][2] = nf.z;	matrix.m[2][3] = 0.0f;
		matrix.m[3][0] = pos.x;			matrix.m[3][1] = pos.y;			matrix.m[3][2] = pos.z;			matrix.m[3][3] = 1.0f;
		return matrix;
	}//matrix point at
	mat4x4 mqi(mat4x4 &m)
	{
		mat4x4 matrix;
		matrix.m[0][0] = m.m[0][0]; matrix.m[0][1] = m.m[1][0]; matrix.m[0][2] = m.m[2][0]; matrix.m[0][3] = 0.0f;
		matrix.m[1][0] = m.m[0][1]; matrix.m[1][1] = m.m[1][1]; matrix.m[1][2] = m.m[2][1]; matrix.m[1][3] = 0.0f;
		matrix.m[2][0] = m.m[0][2]; matrix.m[2][1] = m.m[1][2]; matrix.m[2][2] = m.m[2][2]; matrix.m[2][3] = 0.0f;
		matrix.m[3][0] = -(m.m[3][0] * matrix.m[0][0] + m.m[3][1] * matrix.m[1][0] + m.m[3][2] * matrix.m[2][0]);
		matrix.m[3][1] = -(m.m[3][0] * matrix.m[0][1] + m.m[3][1] * matrix.m[1][1] + m.m[3][2] * matrix.m[2][1]);
		matrix.m[3][2] = -(m.m[3][0] * matrix.m[0][2] + m.m[3][1] * matrix.m[1][2] + m.m[3][2] * matrix.m[2][2]);
		matrix.m[3][3] = 1.0f;
		return matrix;
	}//matrix quick inverse
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
	vec3d vip(vec3d &plane_p, vec3d &plane_n, vec3d &lineStart, vec3d &lineEnd)
	{
		plane_n = vn(plane_n);
		float plane_d = -vdp(plane_n, plane_p);
		float ad = vdp(lineStart, plane_n);
		float bd = vdp(lineEnd, plane_n);
		float t = (-plane_d - ad) / (bd - ad);
		vec3d lineStartToEnd = vsub(lineEnd, lineStart);
		vec3d lineToIntersect = vmul(lineStartToEnd, t);
		return vadd(lineStart, lineToIntersect);
	}//adds vip rank to the game! jk, vector intersect plane
	int tcap(vec3d plane_p, vec3d plane_n, triangle &in_tri, triangle &out_tri1, triangle &out_tri2) {
		auto dist = [&](vec3d &p) {
			plane_n = vn(plane_n);
			return (plane_n.x*p.x + plane_n.y*p.y + plane_n.z*p.z - vdp(plane_n, plane_p));
		};
		vec3d* inside_points[3]; int ipc = 0; //inside point count
		vec3d* outside_points[3]; int opc = 0;//outside point count
		float d0 = dist(in_tri.p[0]);
		float d1 = dist(in_tri.p[1]);
		float d2 = dist(in_tri.p[2]);
		if (d0 >= 0) { inside_points[ipc++] = &in_tri.p[0]; }
		else { outside_points[opc++] = &in_tri.p[0]; }
		if (d1 >= 0) { inside_points[ipc++] = &in_tri.p[1]; }
		else { outside_points[opc++] = &in_tri.p[1]; }
		if (d2 >= 0) { inside_points[ipc++] = &in_tri.p[2]; }
		else { outside_points[opc++] = &in_tri.p[2]; }
		if (ipc == 0) {
			return 0;
		}
		if (ipc == 3) {
			out_tri1 = in_tri;
			return 1;
		}
		if (ipc == 1 && opc == 2)
		{
			out_tri1.col = in_tri.col;
			out_tri1.p[0] = *inside_points[0];
			out_tri1.p[1] = vip(plane_p, plane_n, *inside_points[0], *outside_points[0]);
			out_tri1.p[2] = vip(plane_p, plane_n, *inside_points[0], *outside_points[1]);
			return 1;
		}

		if (ipc == 2 && opc == 1)
		{
			out_tri1.col = in_tri.col;
			out_tri2.col = in_tri.col;
			out_tri1.p[0] = *inside_points[0];
			out_tri1.p[1] = *inside_points[1];
			out_tri1.p[2] = vip(plane_p, plane_n, *inside_points[0], *outside_points[0]);
			out_tri2.p[0] = *inside_points[1];
			out_tri2.p[1] = out_tri1.p[2];
			out_tri2.p[2] = vip(plane_p, plane_n, *inside_points[1], *outside_points[0]);
			return 2;
		}
	}
	olc::Pixel GetColour(float lum) {
		int nValue = (int)(std::max(lum, 0.20f) * 255.0f);
		return olc::Pixel(nValue, nValue, nValue);
	}
public:
	bool ouc() override {
		meshCube.lobj("objs/mountains.obj"); // change this to whatever obj you want
		// list of objects:
		/*
		objs/sanik.obj
		objs/ball.obj
		objs/cube.obj
		objs/telephone.obj
		objs/teapot.obj
		objs/axis.obj
		objs/mountains.obj
		*/
		matp = mmp(90.0f, (float)sh() / (float)sw(), 0.1f, 1000.0f);
		return true;
	}
	bool oup(float fet) override{
		if (GetKey(olc::Key::UP).bHeld) {
			vc.y += 8.0f * fet;
		}
		if (GetKey(olc::Key::DOWN).bHeld) {
			vc.y -= 8.0f * fet;
		}
		if (GetKey(olc::Key::LEFT).bHeld) {
			vc.x += 8.0f*fet;
		}
		if (GetKey(olc::Key::RIGHT).bHeld) {
			vc.x -= 8.0f*fet;
		}
		vec3d vf = vmul(ld, 8.0f*fet);
		if (GetKey(olc::Key::A).bHeld)
			fy -= 2.0f * fet;
		if (GetKey(olc::Key::D).bHeld)
			fy += 2.0f * fet;
		if (GetKey(olc::Key::W).bHeld)
			vc = vadd(vc, vf);
		if (GetKey(olc::Key::S).bHeld)
			vc = vsub(vc, vf);
		mat4x4 matRotZ, matRotX;
		//t += 1.0f * fet; // i commented this for trying to not make you throw up and become dizzy! you're welcome!
		matRotZ = mmz(t*0.5f);
		matRotX = mmx(t);
		mat4x4 matTrans;
		matTrans = mmt(0.0f, 0.0f, 16.0f);
		mat4x4 world;
		world = mmi();
		world = mmm(matRotZ, matRotX);
		world = mmm(world, matTrans);
		vec3d vup = { 0,1,0 };
		vec3d vtarget = { 0,0,1 };
		mat4x4 mcr = mmy(fy);// mat camera rotation
		ld = mmv(mcr, vtarget);
		vtarget = vadd(vc, ld);
		mat4x4 mc = mpa(vc, vtarget, vup);//mat camera
		mat4x4 mv = mqi(mc);//mat view
		std::vector<triangle> ttr; //triangles to raster
		for (auto tri : meshCube.tris) {
			triangle trip, trit, triv; //triangle projected, triangle transformed and triangle viewed
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
				vec3d lightd = { 0.0f, 1.0f, -1.0f };
				lightd = vn(lightd);
				float dp = std::max(0.1f, vdp(lightd, normal));
				trit.col = GetColour(dp);
				triv.p[0] = mmv(mv, trit.p[0]);
				triv.p[1] = mmv(mv, trit.p[1]);
				triv.p[2] = mmv(mv, trit.p[2]);
				triv.col = trit.col;
				int ct = 0;
				triangle clipped[2];
				ct = tcap({ 0.0f, 0.0f, 0.1f }, { 0.0f, 0.0f, 1.0f }, triv, clipped[0], clipped[1]);
				for (int n = 0; n < ct; n++)
				{
					trip.p[0] = mmv(matp, clipped[n].p[0]);
					trip.p[1] = mmv(matp, clipped[n].p[1]);
					trip.p[2] = mmv(matp, clipped[n].p[2]);
					trip.col = clipped[n].col;
					trip.p[0] = vdiv(trip.p[0], trip.p[0].w);
					trip.p[1] = vdiv(trip.p[1], trip.p[1].w);
					trip.p[2] = vdiv(trip.p[2], trip.p[2].w);
					trip.p[0].x *= -1.0f;
					trip.p[1].x *= -1.0f;
					trip.p[2].x *= -1.0f;
					trip.p[0].y *= -1.0f;
					trip.p[1].y *= -1.0f;
					trip.p[2].y *= -1.0f;
					vec3d ov = { 1,1,0 };
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
		}
		std::sort(ttr.begin(), ttr.end(), [](triangle &t1, triangle &t2){
			float z1 = (t1.p[0].z + t1.p[1].z + t1.p[2].z) / 3.0f;
			float z2 = (t2.p[0].z + t2.p[1].z + t2.p[2].z) / 3.0f;
			return z1 > z2;
		});
		FillRect(0, 0, sw(), sh(), olc::BLACK);
		for (auto &ttor : ttr) {
			triangle clipped[2];
			std::list<triangle> lt;//list triangles
			lt.push_back(ttor);
			int nt = 1;//new triangles
			for (int p = 0; p < 4; p++)
			{
				int tta = 0;//tris to add
				while (nt > 0)
				{
					triangle test = lt.front();
					lt.pop_front();
					nt--;
					switch (p)
					{
					case 0:	tta = tcap({ 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, test, clipped[0], clipped[1]); break;
					case 1:	tta = tcap({ 0.0f, (float)sh() - 1, 0.0f }, { 0.0f, -1.0f, 0.0f }, test, clipped[0], clipped[1]); break;
					case 2:	tta = tcap({ 0.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, test, clipped[0], clipped[1]); break;
					case 3:	tta = tcap({ (float)sw() - 1, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }, test, clipped[0], clipped[1]); break;
					}
					for (int w = 0; w < tta; w++)
						lt.push_back(clipped[w]);
				}
				nt = lt.size();
			}
			for (auto &t : lt)
			{
				FillTriangle(t.p[0].x, t.p[0].y, t.p[1].x, t.p[1].y, t.p[2].x, t.p[2].y, t.col);
				//DrawTriangle(t.p[0].x, t.p[0].y, t.p[1].x, t.p[1].y, t.p[2].x, t.p[2].y, olc::YELLOW); //uncomment this line to enable wireframes (good for debugging)
			}
		}
		return true;
	}
};
int main() {
	ASCIIEngine engine;
	if (engine.Construct(720, 480, 2, 2)) {
		engine.Start();
	}
	return 0;
}