#include <GL/glut.h>
#include<math.h>
#include<vector>
#include<stdlib.h>
#include<stdio.h>
//1871124 서지완
/*
	탱크를 부셔라!
	나를향해 폭탄을 날리는 탱크에 맞설 카우보이가 되어주세요.
*/
using namespace std;

int MX = 300, MY = 300;
float time = 0;
float x = 0, y = 0; //플레이어 좌표 전역변수
float middle_x = 0, middle_y = 0, middle_z = -30; // 중앙선의 시작 좌표
float health_player = 1000; //체력
float health_enemy = 10000; //체력
float t = 0; //팔을 앞으로 뻣기위한 전역변수
float enemy_oldx, enemy_oldy;
float p_x = 0, p_y = 8;

class Point3 { //정점클래스
public:
	float x, y, z;//좌표
	void set(float dx, float dy, float dz) { x = dx; y = dy; z = dz; }//정점좌표변경메소드
	void set(Point3& p) { x = p.x; y = p.y; z = p.z; }//정점좌표변경메소드2 다른정점 대입
	Point3(float xx, float yy, float zz) { x = xx; y = yy; z = zz; }//생성자: 정점초기화
	Point3() { x = y = z = 0; }//생성자: 초기화없을시 원점
};
class Vector3 {
public:
	float x, y, z;//벡터성분
	void set(float dx, float dy, float dz) { x = dx; y = dy; z = dz; }//벡터생성메소드
	//void set(Vector3& v){ x = v.x; y = v.y; z = v.z;}//벡터생성메소드
	void set(Vector3 v) { x = v.x; y = v.y; z = v.z; }//벡터생성메소드
	void flip() { x = -x; y = -y; z = -z; } //반대방향벡터
	void setDiff(Point3& a, Point3& b)//두벡터의차
	{
		x = a.x - b.x; y = a.y - b.y; z = a.z - b.z;
	}
	void normalize();//단위벡터로
	Vector3(float xx, float yy, float zz) { x = xx; y = yy; z = zz; } //생성자
	//Vector3(const Vector3& v) {x = v.x; y = v.y; z = v.z;} // 복사생성자
	Vector3() { x = y = z = 0.0; }
	Vector3 cross(Vector3 b);//외적
	float dot(Vector3 b);//내적
};
class Camera {
public:
	Point3 eye;//시점좌표계원점
	Vector3 u, v, n;//시점좌표계를 구성하는 정규직교기저
	float aspect, nearDist, farDist, viewAngle;//gluPerspective의 파라미터들 시점변환후 이용
	void setModelViewMatrix();// 시점변환에 따라 모델뷰행렬조정
	Camera(void); // 생성자

	// 비행시뮬레이터의 회전 변환함수들 roll pitch yaw
	void roll(float angle);
	void pitch(float angle);
	void yaw(float angle);
	// 이동변환
	void slide(float du, float dv, float dn);

	//카메라위치, 시점,카메라업벡터 지정
	void set(Point3 Eye, Point3 look, Vector3 up); // 벡터로
	void set(float eyeX, float eyeY, float eyeZ, float lookX, float lookY, float lookZ, float upX, float upY, float upZ); //점으로

	void setShape(float viewAngle, float aspect, float Near, float Far); //화면정의
	void setAspect(float asp); // 종횡비정의
	void getShape(float& viewAngle, float& aspect, float& Near, float& Far); //화면구성값보기
	void rotAxes(Vector3& a, Vector3& b, float angle);//시점좌표계축회전
	void setDefaultCamera();// 카메라초기설정함수
};

Camera::Camera(void) {
	setDefaultCamera();//생성자, 초기화함수 별도 호출
}
void Camera::setDefaultCamera(void) {// 초기화함수
	setShape(45.0f, 640 / (float)480, 0.1f, 200.0f);//화면구성값 정의 fov,aspect,near clip,far clip
	Point3 eyePoint = Point3(10.0, 0.0, 0.0); // 카메라위치 지정
	Point3 lookPoint = Point3(0.0, 0.0, 0.0); // 바라보는 지점 지정
	Vector3 upVector(0, 1, 0); // 카메라의 업벡터 지정
	set(eyePoint, lookPoint, upVector);
}
void Camera::set(float eyeX, float eyeY, float eyeZ, float lookX, float lookY, float lookZ, float upX, float upY, float upZ) {
	Point3 Eye = Point3(eyeX, eyeY, eyeZ); //카메라, 시점, 업벡터 정의
	Point3 look = Point3(lookX, lookY, lookZ);
	Vector3 up(upX, upY, upZ);
	eye.set(Eye);//카메라위치정의
	n.set(eye.x - look.x, eye.y - look.y, eye.z - look.z);//시점과 카메라좌표를 빼서 시축(optical axis)구성
	u.set(up.cross(n)); // 카메라 업벡터와의 내적으로 u구성
	v.set(n.cross(u)); // u와 n의 내적으로 시점좌표계의 y축
	u.normalize();v.normalize();n.normalize();// 세 시점좌표계 축들을 정규화 이로써 카메라 시공간의 정규직교기저가 완성됨
	setModelViewMatrix();// 현재 정의된 시점좌표계를 모델뷰행렬에 적용
}
void Camera::set(Point3 Eye, Point3 look, Vector3 up) { //위의 set함수의 오버라이딩 같은기능이다.
	eye.set(Eye);
	n.set(eye.x - look.x, eye.y - look.y, eye.z - look.z);
	u.set(up.cross(n));
	v.set(n.cross(u));
	u.normalize();v.normalize();n.normalize();
	setModelViewMatrix();
}
void Camera::setModelViewMatrix(void) {
	float m[16];// 모델뷰행렬에 집어넣을 행렬값 동차좌표계 변환행렬로서 4x4=16개의 원소를 가짐
	Vector3 eVec(eye.x, eye.y, eye.z);//카메라위치
	// 모델뷰행렬의 회전변환과 이동변환적용
	m[0] = u.x;	m[4] = u.y;	m[8] = u.z;	m[12] = -eVec.dot(u);
	m[1] = v.x;	m[5] = v.y;	m[9] = v.z;	m[13] = -eVec.dot(v);
	m[2] = n.x;	m[6] = n.y;	m[10] = n.z;	m[14] = -eVec.dot(n);
	m[3] = 0;		m[7] = 0;		m[11] = 0;	m[15] = 1.0;
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(m);//행렬m을 모델뷰행렬에 넣어준다
}
void Camera::setShape(float vAngle, float asp, float nr, float fr) {//투상을 정의한다.
	viewAngle = vAngle;//시야각
	aspect = asp;//종횡비
	nearDist = nr;//전방투상면
	farDist = fr;//후방투상면
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();// 투상행렬을 새로정의하기위해 단위행렬
	gluPerspective(viewAngle, aspect, nearDist, farDist);//원근투상정의
	glMatrixMode(GL_MODELVIEW);
}
void Camera::setAspect(float asp) {
	aspect = asp;// 종횡비만새로정의
}
void Camera::getShape(float& vAngle, float& asp, float& nr, float& fr) {
	vAngle = viewAngle;//원근투상의 구성값 반환
	asp = aspect;
	nr = nearDist;
	fr = farDist;
}
void Camera::slide(float du, float dv, float dn) {//이동변환
	// 물체 좌표계 기준으로 평행이동 하는 다음 코드와 비교해 볼것
	//eye.x += du;	eye.y += dv;	eye.z += dn;

	eye.x += du * u.x + dv * v.x + dn * n.x;//시점좌표축의 정규직교기저에 이동변환행렬을 곱한다 변환된 결과를 모델뷰행렬에 적용
	eye.y += du * u.y + dv * v.y + dn * n.y;//시점좌표계가 이동한 효과 // Emmanuel Agu자료참고
	eye.z += du * u.z + dv * v.z + dn * n.z;
	setModelViewMatrix();
}
//시점좌표계를 이루는 기저를 기울여 회전변환 // Emmanuel Agu자료 참고
// 해설:
// 2차원 평면의 두 축 x, y 축에 해당하는 단위벡터 a, b 벡터를 이를 angle 만큼 회전하자
// a' = a cos(angle) + b sin(angle) ; 2차원 평면에서 단위원을 그리면 자명함
// b' = -a sin(angle) + b cos(angle) ; a'과 b'의 수직조건 생각하면 자명함
// 이제 a, b 두 벡터를 공간상의 수직인 두 기저 벡터로 확장하면, 수정없이 확장됨
void Camera::rotAxes(Vector3& a, Vector3& b, float angle) {

	float ang = 3.14159265f / 180 * angle;//각을 라디안단위로 변환한다. cos, sin 함수를 이용하기 위해서
	float C = cosf(ang), S = sinf(ang); // 변환하려는 각의 cos, sin 값 구함

	// 이를 이용해 두 축을 angle만큼 회전한다
	Vector3 t(C * a.x + S * b.x, C * a.y + S * b.y, C * a.z + S * b.z);
	b.set(-S * a.x + C * b.x, -S * a.y + C * b.y, -S * a.z + C * b.z);
	a.set(t.x, t.y, t.z);
}
void Camera::roll(float angle) {
	rotAxes(u, v, angle);//n축중심의 회전 나머지 u,v축이 angle각도만큼 회전한다
	setModelViewMatrix();//모델뷰행렬에 적용
}
void Camera::pitch(float angle) {
	rotAxes(n, v, angle);////u축중심의 회전 나머지 n,v축이 angle각도만큼 회전한다
	setModelViewMatrix();//모델뷰행렬에 적용
}
void Camera::yaw(float angle) {
	rotAxes(u, n, angle);//v축중심의 회전 나머지 u,n축이 angle각도만큼 회전한다
	setModelViewMatrix();//모델뷰행렬에 적용
}
//메소드호출벡터와 파라미터벡터의 외적벡터를 반환하는 함수
Vector3 Vector3::cross(Vector3 b) {
	return Vector3(y * b.z - z * b.y, z * b.x - x * b.z, x * b.y - y * b.x);
}
float Vector3::dot(Vector3 b) { return x * b.x + y * b.y + z * b.z; }//두 벡터의 내적
void Vector3::normalize() {//해당벡터를 정규화하는 함수
	double sizeSq = x * x + y * y + z * z;// 크기를 구하고
	if (sizeSq < 0.0000001) {
		//cerr << "\nnormalize() sees vector (0,0,0)!";
		return;// does nothing to zero vectors;
	}
	float scaleFactor = (float)(1.0 / sqrt(sizeSq));// 크기로 나눠준다
	x *= scaleFactor;y *= scaleFactor;z *= scaleFactor;// 나눠준 비율을 기저에 적용하여 정규화
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
void MyTimer2(int n) //2초마다 중앙선을 하나 생성
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
	glOrtho(-15.0, 15.0, -15.0, 15.0, -100.0, 100.0); // 15가 좋은 view
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(MX / 300.0, MY / 300.0, 1.0,
		0.0, 0.0, 0.0,
		0.0, 1.0, 0.0
	);  //시점변환
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
	glEnable(GL_NORMALIZE); //조명계산시 법선벡터를 정확하게 다시해라
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
bool enemy_HitCheck() { // 공과 big stone의 충돌이면 true리턴
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
	vec3 acc(0, -0.8, 0); // 중력가속도는 동일
	float boomTime = 0.01;

	int size = booms.size();
	for (int i = 0; i < size; i++) {
		booms[i].v = booms[i].v + acc * boomTime;
		booms[i].pos = booms[i].pos + booms[i].v * boomTime;

		if (booms[i].pos.m[1] < 0.55) { // 바닥충돌
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
	vec3 acc(0, -0.8, 0); // 중력가속도는 동일
	float stoneTime = 0.01;

	int size = stones.size();
	for (int i = 0; i < size; i++) {
		stones[i].v = stones[i].v + acc * stoneTime;
		stones[i].pos = stones[i].pos + stones[i].v * stoneTime;

		if (stones[i].pos.m[1] < 0.5 + 0.05) { // 바닥충돌
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
	vec3 acc(0, -0.8, 0); // 중력가속도는 동일

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
		glScalef(1, 0.5, 3); //중앙선
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
	glutSolidSphere(3, 20, 20);//몸통
}
void Draw_Arm() {
	glScalef(0.3, 1, 0.3);
	glutSolidCube(5);
}
void Draw_Cuff() {//팔목
	glTranslatef(0, 4, 0);
	glutSolidCube(3);
}
void Draw_Leg() {
	glScalef(0.5, 2, 0.5);
	glutSolidSphere(3, 20, 20);
}
void Draw_Pelvis() { //골반
	//glScalef(1, 1, 1);
	glutSolidSphere(1.5, 20, 20);//몸통
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
	glutSolidCube(5);		//몸통
	glPushMatrix();
	glTranslatef(0, 0, 3);
	glScalef(0.5, 0.5, 1.2);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse_gun);
	glutSolidCube(3);  //총구

	glPopMatrix();
	glPopMatrix();
	glPushMatrix();
	glTranslatef(0, 7.5, 0);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse_middle);
	glutSolidCube(10); //아랫몸통
	glPushMatrix();
	glTranslatef(5, -5, 5);
	glRotatef(90, 0, 1, 0);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse_wheel);
	glutSolidTorus(1, 2, 20, 20); //왼쪽바퀴1
	glTranslatef(5, 0, 0);
	glutSolidTorus(1, 2, 20, 20); //왼쪽바퀴2
	glTranslatef(5, 0, 0);
	glutSolidTorus(1, 2, 20, 20); //왼쪽바퀴3


	glTranslatef(0, 0, -10);
	glutSolidTorus(1, 2, 20, 20); //오른쪽바퀴1
	glTranslatef(-5, 0, 0);
	glutSolidTorus(1, 2, 20, 20); //오른쪽바퀴2
	glTranslatef(-5, 0, 0);
	glutSolidTorus(1, 2, 20, 20); //오른쪽바퀴3
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
	glTranslatef(0, 20, 0); //20만큼 올려놓고
	glPushMatrix();
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse_body);
	Draw_Body();     //20만큼 올려놓고+몸통 그리기

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
	glPushMatrix();		  //20만큼 올려놓고+오른팔 시작
	glTranslatef(2, 6.0, 0);
	//glRotatef(sin(time * 0.01) * 30 - 30, 0, 0, 1);
	glRotatef(fabs(sin(time * 0.01)) * t, -1, 1, -1);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse_arm);
	Draw_Arm();
	glPushMatrix(); //20만큼 올려놓고+오른팔 시작+팔목
	Draw_Cuff();
	glPushMatrix();
	glTranslatef(0, 2.5, 0);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse_hand);
	Draw_Hand();
	glPopMatrix();
	glPopMatrix();
	glPopMatrix();

	glPushMatrix();		  //20만큼 올려놓고+왼팔 시작
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse_arm);
	glTranslatef(-3, 4.0, 0);
	glRotatef(sin(time * 0.01) * 30 + 180, 1, 0, 0);
	Draw_Arm();
	glPushMatrix(); //20만큼 올려놓고+왼팔 시작+팔목
	Draw_Cuff();
	glPushMatrix();
	glTranslatef(-2, 2.5, 0);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse_hand);
	Draw_Hand();
	glPopMatrix();
	glPopMatrix();
	glPopMatrix();
	glPushMatrix();		  //20만큼 올려놓고+오른다리 시작
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse_leg);
	glTranslatef(-1.5, -10.5, 0);
	glRotatef(cos(time * 0.01) * 40, 1, 0, 0);
	glTranslatef(0, -3, 0);
	//glRotatef(180, 0, 1,0);
	Draw_Leg();
	glPushMatrix();
	glTranslatef(1, -3, -3);
	DrawFoot(); //20만큼 올려놓고+오른다리 시작하고 + 발그리기
	glPopMatrix();

	glPopMatrix();
	glPushMatrix();		  //20만큼 올려놓고+왼다리 시작
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse_leg);
	glTranslatef(1.5, -10.5, 0);
	glRotatef(-cos(time * 0.01) * 40, 1, 0, 0);
	glTranslatef(0, -3, 0);
	//glRotatef(180, 0, 1,0);
	Draw_Leg();
	glPushMatrix();
	glTranslatef(1, -3, -3);
	DrawFoot(); //20만큼 올려놓고+왼다리 시작하고 + 발그리기
	glPopMatrix();

	glPopMatrix();


	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse_head);
	Draw_Head();
	glPushMatrix();
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse_hat);
	glTranslatef(0, 2, 0);//모자
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
	glScalef(1, 0.5, 100); //주변선
	glutSolidCube(1.0);
	glTranslatef(-25, 0.3, 0);
	glutSolidCube(1.0); //주변선
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
	glScalef(100, 0.5, 100); //x-z방향 넓은 판 
	glutSolidCube(1.0);

	glPopMatrix();
	glPushMatrix();
	glTranslatef(0, 0.1, 0);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse_aspalt);
	glScalef(25, 0.5, 100); //x-z방향 넓은 판 
	glutSolidCube(1.0);
	glPopMatrix();



	//player
	glPushMatrix();
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse_defaultPlayer); // 추가 재질속성
	glTranslatef(0, 0, 8); //기본위치
	glTranslatef(x, 0, y); //키보드조작
	glScalef(0.2, 0.2, 0.2);
	DrawMan();
	glTranslatef(0, 40, 0);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse_player_health); // 추가 재질속성
	glScalef(health_player * 0.025, 1, 1);//체력바
	glutSolidCube(1);
	glPopMatrix();

	//Enemy

	glPushMatrix();
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse2_defaultEnemy);
	glTranslatef(0, 0, -10); //기본위치
	glTranslatef(sin(time * 0.001) * 10, 0, 0);
	glScalef(0.4, 0.4, 0.4);
	DrawEnemy();
	glTranslatef(0, 20, 0);
	glScalef(2, 2, 2);
	glScalef(health_enemy * 0.001, 0.7, 0.7);//체력바
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
	glutInit(&argc, argv);               //GLUT 윈도우 함수
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
	// gluLookAt을 대신하는 클래스 구현
	cam.set(0, 3, 4, 0, 0, 0, 0, 1, 0);
	InitLight(); //컬러 무시
	glutTimerFunc(1, MyTimer, 1);
	glutTimerFunc(1, MyTimer2, 1);

	glutMainLoop();

	return 0;
}