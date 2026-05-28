#pragma once
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <vector>
#include <string>
#include <iostream>

struct Color 
{
	float r, g, b;
	Color() : r(0), g(0), b(0) {};
	Color(float r, float g, float b) : r(r), g(g), b(b) {};
};

class Engine2D
{
private:
	std::string title;
	std::vector<Color> pixelBuffer;
	GLuint textureID, vao,vbo, ebo, shaderProgram, vs, fs;
	void init();
	void setupCanvas();
	void uploadTexture();
	//Manejo de eventos
	bool keyState[GLFW_KEY_LAST];
	bool mouseButtonState[GLFW_MOUSE_BUTTON_LAST];
	static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
	static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
	static void cursorPosCallback(GLFWwindow* window, double xpos, double ypos);
protected:
	GLFWwindow* window;
	int width, height;
	void putPixel(int x, int y, const Color& color);
	void clear(const Color& color);
	glm::vec2 getMousePosition();
	bool isKeyPressed(int key) const;
	bool isMouseButtonPressed(int button) const;
public:
	Engine2D(int width, int height, const std::string& title);
	~Engine2D();
	void run();
	//API
	virtual void onkeyDown(int key) {};
	virtual void onkeyUp(int key) {};
	virtual void onMouseButtonDown(int button, double x, double y) {};
	virtual void onMouseButtonUp(int button, double x, double y) {};
	virtual void onMouseMove(double x, double y) {};
	virtual void setup() {};
	virtual void update(float deltaTime) {};
	virtual void drawUI() {};
private:
	const char* vertexShaderSource = R"(
		#version 330 core
		layout (location = 0) in vec2 aPos;
		layout (location = 1) in vec2 aTexCoord;
		out vec2 TexCoord;
		void main() {
			gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);
			// Invertimos el eje Y de la textura para que el (0,0) esté arriba a la izquierda
			TexCoord = vec2(aTexCoord.x, 1.0 - aTexCoord.y); 
		}
	)";
	const char* fragmentShaderSource = R"(
		#version 330 core
		out vec4 FragColor;
		in vec2 TexCoord;
		uniform sampler2D screenTexture;
		void main() {
			FragColor = texture(screenTexture, TexCoord);
		}
	)";
};
