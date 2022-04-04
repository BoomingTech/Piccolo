#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/matrix_interpolation.hpp>

#include <iostream>

static int test_axisAngle()
{
	int Error = 0;

	glm::mat4 m1(-0.9946f, 0.0f, -0.104531f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.104531f, 0.0f, -0.9946f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);
	glm::mat4 m2(-0.992624f, 0.0f, -0.121874f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.121874f, 0.0f, -0.992624f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);

	glm::mat4 const m1rot = glm::extractMatrixRotation(m1);
	glm::mat4 const dltRotation = m2 * glm::transpose(m1rot);

	glm::vec3 dltAxis(0.0f);
	float dltAngle = 0.0f;
	glm::axisAngle(dltRotation, dltAxis, dltAngle);

	std::cout << "dltAxis: (" << dltAxis.x << ", " << dltAxis.y << ", " << dltAxis.z << "), dltAngle: " << dltAngle << std::endl;

	glm::quat q = glm::quat_cast(dltRotation);
	std::cout << "q: (" << q.x << ", " << q.y << ", " << q.z << ", " << q.w << ")" << std::endl;
	float yaw = glm::yaw(q);
	std::cout << "Yaw: " << yaw << std::endl;

	return Error;
}

static int test_rotate()
{
	glm::mat4 m2(1.0);
	float myAngle = 1.0f;
	m2 = glm::rotate(m2, myAngle, glm::vec3(1.0f, 0.0f, 0.0f));
	glm::vec3 m2Axis;
	float m2Angle;
	glm::axisAngle(m2, m2Axis, m2Angle);

	return 0;
}

int main()
{
	int Error = 0;

	Error += test_axisAngle();
	Error += test_rotate();

	return Error;
}


