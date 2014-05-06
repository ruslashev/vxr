#include <vector>
#include "game.hpp"

struct vec3 {
	double x, y, z;
	vec3 ()                             : x(0), y(0), z(0) {}
	vec3 (double a)                     : x(a), y(a), z(a) {}
	vec3 (double a, double b, double c) : x(a), y(b), z(c) {}

	vec3 operator+ (const vec3 &v) const { return vec3(x + v.x, y + v.y, z + v.z); }
	vec3& operator+= (const vec3 &v) { x += v.x; y += v.y; z += v.z; return *this; }
	vec3 operator- (const vec3 &v) const { return vec3(x - v.x, y - v.y, z - v.z); }
	vec3 operator* (const double &r) const { return vec3(x*r, y*r, z*r); }
	vec3 operator* (const vec3 &v) const { return vec3(x*v.x, y*v.y, z*v.z); }

	double lengthSq() const { return x*x + y*y + z*z; }
	double length() const { return sqrt(lengthSq()); }

	void normalize() {
		double lenSq = lengthSq();
		if (lenSq != 0) {
			const double len = sqrt(lenSq);
			x /= len; y /= len; z /= len;
		}
	}
	double dot(const vec3 &v) const { return x*v.x + y*v.y + z*v.z; }
};

class Sphere
{
public:
	vec3 center;
	double radius;
	vec3 surfaceColor, emissionColor;
	double reflection;
	Sphere(const vec3 &c, const double &r, const vec3 &sc,
			const double &refl = 0, const vec3 &ec = 0) :
		center(c), radius(r), surfaceColor(sc), emissionColor(ec),
		reflection(refl)
	{}

	bool intersect(const vec3 &rayorig, const vec3 &raydir, double *dist) const
	{
		const vec3 l = center - rayorig;
		const double tca = l.dot(raydir);
		if (tca < 0)
			return false;

		double d2 = l.dot(l) - tca*tca;
		double radiusSq = radius*radius;
		if (d2 > radiusSq)
			return false;

		double thc = sqrt(radiusSq - d2);
		if (dist != NULL) {
			double t0 = tca - thc;
			double t1 = tca + thc;
			if (t0 < 0)
				*dist = t1;
			else
				*dist = t0;
		}

		return true;
	}
};

vec3 position;
vec3 velocity;
double pitch = 0;
double yaw = 0;

int pitchChange = 0;
int yawChange = 0;

bool keyW = false;
bool keyA = false;
bool keyS = false;
bool keyD = false;

void Update(double dt)
{
	pitch += 1.2*pitchChange*dt;
	yaw += 1.2*yawChange*dt;

	if (pitch > M_PI_2) pitch = M_PI_2;
	else if (pitch < -M_PI_2) pitch = -M_PI_2;

	if (yaw > M_PI*2 || yaw < -M_PI*2)
		yaw = 0;

	velocity = 0;

	if (keyA)
		velocity.x -= 100.0*dt;
	if (keyW)
		velocity.z -= 100.0*dt;
	if (keyS)
		velocity.z += 100.0*dt;
	if (keyD)
		velocity.x += 100.0*dt;

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
	keyW = keyA = keyS = keyD = false;
	if (down) {
		keyW = states[SDL_SCANCODE_W];
		keyA = states[SDL_SCANCODE_A];
		keyS = states[SDL_SCANCODE_S];
		keyD = states[SDL_SCANCODE_D];
	}

	if (states[SDL_SCANCODE_SPACE] && down)
		velocity.y += 8.0;
}

double lerp(const double &a, const double &b, const double &t)
{
	return b*t + a*(1 - t);
}

const double bias = 0.0001;
const int maxDepth = 10;
vec3 trace(const vec3 &origin, const vec3 &direction,
		const std::vector<Sphere*> spheres, const int depth)
{
	double distance = INFINITY;
	Sphere *sphere = NULL;
	for (auto &s : spheres) {
		double newDist = INFINITY;
		if (s->intersect(origin, direction, &newDist)) {
			if (newDist < distance) {
				distance = newDist;
				sphere = s;
			}
		}
	}
	if (sphere == NULL)
		return vec3(2);

	vec3 surfaceColor;
	vec3 intersection = origin + direction*distance;
	vec3 normal = intersection - sphere->center;
	normal.normalize();

	if (depth < maxDepth && sphere->reflection > 0) {
		double facingRatio = -direction.dot(normal);
		double fresnelEffect = lerp(pow((1-facingRatio), 3), 1, 0.1); // TODO adjust

		vec3 reflectionDir = direction - normal*2*direction.dot(normal);
		vec3 reflection = trace(intersection + normal*bias,
				reflectionDir, spheres, depth+1);
		surfaceColor = reflection*fresnelEffect*sphere->surfaceColor;
	} else { // diffuse
		for (unsigned i = 0; i < spheres.size(); i++) {
			if (spheres[i]->emissionColor.x > 0) {
				// this is a light
				vec3 transmission = 1;
				vec3 lightDirection = spheres[i]->center - intersection;
				lightDirection.normalize();
				for (unsigned j = 0; j < spheres.size(); ++j) {
					if (i == j)
						continue;
					if (spheres[j]->intersect(
								intersection + normal*bias,
								lightDirection, NULL))
					{
						transmission = 0;
						break;
					}
				}
				surfaceColor += sphere->surfaceColor * transmission *
					std::max(0.0, normal.dot(lightDirection)) * spheres[i]->emissionColor;
			}
		}
	}
	return surfaceColor + sphere->emissionColor;
}

std::vector<Sphere*> spheres;
void Init()
{
	spheres.push_back(new Sphere(vec3(0, 0, -20), 4, vec3(1.00, 0.32, 0.36), 1, 0.0));
	spheres.push_back(new Sphere(vec3(5, -1, -15), 2, vec3(0.90, 0.76, 0.46), 1, 0.0));
	spheres.push_back(new Sphere(vec3(5, 0, -25), 3, vec3(0.65, 0.77, 0.97), 1, 0.0));
	spheres.push_back(new Sphere(vec3(-5.5, 0, -15), 3, vec3(0.90, 0.90, 0.90), 1, 0.0));

	spheres.push_back(new Sphere(vec3(0, -10004, -20), 10000, vec3(0.2), 0, 0.0));
	// light
	spheres.push_back(new Sphere(vec3(0, 20, -30), 3, vec3(0), 0, vec3(3)));
}

void Cleanup()
{
	while (!spheres.empty()) {
		Sphere *s = spheres.back();
		delete s;
		spheres.pop_back();
	}
}

void DrawFrame(PixelDrawer *drawer)
{
	double fov = 30;
	double angle = tan(M_PI_2 * fov / 180.0);
	double aspectRatio = (double)WindowWidth / WindowHeight;

	for (int dy = 0; dy < WindowHeight; dy++)
		for (int dx = 0; dx < WindowWidth; dx++) {
			double x = (2*(dx + 0.5)/WindowWidth-1) * angle * aspectRatio;
			double y = (1 - 2*(dy+0.5)/WindowHeight) * angle;
			vec3 direction(x, y, -1);
			direction.normalize();
			vec3 pxColor = trace(position, direction, spheres, 0);
			pxColor.x = std::min(1.0, pxColor.x);
			pxColor.y = std::min(1.0, pxColor.y);
			pxColor.z = std::min(1.0, pxColor.z);
			pxColor = pxColor*255.0;
			uint32_t outColor = ((0xFF << 24) |
					((unsigned char)pxColor.x << 16) |
					((unsigned char)pxColor.y << 8) |
					(unsigned char)pxColor.z);
			drawer->WritePixel(dx, dy, outColor);
		}

	// Aim reticle
	for (int x = WindowWidth/2-4; x < WindowWidth/2+5; x++)
		drawer->WritePixel(x, WindowHeight/2, 0xFFFFFFFF);
	for (int y = WindowHeight/2-4; y < WindowHeight/2+5; y++)
		drawer->WritePixel(WindowWidth/2, y, 0xFFFFFFFF);
}

