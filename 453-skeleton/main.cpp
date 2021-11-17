/*
 *Assignment 4: Virtual Orrery and Real Time Rendering
 *@author Manisha Gupta
 *@version 1.0
 *
 *Course: CPSC 453 L01 F2020
 *Instructor: Faramarz Famil Samavati
 *
 * Utilized skeleton/boilerplate code provided on D2L for CPSC 453
 */

#define _USE_MATH_DEFINES
#include <math.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <string>
#include <list>
#include <vector>
#include <limits>
#include <functional>

#include "Geometry.h"
#include "GLDebug.h"
#include "Log.h"
#include "ShaderProgram.h"
#include "Shader.h"
#include "Texture.h"
#include "Window.h"
#include "Camera.h"

#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"


// We gave this code in one of the tutorials, so leaving it here too
void updateGPUGeometry(GPU_Geometry &gpuGeom, CPU_Geometry const &cpuGeom) {
	gpuGeom.bind();
	gpuGeom.setVerts(cpuGeom.verts);
	gpuGeom.setTexCoords(cpuGeom.texCoords);
	gpuGeom.setNormals(cpuGeom.normals);
}

struct celestialObject {
	celestialObject(std::string texturePath, GLenum textureInterpolation) :
		texture(texturePath, textureInterpolation),
		trMatrix(0.0f)
	{}

	CPU_Geometry cgeom;
	GPU_Geometry ggeom;
	Texture texture;
	glm::mat4 trMatrix;
};

// EXAMPLE CALLBACKS
class Assignment4 : public CallbackInterface {

public:
	Assignment4() : camera(0.0, 0.0, 2.0), aspect(1.0f),
		go(false),
		pause(false),
		reset(false)
	{}

	virtual void keyCallback(int key, int scancode, int action, int mods) {
		if (key == GLFW_KEY_0 && action == GLFW_PRESS) {
			go = true;
		}
		if (key == GLFW_KEY_1 && action == GLFW_PRESS) {
			pause = true;
		}
		if (key == GLFW_KEY_2 && action == GLFW_PRESS) {
			reset = true;
		}
	}
	virtual void mouseButtonCallback(int button, int action, int mods) {
		if (button == GLFW_MOUSE_BUTTON_RIGHT) {
			if (action == GLFW_PRESS) {
				rightMouseDown = true;
			} else if (action == GLFW_RELEASE) {
				rightMouseDown = false;
			}
		}
	}
	virtual void cursorPosCallback(double xpos, double ypos) {
		if (rightMouseDown) {
			double dx = xpos - mouseOldX;
			double dy = ypos - mouseOldY;
			camera.incrementTheta(dy);
			camera.incrementPhi(dx);
		}
		mouseOldX = xpos;
		mouseOldY = ypos;
	}
	virtual void scrollCallback(double xoffset, double yoffset) {
		camera.incrementR(yoffset);
	}
	virtual void windowSizeCallback(int width, int height) {
		// The CallbackInterface::windowSizeCallback will call glViewport for us
		CallbackInterface::windowSizeCallback(width,  height);
		aspect = float(width)/float(height);
	}

	void refreshStatus() {
		go = false;
		pause = false;
		reset = false;
	}

	bool buttonDown(int button) {
		if (button == GLFW_KEY_0) return go;
		else if (button == GLFW_KEY_1) return pause;
		else if (button == GLFW_KEY_2) return reset;

		return false;
	}

	void viewPipeline(ShaderProgram &sp, glm::mat4& trMatrix, int& doLighting) {
		glm::mat4 M = trMatrix;
		glm::mat4 V = camera.getView();
		glm::mat4 P = glm::perspective(glm::radians(45.0f), aspect, 0.01f, 1000.f);

		GLint location = glGetUniformLocation(sp, "lightPos");
		glm::vec3 lightPos = glm::vec3(0.0f, 0.0f, 0.0f); 
		glUniform3fv(location, 1, glm::value_ptr(lightPos));

		location = glGetUniformLocation(sp, "shade");
		int shade = doLighting;
		glUniform1i(location, shade);

		GLint uniMat = glGetUniformLocation(sp, "M");
		glUniformMatrix4fv(uniMat, 1, GL_FALSE, glm::value_ptr(M));
		uniMat = glGetUniformLocation(sp, "V");
		glUniformMatrix4fv(uniMat, 1, GL_FALSE, glm::value_ptr(V));
		uniMat = glGetUniformLocation(sp, "P");
		glUniformMatrix4fv(uniMat, 1, GL_FALSE, glm::value_ptr(P));
	}

	Camera camera;

private:

	bool rightMouseDown = false;
	float aspect;
	double mouseOldX;
	double mouseOldY;
	bool go;
	bool pause;
	bool reset;

};

CPU_Geometry unitSphere() {
	CPU_Geometry sphere;
	float radius = 1.0f;
	float horDiv = 72.0f;														
	float verDiv = horDiv / 2.0f;													

	for (float i = 0.0f; i <= verDiv; i++) {
		float theta = M_PI / 2 - i * (M_PI / verDiv);    
		float r = radius * cos(theta);					 
		float z = radius * sin(theta);					 

		for (float j = 0.0f; j <= horDiv; j++) {
			float phi = j * (2 * M_PI / horDiv);       

			float x = r * cos(phi);						
			float y = r * sin(phi);						
			sphere.verts.push_back(glm::vec3(x, y, z));

			float nx = x * (1.0f / radius);
			float ny = y * (1.0f / radius);
			float nz = z * (1.0f / radius);
			sphere.normals.push_back(glm::vec3(nx, ny, nz));

			float s = j / horDiv;
			float t = i / verDiv;
			sphere.texCoords.push_back(glm::vec2(s, t));
		}
	}

	CPU_Geometry temp = sphere;
	sphere.verts.clear();
	sphere.normals.clear();
	sphere.texCoords.clear();

	for (int i = 0; i < verDiv; i++)
	{
		int curr = i * (horDiv + 1);     
		int currAbove = curr + horDiv + 1;      

		for (int j = 0; j < horDiv; j++, curr++, currAbove++) {
			if (i != 0) {
				sphere.verts.push_back(temp.verts[curr]);
				sphere.verts.push_back(temp.verts[currAbove]);
				sphere.verts.push_back(temp.verts[curr + 1]);

				sphere.normals.push_back(temp.normals[curr]);
				sphere.normals.push_back(temp.normals[currAbove]);
				sphere.normals.push_back(temp.normals[curr + 1]);

				sphere.texCoords.push_back(temp.texCoords[curr]);
				sphere.texCoords.push_back(temp.texCoords[currAbove]);
				sphere.texCoords.push_back(temp.texCoords[curr + 1]);
			}

			if (i != (verDiv - 1)) {
				sphere.verts.push_back(temp.verts[curr + 1]);
				sphere.verts.push_back(temp.verts[currAbove]);
				sphere.verts.push_back(temp.verts[currAbove + 1]);

				sphere.normals.push_back(temp.normals[curr + 1]);
				sphere.normals.push_back(temp.normals[currAbove]);
				sphere.normals.push_back(temp.normals[currAbove + 1]);

				sphere.texCoords.push_back(temp.texCoords[curr + 1]);
				sphere.texCoords.push_back(temp.texCoords[currAbove]);
				sphere.texCoords.push_back(temp.texCoords[currAbove + 1]);
			}
		}
	}
	return sphere;
}

void setUp(celestialObject& space, celestialObject& earth, celestialObject& sun, celestialObject& moon) {
	space.trMatrix = glm::mat4(
		3.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 3.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 3.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	) * glm::mat4(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, cos(M_PI / 2), -sin(M_PI / 2), 0.0f,
		0.0f, sin(M_PI / 2), cos(M_PI / 2), 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);

	sun.trMatrix = glm::mat4(
		0.15f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.15f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.15f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	); 
}

void rotations(celestialObject& earth, celestialObject& sun, celestialObject& moon, float thetaPrev, float thetaCurr) {
	sun.trMatrix = glm::rotate(sun.trMatrix, 0.0001f, glm::vec3(0.0f, 1.0f, 0.0f));

	float thetaAxis = glm::radians(5.4f);
	float thetaOrb = glm::radians(5.4f);

	glm::mat4 axis = glm::mat4(1.0f);
	//axis = glm::rotate(axis, thetaOrb, glm::vec3(0.0f, 0.0f, 1.0f));

	//earth
	glm::mat4 earthOrb = glm::mat4(
		cos(-thetaCurr * 0.2f), 0.0f, sin(-thetaCurr * 0.2f), 0.0f,
		0.0, 1.0f, 0.0f, 0.0f,
		-sin(-thetaCurr * 0.2f), 0.0f, cos(-thetaCurr * 0.2f), 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	) * glm::mat4(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.0f, 0.0f, 1.0f
	) * glm::mat4(
		0.07f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.07f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.07f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);

	glm::vec3 earth_pos = earthOrb * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

	glm::mat4 earthAxis = glm::mat4(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		earth_pos.x, earth_pos.y, earth_pos.z, 1.0f
	) * glm::mat4(
		cos(-thetaCurr * 1.5f), 0.0f, sin(-thetaCurr * 1.5f), 0.0f,
		0.0, 1.0f, 0.0f, 0.0f,
		-sin(-thetaCurr * 1.5f), 0.0f, cos(-thetaCurr * 1.5f), 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	) * glm::mat4(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		-earth_pos.x, -earth_pos.y, -earth_pos.z, 1.0f
	);

	earthAxis = glm::rotate(earthAxis, thetaAxis, glm::vec3(0.0f, 0.0f, 1.0f));

	earth.trMatrix = earthAxis * earthOrb;

	//moon
	earth_pos = earth.trMatrix * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	
	glm::mat4 moonOrb = glm::mat4(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		earth_pos.x, earth_pos.y, earth_pos.z, 1.0f
	) * glm::mat4(
		cos(-thetaCurr * 0.9f), 0.0f, sin(-thetaCurr * 0.9f), 0.0f,
		0.0, 1.0f, 0.0f, 0.0f,
		-sin(-thetaCurr * 0.9f), 0.0f, cos(-thetaCurr * 0.9f), 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	) * glm::mat4(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.15f, 0.0f, 0.0f, 1.0f
	) * glm::mat4(
		0.03f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.03f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.03f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);

	glm::vec3 moon_pos = moonOrb * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

	glm::mat4 moonAxis = glm::mat4(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		moon_pos.x, moon_pos.y, moon_pos.z, 1.0f
	) * glm::mat4(
		cos(-thetaCurr * 1.2f), 0.0f, sin(-thetaCurr * 1.2f), 0.0f,
		0.0, 1.0f, 0.0f, 0.0f,
		-sin(-thetaCurr * 1.2f), 0.0f, cos(-thetaCurr * 1.2f), 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	) * glm::mat4(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		-moon_pos.x, -moon_pos.y, -moon_pos.z, 1.0f
	);

	moonAxis = glm::rotate(moonAxis, thetaAxis, glm::vec3(0.0f, 0.0f, 1.0f));

	moon.trMatrix = moonAxis * moonOrb;
	
}

int main() {
	Log::debug("Starting main");

	// WINDOW
	glfwInit();
	Window window(800, 800, "CPSC 453"); // can set callbacks at construction if desired


	GLDebug::enable();

	// CALLBACKS
	auto a4 = std::make_shared<Assignment4>();
	window.setCallbacks(a4);


	ShaderProgram shader("shaders/test.vert", "shaders/test.frag");

	celestialObject space("textures/stars_milky_way.jpg", GL_LINEAR);
	celestialObject earth("textures/earth_nightmap.jpg", GL_LINEAR);
	celestialObject sun("textures/sun.jpg", GL_LINEAR);
	celestialObject moon("textures/moon.jpg", GL_LINEAR);

	space.cgeom = unitSphere();
	earth.cgeom = unitSphere();
	sun.cgeom = unitSphere();
	moon.cgeom = unitSphere();

	updateGPUGeometry(space.ggeom, space.cgeom);
	updateGPUGeometry(earth.ggeom, earth.cgeom);
	updateGPUGeometry(sun.ggeom, sun.cgeom);
	updateGPUGeometry(moon.ggeom, moon.cgeom);

	setUp(space, earth, sun, moon);
	float timePrev = (float)glfwGetTime();
	float timeCurr;
	float thetaPrev;
	float thetaCurr = 0;
	int doLighting;
	int animation = 0; // 0 = regular; 1 = paused; 2 = restart

	// RENDER LOOP
	while (!window.shouldClose()) {
		a4->refreshStatus();
		glfwPollEvents();

		if (a4->buttonDown(GLFW_KEY_0)) {
			animation = 0;
		}
		else if (a4->buttonDown(GLFW_KEY_1)) {
			animation = 1;
		}
		else if (a4->buttonDown(GLFW_KEY_2)) {
			animation = 2;
		}

		if (animation == 0) {
			thetaPrev = thetaCurr;
			timeCurr = glfwGetTime();
			thetaCurr = (timeCurr - timePrev) + thetaCurr;
			timePrev = timeCurr;

			rotations(earth, sun, moon, thetaPrev, thetaCurr);
		}
		else if (animation == 1) {
			thetaPrev = thetaCurr;
			timeCurr = glfwGetTime();
			timePrev = timeCurr;
		}
		else if (animation == 2) {
			thetaCurr = 0;
			timePrev = (float)glfwGetTime();
			animation = 0;
		}
		

		glEnable(GL_LINE_SMOOTH);
		glEnable(GL_FRAMEBUFFER_SRGB);
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);

		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		shader.use();
		
		space.ggeom.bind();
		space.texture.bind();
		doLighting = 0;
		a4->viewPipeline(shader, space.trMatrix, doLighting);
		glDrawArrays(GL_TRIANGLES, 0, GLsizei(space.cgeom.verts.size()));
		space.texture.unbind();
		
		earth.ggeom.bind();
		earth.texture.bind();
		doLighting = 1;
		a4->viewPipeline(shader, earth.trMatrix, doLighting);
		glDrawArrays(GL_TRIANGLES, 0, GLsizei(earth.cgeom.verts.size()));
		earth.texture.unbind();
		
		sun.ggeom.bind();
		sun.texture.bind();
		doLighting = 0;
		a4->viewPipeline(shader, sun.trMatrix, doLighting);
		glDrawArrays(GL_TRIANGLES, 0, GLsizei(sun.cgeom.verts.size()));
		sun.texture.unbind();
		
		moon.ggeom.bind();
		moon.texture.bind();
		doLighting = 1;
		a4->viewPipeline(shader, moon.trMatrix, doLighting);
		glDrawArrays(GL_TRIANGLES, 0, GLsizei(moon.cgeom.verts.size()));
		moon.texture.unbind();
		
		glDisable(GL_FRAMEBUFFER_SRGB); // disable sRGB for things like imgui

		window.swapBuffers();
	}

	glfwTerminate();
	return 0;
}
