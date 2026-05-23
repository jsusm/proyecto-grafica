#include "engine2D.h"

Engine2D::Engine2D(int width, int height, const std::string& title) : width(width), height(height), title(title) {
	pixelBuffer.resize(width * height, Color(0.0f, 0.0f, 0.0f));
	for (int i = 0; i < GLFW_KEY_LAST; ++i) keyState[i] = false;
	for (int i = 0; i < GLFW_MOUSE_BUTTON_LAST; ++i) mouseButtonState[i] = false;
	init();
	setupCanvas();
}

Engine2D::~Engine2D() {
	glDeleteTextures(1, &textureID);
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);
	glDeleteBuffers(1, &ebo);
	glDeleteProgram(shaderProgram);
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	glfwDestroyWindow(window);
	glfwTerminate();
}

void Engine2D::init() {
	if (!glfwInit()) {
		std::cout << "Failed to initialize GLFW\n";
		return;
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
	if (!window) {
		glfwTerminate();
		std::cout << "Failed to create GLFW window\n";
		return;
	}
	glfwMakeContextCurrent(window);
	if (!gladLoadGL(glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD\n";
		glfwDestroyWindow(window);
		glfwTerminate();
		return;
	}
	glViewport(0, 0, width, height);
	glfwSetWindowUserPointer(window, this);
	glfwSetKeyCallback(window, keyCallback);
	glfwSetMouseButtonCallback(window, mouseButtonCallback);
	glfwSetCursorPosCallback(window, cursorPosCallback);
	// Setup ImGui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330 core");
	return;
};

void Engine2D::setupCanvas() {
	float vertex[] = {
		 1.0f,  1.0f,  1.0f, 1.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,
		-1.0f, -1.0f,  0.0f, 0.0f,
		-1.0f,  1.0f,  0.0f, 1.0f
	};
	unsigned int index[] = { 0, 1, 3, 1, 2, 3 };

	glGenTextures(1, &textureID);
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &ebo);
	vs = glCreateShader(GL_VERTEX_SHADER);
	fs = glCreateShader(GL_FRAGMENT_SHADER);
	shaderProgram = glCreateProgram();

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex), vertex, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index), index, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);
	//Shaders
	glShaderSource(vs, 1, &vertexShaderSource, nullptr);
	glCompileShader(vs);
	glShaderSource(fs, 1, &fragmentShaderSource, nullptr);
	glCompileShader(fs);
	glAttachShader(shaderProgram, vs);
	glAttachShader(shaderProgram, fs);
	glLinkProgram(shaderProgram);
	glDeleteShader(vs);
	glDeleteShader(fs);
	//Texturas
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_FLOAT, nullptr);
	return;
}

void Engine2D::uploadTexture() {
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGB, GL_FLOAT, pixelBuffer.data());
	return;
}

void Engine2D::run() {
	setup();
	while (!glfwWindowShouldClose(window)) {
		float currentFrame = glfwGetTime();
		static float lastFrame = 0.0f;
		float deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		glfwPollEvents();
		update(deltaTime);
		uploadTexture();
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glUseProgram(shaderProgram);
		glBindVertexArray(vao);
		glBindTexture(GL_TEXTURE_2D, textureID);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		drawUI();
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		glfwSwapBuffers(window);
	}
	return;
}

void Engine2D::putPixel(int x, int y, const Color& color) {
	if (x < 0 || x >= width || y < 0 || y >= height) return;
	pixelBuffer[y * width + x] = color;
	return;
}

void Engine2D::clear(const Color& color) {
	std::fill(pixelBuffer.begin(), pixelBuffer.end(), color);
	return;
}

glm::vec2 Engine2D::getMousePosition() {
	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);
	return glm::vec2(static_cast<float>(xpos), static_cast<float>(ypos));
}

void Engine2D::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key < 0 || key >= GLFW_KEY_LAST) return;
	Engine2D* engine = static_cast<Engine2D*>(glfwGetWindowUserPointer(window));
	if (action == GLFW_PRESS) {
		engine->keyState[key] = true;
		engine->onkeyDown(key);
	}
	else if (action == GLFW_RELEASE) {
		engine->keyState[key] = false;
		engine->onkeyUp(key);
	}
}

void Engine2D::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
	if (button < 0 || button >= GLFW_MOUSE_BUTTON_LAST) return;
	Engine2D* engine = static_cast<Engine2D*>(glfwGetWindowUserPointer(window));
	glm::vec2 mousePos = engine->getMousePosition();
	if (action == GLFW_PRESS) {
		engine->mouseButtonState[button] = true;
		engine->onMouseButtonDown(button, mousePos.x, mousePos.y);
	}
	else if (action == GLFW_RELEASE) {
		engine->mouseButtonState[button] = false;
		engine->onMouseButtonUp(button, mousePos.x, mousePos.y);
	}
}

void Engine2D::cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
	Engine2D* engine = static_cast<Engine2D*>(glfwGetWindowUserPointer(window));
	if (engine) {
		engine->onMouseMove(xpos, ypos);
	}
}

bool Engine2D::isKeyPressed(int key) const {
	if (key < 0 || key >= GLFW_KEY_LAST) return false;
	return keyState[key];
}

bool Engine2D::isMouseButtonPressed(int button) const {
	if (button < 0 || button >= GLFW_MOUSE_BUTTON_LAST) return false;
	return mouseButtonState[button];
}