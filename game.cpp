#include "game.hpp"

struct vec3 {
	double x, y, z;
	vec3 () {}
	vec3 operator+= (const vec3 &r) { x += r.x; y += r.y; z += r.z; return *this; }
	vec3 operator* (const double &r) { x *= r; y *= r; z *= r; return *this; }
	vec3 (double a, double b, double c) {
		x = a;
		y = b;
		z = c;
	}
};

struct rgb {
	unsigned char r, g, b;

	rgb() {}
	rgb(unsigned char nr, unsigned char ng, unsigned char nb) {
		r = nr;
		g = ng;
		b = nb;
	}
	void unpack(uint32_t col) {
		r = ((col & 0xff0000) >> 16);
		g = ((col & 0x00ff00) >> 8);
		b =  (col & 0x0000ff);
	}
	uint32_t pack() const {
		return ((0xFF << 24) | (r << 16) | (g << 8) | b);
	}
};

const rgb skyColor(123, 202, 239);

const int WorldSizeX = 16;
const int WorldSizeY = 16;
const int WorldSizeZ = 16;

uint8_t world[WorldSizeX*WorldSizeY*WorldSizeZ] = {};

vec3 position;
vec3 velocity;
double pitch = 0;
double yaw = 0;

int pitchChange = 0;
int yawChange = 0;
double pitchCos;
double yawCos;
double pitchSin;
double yawSin;

bool keyW = false;
bool keyA = false;
bool keyS = false;
bool keyD = false;

bool inWorld(int x, int y, int z) {
	return (x >= 0 && y >= 0 && z >= 0 &&
			x < WorldSizeX && y < WorldSizeY && z < WorldSizeZ);
}

void updateView()
{
	if (pitch > M_PI_2) pitch = M_PI_2;
	else if (pitch < -M_PI_2) pitch = -M_PI_2;

	pitchSin = sin(pitch);
	pitchCos = cos(pitch);

	if (yaw > M_PI*2 || yaw < -M_PI*2)
		yaw = 0;

	yawSin = sin(yaw);
	yawCos = cos(yaw);
}

void Init()
{
}

void Update(double dt)
{
	pitch += 1.2*pitchChange*dt;
	yaw += 1.2*yawChange*dt;
	updateView();

	velocity.x = velocity.z = 0.0;

	if (keyA) {
		velocity.x += dt*200.0*cos(M_PI - yaw);
		velocity.z += dt*200.0*sin(M_PI - yaw);
	}
	if (keyW) {
		velocity.x += dt*200.0*cos(-M_PI_2 - yaw);
		velocity.z += dt*200.0*sin(-M_PI_2 - yaw);
	}
	if (keyS) {
		velocity.x += dt*200.0*cos(M_PI_2 - yaw);
		velocity.z += dt*200.0*sin(M_PI_2 - yaw);
	}
	if (keyD) {
		velocity.x += dt*200.0*cos(-yaw);
		velocity.z += dt*200.0*sin(-yaw);
	}

	// Simulate gravity
	velocity.y -= 20.0*dt;

	position += velocity*dt;
}

void handleInput(const uint8_t *states, bool down) {
	// View
	if (states[SDL_SCANCODE_UP])
		pitchChange += down ? 1 : -1;
	if (states[SDL_SCANCODE_DOWN])
		pitchChange += down ? -1 : 1;

	if (states[SDL_SCANCODE_LEFT])
		yawChange += down ? 1 : -1;
	if (states[SDL_SCANCODE_RIGHT])
		yawChange += down ? -1 : 1;

	// Movement
	if (states[SDL_SCANCODE_A])
		keyA = down;
	if (states[SDL_SCANCODE_W])
		keyW = down;
	if (states[SDL_SCANCODE_S])
		keyS = down;
	if (states[SDL_SCANCODE_D])
		keyD = down;

	if (states[SDL_SCANCODE_SPACE] && down)
		velocity.y += 8.0;
}

void DrawFrame(PixelDrawer *drawer)
{
	for (int dy = 0; dy < 600; dy++)
		for (int dx = 0; dx < 800; dx++)
			drawer->WritePixel(dx, dy, skyColor.pack());

	// Aim reticle
	for (int x = 400-5; x < 400+5; x++) {
		drawer->WritePixel(x, 299, 0xFFFFFFFF);
		drawer->WritePixel(x, 300, 0xFFFFFFFF);
	}
	for (int y = 300-5; y < 300+5; y++) {
		drawer->WritePixel(399, y, 0xFFFFFFFF);
		drawer->WritePixel(400, y, 0xFFFFFFFF);
	}
}

