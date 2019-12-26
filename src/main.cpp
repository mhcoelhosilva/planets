#include "loadTex.hpp"
#include "loadOBJ.hpp"
#include "planet.hpp"
#include "loadShaders.hpp"
#include "text2D.hpp"

#include <chrono>

using namespace std;

double offset = 0.0;

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

glm::vec3 cameraPos = { 0, 0, 270 };
glm::vec3 cameraLookAt = { 0, 0, 0 };

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	}
	else if (key == GLFW_KEY_A && action == GLFW_REPEAT)
	{
		cameraPos.x = cameraPos.x - 5.0f;
		cameraLookAt.x = cameraLookAt.x - 5.0f;
	}
	else if (key == GLFW_KEY_D && action == GLFW_REPEAT)
	{
		cameraPos.x = cameraPos.x + 5.0f;
		cameraLookAt.x = cameraLookAt.x + 5.0f;
	}
	else if (key == GLFW_KEY_W && action == GLFW_REPEAT)
	{
		cameraPos.z = cameraPos.z - 5.0f;
	}
	else if (key == GLFW_KEY_S && action == GLFW_REPEAT)
	{
		cameraPos.z = cameraPos.z + 5.0f;
	}
	else if (key == GLFW_KEY_UP && action == GLFW_REPEAT)
	{
		Planet::scaleConst = Planet::scaleConst + 5.0f;
	}
	else if (key == GLFW_KEY_DOWN && action == GLFW_REPEAT)
	{
		Planet::scaleConst = Planet::scaleConst - 5.0f;
	}
}

int main(void)
{
    //Declare window/context
    GLFWwindow* window;
    //Declare handlers for vertex array, vertex buffer, shaders, and program
    GLuint vertex_shader, fragment_shader, program;

    //Set error callback function
    glfwSetErrorCallback(error_callback);

    //Initialize glfw and set minimum version
//    glewExperimental = true; // Needed for core profile
    if (!glfwInit())
      {
        exit(EXIT_FAILURE);
      }

    glfwWindowHint(GLFW_SAMPLES, 4); // 4x antialiasing
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // We want OpenGL 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
//    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
//    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // We don't want the old OpenGL

    //Open window/context
    window = glfwCreateWindow(1024, 768, "Planets", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    //cout << glGetError() << endl;
    //cout << gluErrorString(glGetError()) << endl;

    //Set key callback function
    glfwSetKeyCallback(window, key_callback);

    //Set the created context as current
    glfwMakeContextCurrent(window);

    glewExperimental=true; // Needed in core profile
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        return -1;
    }

    //cout << glGetError() << endl;
    //cout << gluErrorString(glGetError()) << endl;

    /* GLFW windows by default use double buffering. That means that each window
     * has two rendering buffers; a front buffer and a back buffer. The front buffer
     * is the one being displayed and the back buffer the one you render to.
     *
     * When the entire frame has been rendered, the buffers need to be swapped with
     * one another, so the back buffer becomes the front buffer and vice versa.
     *
     * The swap interval indicates how many frames to wait until swapping the buffers,
     * commonly known as vsync. By default, the swap interval is zero, meaning buffer
     * swapping will occur immediately. On fast machines, many of those frames will
     * never be seen, as the screen is still only updated typically 60-75 times per second,
     * so this wastes a lot of CPU and GPU cycles.
     */
    glfwSwapInterval(1);

    // Read our .obj file
    std::vector< glm::vec3 > vertices;
    std::vector< glm::vec2 > uvs;
    std::vector< glm::vec3 > normals;
    bool res = loadOBJ("../assets/sphere.obj", vertices, uvs, normals);

    //cout << "vertices.size() = " << vertices.size() << endl;

    //Position vertex data
	GLuint pos_buffer;
    glGenBuffers(1, &pos_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, pos_buffer);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                          0, (void*) 0);

    //UVs
    GLuint uv_buffer;
    glGenBuffers(1, &uv_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, uv_buffer);
    glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
                          0, (void*) 0);

	//Normals
	GLuint normalbuffer;
	glGenBuffers(1, &normalbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
	glVertexAttribPointer(
		2,                                // attribute
		3,                                // size
		GL_FLOAT,                         // type
		GL_FALSE,                         // normalized?
		0,                                // stride
		(void*)0                          // array buffer offset
	);

    //Texture vertex data:
    glEnable(GL_TEXTURE_2D);
	//Set active texture unit (for shader)
	//The main purpose of texture units is to allow us to use more than 1 texture in our shaders.
	//By assigning texture units to the samplers, we can bind to multiple textures at once as long
	//as we activate the corresponding texture unit first.
    glActiveTexture(GL_TEXTURE0);
	//Load planet textures
	vector<Planet> planets;
	for (int i = 0; i < 9; ++i)
	{
		planets.emplace_back(Planet(i));
	}

	//Load, compile, and link standard shaders
	program = loadShaders("../src/standardShader.vertexshader", "../src/standardShader.fragmentshader");
    glUseProgram(program);

	// Initialize text library with the Holstein font
	initText2D("../assets/Holstein.tga");

    //Only at init:
    //Get handles to MatrixViewProjection matrix, and Pos and Color attributes
	GLint mvp_location = glGetUniformLocation(program, "MVP");
	GLint m_location = glGetUniformLocation(program, "M");
	GLint v_location = glGetUniformLocation(program, "V");
    //We also have to tell OpenGL to which texture unit each shader sampler belongs to by setting each sampler
    glUniform1i(glGetUniformLocation(program, "myTextureSampler"), 0);

	chrono::system_clock::time_point oldTime = chrono::system_clock::now();
	chrono::system_clock::time_point FPSTime = oldTime;
	int nbFrames = 0;
	char FPStext[256] = "N/A ms/frame";
    //==Main loop==//
    while (!glfwWindowShouldClose(window))
    {
		chrono::system_clock::time_point newTime = chrono::system_clock::now();
		chrono::system_clock::duration deltaTime = newTime - oldTime;
		oldTime = newTime;

		glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
        //First clear the screen.
        //This will change the background color to dark blue because of the previous glClearColor(0.0f, 0.0f, 0.4f, 0.0f) call:
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(program);

        // Enable depth test
        glEnable(GL_DEPTH_TEST);
        // Accept fragment if it closer to the camera than the former one
        glDepthFunc(GL_LESS);

        //Enable culling
        glEnable(GL_CULL_FACE);
        //Cull back-facing triangles
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);

        //Screen specific variables for projection:
		float ratio;
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        ratio = width / (float) height;
        glViewport(0, 0, width, height);

        // Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
        glm::mat4 Projection = glm::perspective(glm::radians(45.0f), (float) width / (float)height, 0.1f, 50000.0f);

        // View (camera) matrix
        glm::mat4 View = glm::lookAt(
            cameraPos, // Camera is at (5,3,3), in World Space
            cameraLookAt, // and looks at the origin
            glm::vec3(0,1,0)  // Head is up (set to 0,-1,0 to look upside-down)
            );

		//Enable attribute buffers
		//Position vertex data
		glBindBuffer(GL_ARRAY_BUFFER, pos_buffer);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
			0, (void*)0);
		//UVs
		glBindBuffer(GL_ARRAY_BUFFER, uv_buffer);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
			0, (void*)0);
		//Normals
		glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
		glVertexAttribPointer(
			2,                                // attribute
			3,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
		);
		//For each of the planets
		for (unsigned int i = 0; i < planets.size(); ++i)
		{
			//Bind texture
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, planets[i].texID);

			//Update planet location
			planets[i].updateLocation(chrono::duration_cast<chrono::milliseconds>(deltaTime).count());
			//planets[i].updateScale();

			// Our ModelViewProjection : multiplication of our 3 matrices
			glm::mat4 mvp = Projection * View * planets[i].model; // Remember, matrix multiplication is the other way around

			// Send our transformation to the currently bound shader, in the "MVP" uniform
			// This is done in the main loop since each model will have a different MVP matrix (At least for the M part)
			glUniformMatrix4fv(mvp_location, 1, GL_FALSE, &mvp[0][0]);
			glUniformMatrix4fv(m_location, 1, GL_FALSE, &planets[i].model[0][0]);
			glUniformMatrix4fv(v_location, 1, GL_FALSE, &View[0][0]);

			glDrawArrays(GL_TRIANGLES, 0, vertices.size() * sizeof(glm::vec3)); // Starting from vertex 0; 3 vertices = 1 triangle

		}

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);

		//FPS counter
		nbFrames++;
		if (chrono::duration_cast<std::chrono::milliseconds>(newTime - FPSTime).count() > 1000)
		{
			sprintf(FPStext, "%f ms/frame", 1000.0 / double(nbFrames));
			nbFrames = 0;
			FPSTime += chrono::seconds{ 1 };
		}
		printText2D(FPStext, 10, 10, 17);

		glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwDestroyWindow(window);
	// Delete the text's VBO, the shader and the texture
	cleanupText2D();
    glfwTerminate();
    exit(EXIT_SUCCESS);
}
