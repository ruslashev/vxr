#include "game.hpp"

struct rgb {
	unsigned char r, g, b;

	void unpack(uint32_t col) {
		r = ((col & 0xff0000) >> 16);
		g = ((col & 0x00ff00) >> 8);
		b =  (col & 0x0000ff);
	}
	uint32_t pack() {
		return ((0xff << 24) | (r << 16) | (g << 8) | b);
	}
	void darken() {
		r *= 2/3;
		g *= 2/3;
		b *= 2/3;
	}
};

struct vec3 {
	double x, y, z;
	void set(double a, double b, double c) {
		x = a;
		y = b;
		z = c;
	}
};

struct ray {
	bool hit;
	int x, y, z;
	int nx, ny, nz;
	double dist;
};

enum block {
    BLOCK_AIR,
    BLOCK_DIRT
};

enum face {
    FACE_LEFT,
    FACE_RIGHT,
    FACE_BOTTOM,
    FACE_TOP,
    FACE_BACK,
    FACE_FRONT
};

const int WorldSizeX = 16;
const int WorldSizeY = 16;
const int WorldSizeZ = 16;

const int hFov = 90;

const rgb sky { 158, 207, 255 };

extern unsigned int texGrass[];
extern unsigned int texDirt[];
extern unsigned int texGrassSide[];

uint8_t world[WorldSizeX * WorldSizeY * WorldSizeZ] = {};
uint8_t lighting[WorldSizeX * WorldSizeY] = {};

vec3 position = { 8, 10, 8 };
vec3 velocity = { 0, 0, 0 };
double pitch = 0.0;
double yaw = 0.0;
double pitchChange = 0;
double yawChange = 0;
double pitchC = 0.0;
double yawC = 0.0;
double pitchS = 0.0;
double yawS = 0.0;

bool keyA = false;
bool keyW = false;
bool keyS = false;
bool keyD = false;

void handleCollision(vec3 pos, vec3* velocity);
rgb raytrace(vec3 pos, vec3 dir, ray* info);

bool inWorld(int x, int y, int z) {
	return (x >= 0 && y >= 0 && z >= 0 &&
			x < WorldSizeX && y < WorldSizeY && z < WorldSizeZ);
}

uint8_t getBlock(int x, int y, int z) {
    return world[x*WorldSizeY*WorldSizeZ + y*WorldSizeZ + z];
}

void setBlock(int x, int y, int z, uint8_t type) {
    world[x*WorldSizeY*WorldSizeZ + y*WorldSizeZ + z] = type;

    // Update lightmap
    int lightIdx = x*WorldSizeZ + z;

    if (type != BLOCK_AIR && lighting[lightIdx] < y) {
        lighting[lightIdx] = y;
    } else if (type == BLOCK_AIR && lighting[lightIdx] <= y) {
        y = WorldSizeY - 1;

        while (y > 0 && getBlock(x, y, z) == BLOCK_AIR) {
            y--;
        }

        lighting[lightIdx] = y;
    }
}

int getLight(int x, int z) {
    return lighting[x * WorldSizeZ + z];
}

void setView(double p, double y) {
    pitch = p;

    if (pitch > 1.57) pitch = 1.57;
    else if (pitch < -1.57) pitch = -1.57;

    pitchS = sin(pitch);
    pitchC = cos(pitch);

    yaw = y;
    yawS = sin(yaw);
    yawC = cos(yaw);
}

void Init()
{
	// Make flat grass landscape
	for (int x = 0; x < WorldSizeX; x++)
		for (int y = 0; y < WorldSizeY; y++)
			for (int z = 0; z < WorldSizeZ; z++)
				setBlock(x, y, z, y >= WorldSizeY / 2 ? BLOCK_AIR : BLOCK_DIRT);

	// Add arch
	setBlock(11, 8, 4, BLOCK_DIRT);
	setBlock(11, 9, 4, BLOCK_DIRT);
	setBlock(11, 10, 4, BLOCK_DIRT);
	setBlock(10, 10, 4, BLOCK_DIRT);
	setBlock(9, 10, 4, BLOCK_DIRT);
	setBlock(9, 9, 4, BLOCK_DIRT);
	setBlock(9, 8, 4, BLOCK_DIRT);
	setBlock(9, 12, 4, BLOCK_DIRT);

	// Initial player position
	position.set(8.0, 9.8, 8.0);
	setView(0.0, -0.35);
}

void Update(double dt)
{
	// Update view
	pitch += 1.2*pitchChange*dt;
	yaw += 1.2*yawChange*dt;

	setView(pitch, yaw);

	// Set X/Z velocity depending on input
	velocity.x = velocity.z = 0.0;

	if (keyA) {
		velocity.x += 2.0 * cos(M_PI - yaw);
		velocity.z += 2.0 * sin(M_PI - yaw);
	}
	if (keyW) {
		velocity.x += 2.0 * cos(-M_PI / 2 - yaw);
		velocity.z += 2.0 * sin(-M_PI / 2 - yaw);
	}
	if (keyS) {
		velocity.x += 2.0 * cos(M_PI / 2 - yaw);
		velocity.z += 2.0 * sin(M_PI / 2 - yaw);
	}
	if (keyD) {
		velocity.x += 2.0 * cos(-yaw);
		velocity.z += 2.0 * sin(-yaw);
	}

	// Simulate gravity
	velocity.y -= 20.0*dt;

	// Handle block collision (head, lower body and feet)
	vec3 headPos = position;
	vec3 lowerPos = position;
	lowerPos.y -= 1.0;
	vec3 footPos = position;
	footPos.y -= 1.8;

	handleCollision(headPos, &velocity);
	handleCollision(lowerPos, &velocity);
	handleCollision(footPos, &velocity);

	// Apply motion
	position.x += velocity.x * dt;
	position.y += velocity.y * dt;
	position.z += velocity.z * dt;
}

vec3 rayDir(int x, int y) {
    static double vFov = -1, fov;

    // Calculate vertical fov and fov constant from specified horizontal fov
    if (vFov == -1) {
        vFov = 2.0 * atan(tan(hFov / (1300.0) * M_PI) * 800.0 / 600.0);
        fov = tan(vFov * 0.5);
    }

    // This is simply a precomputed version of the actual linear
    // transformation, which is the inverse of the common view and
    // projection transformation used in rasterization.
    double clipX = x / 400.0 - 1.0;
    double clipY = 1.0 - y / 300.0;

    vec3 d = {
        1.6 * fov * yawC * clipX + fov * yawS * pitchS * clipY - pitchC * yawS,
        fov * pitchC * clipY + pitchS,
        -1.6 * fov * yawS * clipX + fov * yawC * pitchS * clipY - pitchC * yawC
    };

    // Normalize
    double length = sqrt(d.x * d.x + d.y * d.y + d.z * d.z);
    d.x /= length;
    d.y /= length;
    d.z /= length;

    return d;
}

void handleInput(const uint8_t *states, bool down) {
	ray info;

	// View
	if (states[SDL_SCANCODE_UP])
		pitchChange += down ? 1.0 : -1.0;
	if (states[SDL_SCANCODE_DOWN])
		pitchChange += down ? -1.0 : 1.0;

	if (states[SDL_SCANCODE_LEFT])
		yawChange += down ? 1.0 : -1.0;
	if (states[SDL_SCANCODE_RIGHT])
		yawChange += down ? -1.0 : 1.0;

	// Movement
	if (states[SDL_SCANCODE_A])
		keyA = down;
	if (states[SDL_SCANCODE_W])
		keyW = down;
	if (states[SDL_SCANCODE_S])
		keyS = down;
	if (states[SDL_SCANCODE_D])
		keyD = down;

	if (states[SDL_SCANCODE_SPACE])
		if (down)
			velocity.y += 8.0;

	// Check if a block was hit and place a new block next to it
	if (states[SDL_SCANCODE_Q]) {
		if (!down) {
			raytrace(position, rayDir(400, 300), &info);

			if (info.hit) {
				int bx = info.x + info.nx;
				int by = info.y + info.ny;
				int bz = info.z + info.nz;

				if (inWorld(bx, by, bz))
					setBlock(bx, by, bz, BLOCK_DIRT);
			}
		}
	}

	// Check if a block was hit and remove it
	if (states[SDL_SCANCODE_E]) {
		if (!down) {
			raytrace(position, rayDir(400, 300), &info);

			if (info.hit)
				setBlock(info.x, info.y, info.z, BLOCK_AIR);
		}
	}
}

void handleCollision(vec3 pos, vec3* velocity) {
	// Check if new position is not inside block
	ray info;
	raytrace(pos, *velocity, &info);

	// If it is, create sliding motion by negating velocity based on hit normal
	if (info.hit && info.dist < 0.1) {
		if (info.nx != 0) velocity->x = 0.0;
		if (info.ny != 0) velocity->y = 0.0;
		if (info.nz != 0) velocity->z = 0.0;
	}
}

void faceNormal(int face, int* x, int* y, int* z) {
    *x = 0;
    *y = 0;
    *z = 0;

    switch (face) {
        case FACE_LEFT:   *x = -1; break;
        case FACE_RIGHT:  *x =  1; break;
        case FACE_BOTTOM: *y = -1; break;
        case FACE_TOP:    *y =  1; break;
        case FACE_BACK:   *z = -1; break;
        case FACE_FRONT:  *z =  1; break;
    }
}

rgb rayColor(int x, int y, int z, int tex, int face) {
    // Get normal
    int nx, ny, nz;
    faceNormal(face, &nx, &ny, &nz);

    // Block is dirt if there's another block directly on top of it
    bool isDirt = (y < WorldSizeY-1) && (getBlock(x, y + 1, z) != BLOCK_AIR);

    // Texture lookup
    uint32_t texColor;
    if (face == FACE_BOTTOM || isDirt) {
        texColor = texDirt[tex];
    } else if (face == FACE_TOP) {
        texColor = texGrass[tex];
    } else {
        texColor = texGrassSide[tex];
    }
	printf("%x\n", texColor);

	rgb color;
	color.unpack(texColor);

    // Side is dark if there are higher blocks in the column faced by it
    // Left and back sides are always dark to simulate a sun angle
    if (inWorld(x + nx, y, z + nz) && getLight(x + nx, z + nz) > y) {
		color.darken();
    } else if (face == FACE_BOTTOM || face == FACE_LEFT || face == FACE_BACK) {
		color.darken();
    }

	return color;
}

int texIndex(vec3 pos, int face) {
	puts("wat");
    double u = 0, v = 0;

    switch (face) {
        case FACE_LEFT:   u = pos.z; v = pos.y; break;
        case FACE_RIGHT:  u = pos.z; v = pos.y; break;
        case FACE_BOTTOM: u = pos.x; v = pos.z; break;
        case FACE_TOP:    u = pos.x; v = pos.z; break;
        case FACE_BACK:   u = pos.x; v = pos.y; break;
        case FACE_FRONT:  u = pos.x; v = pos.y; break;
    }

    v = 1.0 - v;

    return ((int)(u * 15.0))*16 + (int)(v*15.0);
}

rgb raytrace(vec3 pos, vec3 dir, ray* info) {
	vec3 start = pos;

	int x = (int) pos.x;
	int y = (int) pos.y;
	int z = (int) pos.z;

	int x_dir = dir.x >= 0.0 ? 1 : -1;
	int y_dir = dir.y >= 0.0 ? 1 : -1;
	int z_dir = dir.z >= 0.0 ? 1 : -1;

	double dx_off = x_dir > 0 ? 1.0 : 0.0;
	double dy_off = y_dir > 0 ? 1.0 : 0.0;
	double dz_off = z_dir > 0 ? 1.0 : 0.0;

	int x_face = x_dir > 0 ? FACE_LEFT : FACE_RIGHT;
	int y_face = y_dir > 0 ? FACE_BOTTOM : FACE_TOP;
	int z_face = z_dir > 0 ? FACE_BACK : FACE_FRONT;

	int face = FACE_TOP;

	// Finish early if there's no direction
	if (dir.x == 0.0 && dir.y == 0.0 && dir.z == 0.0) {
		goto nohit;
	}

	// Assumption is made that the camera is never outside the world
	while (inWorld(x, y, z)) {
		// Determine if block is solid
		if (getBlock(x, y, z) != BLOCK_AIR) {
			puts("what the christ");
			double dx = start.x - pos.x;
			double dy = start.y - pos.y;
			double dz = start.z - pos.z;
			double dist = sqrt(dx*dx + dy*dy + dz*dz);

			pos.x -= x;
			pos.y -= y;
			pos.z -= z;

			// If hit info is requested, no color computation is done
			if (info != NULL) {
				int nx, ny, nz;
				faceNormal(face, &nx, &ny, &nz);

				info->hit = true;
				info->x = x;
				info->y = y;
				info->z = z;
				info->nx = nx;
				info->ny = ny;
				info->nz = nz;
				info->dist = dist;

				return {0, 0, 0};
			}

			int tex = texIndex(pos, face);

			return rayColor(x, y, z, tex, face);
		}

		// Remaining distance inside this block given ray direction
		double dx = x - pos.x + dx_off;
		double dy = y - pos.y + dy_off;
		double dz = z - pos.z + dz_off;

		// Calculate distance for each dimension
		double t1 = dx / dir.x;
		double t2 = dy / dir.y;
		double t3 = dz / dir.z;

		// Find closest hit
		if (t1 <= t2 && t1 <= t3) {
			pos.x += dx;
			pos.y += t1 * dir.y;
			pos.z += t1 * dir.z;
			x += x_dir;
			face = x_face;
		}
		if (t2 <= t1 && t2 <= t3) {
			pos.x += t2 * dir.x;
			pos.y += dy;
			pos.z += t2 * dir.z;
			y += y_dir;
			face = y_face;
		}
		if (t3 <= t1 && t3 <= t2) {
			pos.x += t3 * dir.x;
			pos.y += t3 * dir.y;
			pos.z += dz;
			z += z_dir;
			face = z_face;
		}
	}

nohit:
	if (info != NULL) {
		info->hit = false;
	}

	return sky;
}

void DrawFrame(PixelDrawer *drawer)
{
    int x = 0;
    int y = 0;

    // Draw world
    for (int dy = 0; dy < 600; dy++)
    for (int dx = 0; dx < 800; dx++)
		drawer->WritePixel(dx, dy, raytrace(position, rayDir(x, y), NULL).pack());

    // Inverse colors in the center of screen to form an aim reticle
    for (x = 400-5; x < 400+5; x++) {
		drawer->WritePixel(x, 299, 0xFFFFFFFF);
		drawer->WritePixel(x, 300, 0xFFFFFFFF);
    }
    for (y = 300-5; y < 300+5; y++) {
		drawer->WritePixel(399, y, 0xFFFFFFFF);
		drawer->WritePixel(400, y, 0xFFFFFFFF);
    }
}

