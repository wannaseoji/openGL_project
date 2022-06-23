#include <GL/glut.h>
#include<math.h>
#include<vector>
#include<stdlib.h>
#include<stdio.h>
//1871124 ������
/*
	��ũ�� �μŶ�!
	�������� ��ź�� ������ ��ũ�� �¼� ī�캸�̰� �Ǿ��ּ���.
*/
using namespace std;

int MX = 300, MY = 300;
float time = 0;
float x = 0, y = 0; //�÷��̾� ��ǥ ��������
float middle_x = 0, middle_y = 0, middle_z = -30; // �߾Ӽ��� ���� ��ǥ
float health_player = 1000; //ü��
float health_enemy = 10000; //ü��
float t = 0; //���� ������ �������� ��������
float enemy_oldx, enemy_oldy;
float p_x = 0, p_y = 8;

class Point3 { //����Ŭ����
public:
	float x, y, z;//��ǥ
	void set(float dx, float dy, float dz) { x = dx; y = dy; z = dz; }//������ǥ����޼ҵ�
	void set(Point3& p) { x = p.x; y = p.y; z = p.z; }//������ǥ����޼ҵ�2 �ٸ����� ����
	Point3(float xx, float yy, float zz) { x = xx; y = yy; z = zz; }//������: �����ʱ�ȭ
	Point3() { x = y = z = 0; }//������: �ʱ�ȭ������ ����
};
class Vector3 {
public:
	float x, y, z;//���ͼ���
	void set(float dx, float dy, float dz) { x = dx; y = dy; z = dz; }//���ͻ����޼ҵ�
	//void set(Vector3& v){ x = v.x; y = v.y; z = v.z;}//���ͻ����޼ҵ�
	void set(Vector3 v) { x = v.x; y = v.y; z = v.z; }//���ͻ����޼ҵ�
	void flip() { x = -x; y = -y; z = -z; } //�ݴ���⺤��
	void setDiff(Point3& a, Point3& b)//�κ�������
	{
		x = a.x - b.x; y = a.y - b.y; z = a.z - b.z;
	}
	void normalize();//�������ͷ�
	Vector3(float xx, float yy, float zz) { x = xx; y = yy; z = zz; } //������
	//Vector3(const Vector3& v) {x = v.x; y = v.y; z = v.z;} // ���������
	Vector3() { x = y = z = 0.0; }
	Vector3 cross(Vector3 b);//����
	float dot(Vector3 b);//����
};
class Camera {
public:
	Point3 eye;//������ǥ�����
	Vector3 u, v, n;//������ǥ�踦 �����ϴ� ������������
	float aspect, nearDist, farDist, viewAngle;//gluPerspective�� �Ķ���͵� ������ȯ�� �̿�
	void setModelViewMatrix();// ������ȯ�� ���� �𵨺��������
	Camera(void); // ������

	// ����ùķ������� ȸ�� ��ȯ�Լ��� roll pitch yaw
	void roll(float angle);
	void pitch(float angle);
	void yaw(float angle);
	// �̵���ȯ
	void slide(float du, float dv, float dn);

	//ī�޶���ġ, ����,ī�޶������ ����
	void set(Point3 Eye, Point3 look, Vector3 up); // ���ͷ�
	void set(float eyeX, float eyeY, float eyeZ, float lookX, float lookY, float lookZ, float upX, float upY, float upZ); //������

	void setShape(float viewAngle, float aspect, float Near, float Far); //ȭ������
	void setAspect(float asp); // ��Ⱦ������
	void getShape(float& viewAngle, float& aspect, float& Near, float& Far); //ȭ�鱸��������
	void rotAxes(Vector3& a, Vector3& b, float angle);//������ǥ����ȸ��
	void setDefaultCamera();// ī�޶��ʱ⼳���Լ�
};

Camera::Camera(void) {
	setDefaultCamera();//������, �ʱ�ȭ�Լ� ���� ȣ��
}
void Camera::setDefaultCamera(void) {// �ʱ�ȭ�Լ�
	setShape(45.0f, 640 / (float)480, 0.1f, 200.0f);//ȭ�鱸���� ���� fov,aspect,near clip,far clip
	Point3 eyePoint = Point3(10.0, 0.0, 0.0); // ī�޶���ġ ����
	Point3 lookPoint = Point3(0.0, 0.0, 0.0); // �ٶ󺸴� ���� ����
	Vector3 upVector(0, 1, 0); // ī�޶��� ������ ����
	set(eyePoint, lookPoint, upVector);
}
void Camera::set(float eyeX, float eyeY, float eyeZ, float lookX, float lookY, float lookZ, float upX, float upY, float upZ) {
	Point3 Eye = Point3(eyeX, eyeY, eyeZ); //ī�޶�, ����, ������ ����
	Point3 look = Point3(lookX, lookY, lookZ);
	Vector3 up(upX, upY, upZ);
	eye.set(Eye);//ī�޶���ġ����
	n.set(eye.x - look.x, eye.y - look.y, eye.z - look.z);//������ ī�޶���ǥ�� ���� ����(optical axis)����
	u.set(up.cross(n)); // ī�޶� �����Ϳ��� �������� u����
	v.set(n.cross(u)); // u�� n�� �������� ������ǥ���� y��
	u.normalize();v.normalize();n.normalize();// �� ������ǥ�� ����� ����ȭ �̷ν� ī�޶� �ð����� �������������� �ϼ���
	setModelViewMatrix();// ���� ���ǵ� ������ǥ�踦 �𵨺���Ŀ� ����
}
void Camera::set(Point3 Eye, Point3 look, Vector3 up) { //���� set�Լ��� �������̵� ��������̴�.
	eye.set(Eye);
	n.set(eye.x - look.x, eye.y - look.y, eye.z - look.z);
	u.set(up.cross(n));
	v.set(n.cross(u));
	u.normalize();v.normalize();n.normalize();
	setModelViewMatrix();
}
void Camera::setModelViewMatrix(void) {
	float m[16];// �𵨺���Ŀ� ������� ��İ� ������ǥ�� ��ȯ��ķμ� 4x4=16���� ���Ҹ� ����
	Vector3 eVec(eye.x, eye.y, eye.z);//ī�޶���ġ
	// �𵨺������ ȸ����ȯ�� �̵���ȯ����
	m[0] = u.x;	m[4] = u.y;	m[8] = u.z;	m[12] = -eVec.dot(u);
	m[1] = v.x;	m[5] = v.y;	m[9] = v.z;	m[13] = -eVec.dot(v);
	m[2] = n.x;	m[6] = n.y;	m[10] = n.z;	m[14] = -eVec.dot(n);
	m[3] = 0;		m[7] = 0;		m[11] = 0;	m[15] = 1.0;
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(m);//���m�� �𵨺���Ŀ� �־��ش�
}
void Camera::setShape(float vAngle, float asp, float nr, float fr) {//������ �����Ѵ�.
	viewAngle = vAngle;//�þ߰�
	aspect = asp;//��Ⱦ��
	nearDist = nr;//���������
	farDist = fr;//�Ĺ������
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();// ��������� ���������ϱ����� �������
	gluPerspective(viewAngle, aspect, nearDist, farDist);//������������
	glMatrixMode(GL_MODELVIEW);
}
void Camera::setAspect(float asp) {
	aspect = asp;// ��Ⱦ�񸸻�������
}
void Camera::getShape(float& vAngle, float& asp, float& nr, float& fr) {
	vAngle = viewAngle;//���������� ������ ��ȯ
	asp = aspect;
	nr = nearDist;
	fr = farDist;
}
void Camera::slide(float du, float dv, float dn) {//�̵���ȯ
	// ��ü ��ǥ�� �������� �����̵� �ϴ� ���� �ڵ�� ���� ����
	//eye.x += du;	eye.y += dv;	eye.z += dn;

	eye.x += du * u.x + dv * v.x + dn * n.x;//������ǥ���� �������������� �̵���ȯ����� ���Ѵ� ��ȯ�� ����� �𵨺���Ŀ� ����
	eye.y += du * u.y + dv * v.y + dn * n.y;//������ǥ�谡 �̵��� ȿ�� // Emmanuel Agu�ڷ�����
	eye.z += du * u.z + dv * v.z + dn * n.z;
	setModelViewMatrix();
}
//������ǥ�踦 �̷�� ������ ��￩ ȸ����ȯ // Emmanuel Agu�ڷ� ����
// �ؼ�:
// 2���� ����� �� �� x, y �࿡ �ش��ϴ� �������� a, b ���͸� �̸� angle ��ŭ ȸ������
// a' = a cos(angle) + b sin(angle) ; 2���� ��鿡�� �������� �׸��� �ڸ���
// b' = -a sin(angle) + b cos(angle) ; a'�� b'�� �������� �����ϸ� �ڸ���
// ���� a, b �� ���͸� �������� ������ �� ���� ���ͷ� Ȯ���ϸ�, �������� Ȯ���
void Camera::rotAxes(Vector3& a, Vector3& b, float angle) {

	float ang = 3.14159265f / 180 * angle;//���� ���ȴ����� ��ȯ�Ѵ�. cos, sin �Լ��� �̿��ϱ� ���ؼ�
	float C = cosf(ang), S = sinf(ang); // ��ȯ�Ϸ��� ���� cos, sin �� ����

	// �̸� �̿��� �� ���� angle��ŭ ȸ���Ѵ�
	Vector3 t(C * a.x + S * b.x, C * a.y + S * b.y, C * a.z + S * b.z);
	b.set(-S * a.x + C * b.x, -S * a.y + C * b.y, -S * a.z + C * b.z);
	a.set(t.x, t.y, t.z);
}
void Camera::roll(float angle) {
	rotAxes(u, v, angle);//n���߽��� ȸ�� ������ u,v���� angle������ŭ ȸ���Ѵ�
	setModelViewMatrix();//�𵨺���Ŀ� ����
}
void Camera::pitch(float angle) {
	rotAxes(n, v, angle);////u���߽��� ȸ�� ������ n,v���� angle������ŭ ȸ���Ѵ�
	setModelViewMatrix();//�𵨺���Ŀ� ����
}
void Camera::yaw(float angle) {
	rotAxes(u, n, angle);//v���߽��� ȸ�� ������ u,n���� angle������ŭ ȸ���Ѵ�
	setModelViewMatrix();//�𵨺���Ŀ� ����
}
//�޼ҵ�ȣ�⺤�Ϳ� �Ķ���ͺ����� �������͸� ��ȯ�ϴ� �Լ�
Vector3 Vector3::cross(Vector3 b) {
	return Vector3(y * b.z - z * b.y, z * b.x - x * b.z, x * b.y - y * b.x);
}
float Vector3::dot(Vector3 b) { return x * b.x + y * b.y + z * b.z; }//�� ������ ����
void Vector3::normalize() {//�ش纤�͸� ����ȭ�ϴ� �Լ�
	double sizeSq = x * x + y * y + z * z;// ũ�⸦ ���ϰ�
	if (sizeSq < 0.0000001) {
		//cerr << "\nnormalize() sees vector (0,0,0)!";
		return;// does nothing to zero vectors;
	}
	float scaleFactor = (float)(1.0 / sqrt(sizeSq));// ũ��� �����ش�
	x *= scaleFactor;y *= scaleFactor;z *= scaleFactor;// ������ ������ ������ �����Ͽ� ����ȭ
}
Camera cam;
class vec3 {
public:
	float m[3];
	vec3(float x = 0, float y = 0, float z = 0) {
		m[0] = x; m[1] = y; m[2] = z;
	}
	vec3 operator-(vec3 x) {
		return vec3(m[0] - x.m[0],
			m[1] - x.m[1],
			m[2] - x.m[2]);
	}
	vec3 operator+(vec3 x) {
		return vec3(m[0] + x.m[0],
			m[1] + x.m[1],
			m[2] + x.m[2]);
	}
	vec3 operator*(float x) {
		return vec3(m[0] * x, m[1] * x, m[2] * x);
	}
	float Length() {
		return sqrtf(m[0] * m[0] + m[1] * m[1] + m[2] * m[2]);
	}

};
class Stone {
public:
	vec3 v;
	vec3 pos;
};
vector<Stone> stones;

class Boom {
public:
	vec3 v;
	vec3 pos;
};
vector<Boom> booms;

class MiddleLine {
public:
	vec3 v;
	vec3 pos;

};
vector<MiddleLine> middleLines;



void MyMouseMove(GLint X, GLint Y) {
	MX = X;
	MY = Y;
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(MX / 300.0, MY / 300.0, 1.0,
		0.0, 0.0, 0.0,
		0.0, 1.0, 0.0);
	glutPostRedisplay();
}

void MyTimer(int n)
{
	time += 1.0;

	if (int(time) % 250 == 0) {
		Boom bm;
		bm.pos.m[0] = sin(time * 0.001) * 10;
		bm.pos.m[1] = 6;
		bm.pos.m[2] = -7;
		bm.v.m[0] = (p_x - 0.5) / 5.;
		bm.v.m[1] = 0;
		bm.v.m[2] = 2;
		booms.push_back(bm);
	}
	glutPostRedisplay();
	glutTimerFunc(1, MyTimer, 1);
	glutPostRedisplay();

}
void MyTimer2(int n) //2�ʸ��� �߾Ӽ��� �ϳ� ����
{


	MiddleLine ml;
	ml.pos.m[0] = middle_x;
	ml.pos.m[1] = middle_y;
	ml.pos.m[2] = middle_z;

	ml.v.m[0] = 0;
	ml.v.m[1] = 0;
	ml.v.m[2] = -1.5;
	middleLines.push_back(ml);
	glutPostRedisplay();

	glutTimerFunc(2000, MyTimer2, 1);
	glutPostRedisplay();

}

void MySpecial(int key, int X, int Y) {
	if (key == GLUT_KEY_UP) {
		y += 0.5f;
		p_y += 0.5f;

	}
	if (key == GLUT_KEY_DOWN) {
		y -= 0.5f;
		p_y -= 0.5f;
	}
	if (key == GLUT_KEY_LEFT) {
		x -= 0.5f;
		p_x -= 0.5f;
	}
	if (key == GLUT_KEY_RIGHT) {
		x += 0.5f;
		p_x += 0.5f;
	}
	glutPostRedisplay();

}
void MyKeyboard(unsigned char KeyPressed, int X, int Y) {
	if (KeyPressed == ' ') { //spacebar 
		Stone st;
		st.pos.m[0] = x + 1.2;
		st.pos.m[1] = 6.1;
		st.pos.m[2] = y + 8;
		st.v.m[0] = 0;
		st.v.m[1] = 0;
		st.v.m[2] = -1.5;
		stones.push_back(st);
		t = 90;
		glutPostRedisplay();
	}

}



void MyReshape(int w, int h) {
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-15.0, 15.0, -15.0, 15.0, -100.0, 100.0); // 15�� ���� view
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(MX / 300.0, MY / 300.0, 1.0,
		0.0, 0.0, 0.0,
		0.0, 1.0, 0.0
	);  //������ȯ
}
void InitLight() {
	GLfloat mat_diffuse[] = { 0.5, 0.4, 0.3, 1.0 };
	GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat mat_ambient[] = { 0.5, 0.4, 0.3, 1.0 };
	GLfloat mat_shininess[] = { 15.0 };
	GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat light_diffuse[] = { 0.8, 0.8, 0.8, 1.0 };
	GLfloat light_ambient[] = { 0.3, 0.3, 0.3, 1.0 };
	GLfloat light_position[] = { -3, 6, 3.0, 0.0 };
	glShadeModel(GL_SMOOTH);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
	glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
	glEnable(GL_NORMALIZE); //������� �������͸� ��Ȯ�ϰ� �ٽ��ض�
}
bool playerHitCheck() {
	vec3 player(p_x, 3.0, p_y);
	int size = booms.size();
	for (int i = 0; i < size; i++) {
		vec3 diff = player - booms[i].pos;
		if (diff.Length() < 3) {
			booms.erase(booms.begin() + i);
			health_player -= 250;//damage
			return true;
		}
	}
	return false;
}
bool enemy_HitCheck() { // ���� big stone�� �浹�̸� true����
	vec3 enemy(sin(time * 0.001) * 10, 3.0, -7);
	int size = stones.size();
	for (int i = 0; i < size; i++) {
		vec3 diff = enemy - stones[i].pos;
		if (diff.Length() < 4) {
			stones.erase(stones.begin() + i);
			health_enemy -= 100;
			return true;
		}
	}
	return false;
}
bool LoadCheck() {
	if (x > 12.5) {
		x = 12.5;
		return false;
	}
	if (x < -12.5) {
		x = -12.5;
		return false;
	}
	return true;
}
void UpdateBooms() {
	vec3 acc(0, -0.8, 0); // �߷°��ӵ��� ����
	float boomTime = 0.01;

	int size = booms.size();
	for (int i = 0; i < size; i++) {
		booms[i].v = booms[i].v + acc * boomTime;
		booms[i].pos = booms[i].pos + booms[i].v * boomTime;

		if (booms[i].pos.m[1] < 0.55) { // �ٴ��浹
			booms[i].pos.m[1] = 0.55;
			booms[i].v.m[1] = fabs(booms[i].v.m[1]);
			// stones[i].v = stones[i].v * 0.6;
		}
	}

	for (int i = 0; i < booms.size(); ) {
		if (booms[i].pos.m[2] > 30 || booms[i].pos.m[2] < -30)
			booms.erase(booms.begin() + i);
		//else if (stones[i].v.Length() < 0.0001)
		//	stones.erase(stones.begin() + i);
		else
			i++;
	}
}
void UpdateStoneballs() {
	vec3 acc(0, -0.8, 0); // �߷°��ӵ��� ����
	float stoneTime = 0.01;

	int size = stones.size();
	for (int i = 0; i < size; i++) {
		stones[i].v = stones[i].v + acc * stoneTime;
		stones[i].pos = stones[i].pos + stones[i].v * stoneTime;

		if (stones[i].pos.m[1] < 0.5 + 0.05) { // �ٴ��浹
			stones[i].pos.m[1] = 0.55;
			stones[i].v.m[1] = fabs(stones[i].v.m[1]);
			// stones[i].v = stones[i].v * 0.6;
		}
	}

	for (int i = 0; i < stones.size(); ) {
		if (stones[i].pos.m[2] > 30 || stones[i].pos.m[2] < -30)
			stones.erase(stones.begin() + i);
		//else if (stones[i].v.Length() < 0.0001)
		//	stones.erase(stones.begin() + i);
		else
			i++;
	}
}
void UpdateMiddleLine() {
	vec3 acc(0, -0.8, 0); // �߷°��ӵ��� ����

	int size = middleLines.size();
	//printf("size :%d\n", size);
	float middleLineTime = 0.01;
	for (int i = 0; i < size; i++) {
		middleLines[i].pos.m[2] = middleLines[i].pos.m[2] + middleLineTime;
		//printf("%f \n", middleLines[i].pos.m[2]);
	}
	for (int i = 0; i < middleLines.size(); ) {
		if (middleLines[i].pos.m[2] > 40) {
			middleLines.erase(middleLines.begin() + i);
			//middleLines.
		}
		else
			i++;
	}
}
void DrawBooms() {
	UpdateBooms();
	int size = booms.size();
	float mat_diffuse_boom[4] = { 0.01,0.01,0.1,1 }; // boom diffuse
	for (int i = 0; i < size; i++) {
		glPushMatrix();
		glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse_boom);
		glTranslatef(booms[i].pos.m[0], booms[i].pos.m[1], booms[i].pos.m[2]);
		glScalef(0.5, 0.5, 0.4);
		glutSolidSphere(1, 20, 20);
		glPopMatrix();
	}
}
void DrawStoneballs() {
	UpdateStoneballs();
	int size = stones.size();
	float mat_diffuse_stone[4] = { 0.3,0.3,0.4,1 }; // stoneball diffuse
	for (int i = 0; i < size; i++) {
		glPushMatrix();
		glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse_stone);
		glTranslatef(stones[i].pos.m[0], stones[i].pos.m[1], stones[i].pos.m[2]);
		glScalef(0.4, 0.4, 0.4);
		glutSolidDodecahedron();
		glPopMatrix();
	}
}

void DrawMiddleLine() {
	UpdateMiddleLine();
	float mat_diffuse_middleLine[4] = { 1,1,0.05,1 }; // middleLine diffuse

	int size = middleLines.size();
	for (int i = 0; i < size; i++) {


		glPushMatrix();

		glTranslatef(0, 0.2, 0);

		glPushMatrix();
		glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse_middleLine);
		glTranslatef(middleLines[i].pos.m[0], middleLines[i].pos.m[1], middleLines[i].pos.m[2]);
		glScalef(1, 0.5, 3); //�߾Ӽ�
		glutSolidCube(1.0);
		glPopMatrix();


		glPopMatrix();
	}

}
void Draw_Head() {
	glPushMatrix();
	glTranslatef(0, 10, 0);
	glutSolidSphere(1.5, 20, 20);
	glPopMatrix();
	glRotatef(sin(time * 0.01) * 15, 0, 0, 1);
	glTranslatef(0, 12, 0);
	glutSolidSphere(3, 20, 20);
}
void Draw_Body() {
	glScalef(1, 3, 1);
	glutSolidSphere(3, 20, 20);//����
}
void Draw_Arm() {
	glScalef(0.3, 1, 0.3);
	glutSolidCube(5);
}
void Draw_Cuff() {//�ȸ�
	glTranslatef(0, 4, 0);
	glutSolidCube(3);
}
void Draw_Leg() {
	glScalef(0.5, 2, 0.5);
	glutSolidSphere(3, 20, 20);
}
void Draw_Pelvis() { //���
	//glScalef(1, 1, 1);
	glutSolidSphere(1.5, 20, 20);//����
}
void DrawEnemy() {
	Vector3 a(0, 0, 1);
	Vector3 b(x - sin((time - 1) * 0.001) * 10, 0, p_y + 10);

	b.normalize();
	float mat_diffuse_wheel[4] = { 0.8,0.8,0.8,1 }; //wheel
	float mat_diffuse_middle[4] = { 34 / 255.,139 / 255.,34 / 255.,1 };
	float mat_diffuse_top[4] = { 46 / 255.,139 / 255.,87 / 255.,1 };
	float mat_diffuse_gun[4] = { 0.8,0.8,0.8,1 };
	glPushMatrix();
	glTranslatef(0, 15, 0);
	glScalef(1, 1, 1.5);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse_top);
	enemy_oldx = x;
	enemy_oldy = y;
	if (sin((time - 1) * 0.01) * 10 > 0 && x >= 0)
		glRotatef(acos(a.dot(b)) * 180 / 3.1415, 0, 1, 0);
	else if (sin((time - 1) * 0.01) * 10 > 0 && x <= 0)
		glRotatef(-acos(a.dot(b)) * 180 / 3.1415, 0, 1, 0);
	else if (sin((time - 1) * 0.01) * 10 < 0 && x >= 0)
		glRotatef(-acos(a.dot(b)) * 180 / 3.1415, 0, -1, 0);
	else if (sin((time - 1) * 0.01) * 10 < 0 && x <= 0)
		glRotatef(-acos(a.dot(b)) * 180 / 3.1415, 0, 1, 0);

	else if (sin((time - 1) * 0.01) * 10 == 0 && x == 0)
	{

	}

	//glRotatef(acos(a.dot(b)) * 180 / 3.14, 0, -1, 0);
//printf("%f ", acos(a.dot(b)));
	glutSolidCube(5);		//����
	glPushMatrix();
	glTranslatef(0, 0, 3);
	glScalef(0.5, 0.5, 1.2);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse_gun);
	glutSolidCube(3);  //�ѱ�

	glPopMatrix();
	glPopMatrix();
	glPushMatrix();
	glTranslatef(0, 7.5, 0);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse_middle);
	glutSolidCube(10); //�Ʒ�����
	glPushMatrix();
	glTranslatef(5, -5, 5);
	glRotatef(90, 0, 1, 0);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse_wheel);
	glutSolidTorus(1, 2, 20, 20); //���ʹ���1
	glTranslatef(5, 0, 0);
	glutSolidTorus(1, 2, 20, 20); //���ʹ���2
	glTranslatef(5, 0, 0);
	glutSolidTorus(1, 2, 20, 20); //���ʹ���3


	glTranslatef(0, 0, -10);
	glutSolidTorus(1, 2, 20, 20); //�����ʹ���1
	glTranslatef(-5, 0, 0);
	glutSolidTorus(1, 2, 20, 20); //�����ʹ���2
	glTranslatef(-5, 0, 0);
	glutSolidTorus(1, 2, 20, 20); //�����ʹ���3
	glPopMatrix();
	glPopMatrix();


}
void Draw_Hand() {

	glScalef(2, 1, 2);
	glutSolidSphere(1, 20, 20);
}
void DrawFoot() {
	glScalef(2, 0.5, 4);
	glutSolidSphere(1, 20, 20);
}

void DrawMan() {
	float mat_diffuse_hat[4] = { 0.7,0.3,0.7,1 }; // hat diffuse
	float mat_diffuse_arm[4] = { 255 / 255.  ,235 / 255. , 203 / 255.,1 }; // player's arm diffuse
	float mat_diffuse_hand[4] = { 25 / 255.  ,23 / 255. , 20 / 255.,1 }; // player's arm diffuse
	float mat_diffuse_leg[4] = { 57 / 255, 165 / 255,  203 / 255. ,1 }; // player's arm diffuse
	float mat_diffuse_head[4] = { 255 / 255.,  149 / 255., 120 / 255. ,1 }; // player's arm diffuse
	float mat_diffuse_body[4] = { 255 / 255,99 / 255.,71 / 255. ,1 }; // player's arm diffuse

	if (int(time) % 500 == 0) {
		t = 0;

	}
	glPushMatrix();
	glTranslatef(0, 20, 0); //20��ŭ �÷�����
	glPushMatrix();
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse_body);
	Draw_Body();     //20��ŭ �÷�����+���� �׸���

	glPopMatrix();
	glPushMatrix();
	glTranslatef(1.5, -9, 0);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse_leg);
	Draw_Pelvis();
	glPopMatrix();
	glPushMatrix();
	glTranslatef(-1.5, -9, 0);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse_leg);
	Draw_Pelvis();
	glPopMatrix();
	glPushMatrix();		  //20��ŭ �÷�����+������ ����
	glTranslatef(2, 6.0, 0);
	//glRotatef(sin(time * 0.01) * 30 - 30, 0, 0, 1);
	glRotatef(fabs(sin(time * 0.01)) * t, -1, 1, -1);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse_arm);
	Draw_Arm();
	glPushMatrix(); //20��ŭ �÷�����+������ ����+�ȸ�
	Draw_Cuff();
	glPushMatrix();
	glTranslatef(0, 2.5, 0);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse_hand);
	Draw_Hand();
	glPopMatrix();
	glPopMatrix();
	glPopMatrix();

	glPushMatrix();		  //20��ŭ �÷�����+���� ����
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse_arm);
	glTranslatef(-3, 4.0, 0);
	glRotatef(sin(time * 0.01) * 30 + 180, 1, 0, 0);
	Draw_Arm();
	glPushMatrix(); //20��ŭ �÷�����+���� ����+�ȸ�
	Draw_Cuff();
	glPushMatrix();
	glTranslatef(-2, 2.5, 0);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse_hand);
	Draw_Hand();
	glPopMatrix();
	glPopMatrix();
	glPopMatrix();
	glPushMatrix();		  //20��ŭ �÷�����+�����ٸ� ����
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse_leg);
	glTranslatef(-1.5, -10.5, 0);
	glRotatef(cos(time * 0.01) * 40, 1, 0, 0);
	glTranslatef(0, -3, 0);
	//glRotatef(180, 0, 1,0);
	Draw_Leg();
	glPushMatrix();
	glTranslatef(1, -3, -3);
	DrawFoot(); //20��ŭ �÷�����+�����ٸ� �����ϰ� + �߱׸���
	glPopMatrix();

	glPopMatrix();
	glPushMatrix();		  //20��ŭ �÷�����+�޴ٸ� ����
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse_leg);
	glTranslatef(1.5, -10.5, 0);
	glRotatef(-cos(time * 0.01) * 40, 1, 0, 0);
	glTranslatef(0, -3, 0);
	//glRotatef(180, 0, 1,0);
	Draw_Leg();
	glPushMatrix();
	glTranslatef(1, -3, -3);
	DrawFoot(); //20��ŭ �÷�����+�޴ٸ� �����ϰ� + �߱׸���
	glPopMatrix();

	glPopMatrix();


	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse_head);
	Draw_Head();
	glPushMatrix();
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse_hat);
	glTranslatef(0, 2, 0);//����
	glScalef(0.5, 0.5, 0.5);
	glRotatef(90, 1, 0, 0);
	glutSolidTorus(3, 4, 20, 20);
	glPushMatrix();
	glTranslatef(0, 0, -4);
	glScalef(4, 4, 4);

	glRotatef(-45, 1, 0, 0);
	glutSolidSphere(1, 20, 20);
	glPopMatrix();
	glPopMatrix();
	glPopMatrix();
}

void DrawSideLine() {
	float mat_diffuse8[4] = { 1,1,1,1 }; // middleLine diffuse
	glPushMatrix();
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse8);
	glTranslatef(12.5, 0.3, 0);
	glScalef(1, 0.5, 100); //�ֺ���
	glutSolidCube(1.0);
	glTranslatef(-25, 0.3, 0);
	glutSolidCube(1.0); //�ֺ���
	glPopMatrix();
}


void MyDisplay() {


	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	float mat_diffuse_defaultPlayer[4] = { 0.0,0.8,0.3,1 }; // player diffuse
	float mat_diffuse2_defaultEnemy[4] = { 1,0.1,0.1,1 }; // enemy diffuse
	float mat_diffuse_map[4] = { 0.5,0.5,0.5,1 }; // map diffuse
	float mat_diffuse_aspalt[4] = { 0.05,0.05,0.05,1 }; // aspalt diffuse
	float mat_diffuse_player_health[4] = { 1,0.05,0.05,1 }; // player health diffuse


	glColor3f(1, 1, 0);
	glPushMatrix();
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse_map);
	glScalef(100, 0.5, 100); //x-z���� ���� �� 
	glutSolidCube(1.0);

	glPopMatrix();
	glPushMatrix();
	glTranslatef(0, 0.1, 0);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse_aspalt);
	glScalef(25, 0.5, 100); //x-z���� ���� �� 
	glutSolidCube(1.0);
	glPopMatrix();



	//player
	glPushMatrix();
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse_defaultPlayer); // �߰� �����Ӽ�
	glTranslatef(0, 0, 8); //�⺻��ġ
	glTranslatef(x, 0, y); //Ű��������
	glScalef(0.2, 0.2, 0.2);
	DrawMan();
	glTranslatef(0, 40, 0);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse_player_health); // �߰� �����Ӽ�
	glScalef(health_player * 0.025, 1, 1);//ü�¹�
	glutSolidCube(1);
	glPopMatrix();

	//Enemy

	glPushMatrix();
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse2_defaultEnemy);
	glTranslatef(0, 0, -10); //�⺻��ġ
	glTranslatef(sin(time * 0.001) * 10, 0, 0);
	glScalef(0.4, 0.4, 0.4);
	DrawEnemy();
	glTranslatef(0, 20, 0);
	glScalef(2, 2, 2);
	glScalef(health_enemy * 0.001, 0.7, 0.7);//ü�¹�
	glutSolidCube(1);
	glPopMatrix();

	DrawStoneballs();
	DrawMiddleLine();
	DrawSideLine();
	DrawBooms();

	bool player_hit = playerHitCheck();
	bool enemy_hit = enemy_HitCheck();
	bool inLoad = LoadCheck();
	if (inLoad == false) {}//exit(0);

	if (health_enemy <= 0 || health_player <= 0) {
		printf("///////////////////////////////////////////////////////////////////////\n");
		printf("///////////////////////////////////////////////////////////////////////\n");
		printf("/////////////////////////////   THE END    ////////////////////////////\n");
		printf("///////////////////////////////////////////////////////////////////////\n");
		printf("///////////////////////////////////////////////////////////////////////\n");
		exit(0);
	}

	glFlush();

}


int main(int argc, char** argv) {
	glutInit(&argc, argv);               //GLUT ������ �Լ�
	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(600, 600);

	glutCreateWindow("OpenGL Drawing Example");
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glutDisplayFunc(MyDisplay);
	glutReshapeFunc(MyReshape);
	//glutIdleFunc(MyIdle);
	glutSpecialFunc(MySpecial);
	glutMotionFunc(MyMouseMove);
	glutKeyboardFunc(MyKeyboard);
	glClearColor(0.0, 0.0, 0.0, 1.0);
	// gluLookAt�� ����ϴ� Ŭ���� ����
	cam.set(0, 3, 4, 0, 0, 0, 0, 1, 0);
	InitLight(); //�÷� ����
	glutTimerFunc(1, MyTimer, 1);
	glutTimerFunc(1, MyTimer2, 1);

	glutMainLoop();

	return 0;
}