#include "loadTex.hpp"
#include "loadOBJ.hpp"
#include "planet.hpp"

#include <chrono>

using namespace std;

double offset = 0.0;

//In vertex shader: transforming vertex positions as position = MVP * position (in model space)
static const char* vertex_shader_text =
"#version 330 core\n"
"uniform mat4 MVP;\n"
"uniform mat4 M;\n"
"uniform mat4 V;\n"
"layout(location = 0) in vec3 vertexPosition_modelspace;\n"
"layout(location = 1) in vec2 vertexUV;\n"
"layout(location = 2) in vec3 vertexNormal_modelspace;\n"
"out vec2 UV;\n"
"out vec4 texCoords;\n"
"out vec3 Normal_cameraspace;\n"
"out vec3 LightDirection_cameraspace;\n"
"out vec3 EyeDirection_cameraspace;\n"
"void main()\n"
"{\n"
"    gl_Position = MVP * vec4(vertexPosition_modelspace, 1.0);\n"
"    UV = vec2(vertexUV.x, 1.0 - vertexUV.y);\n"
"    texCoords = vec4(vertexPosition_modelspace, 1.0);\n"
// Position of the vertex, in worldspace : M * position
"	 vec3 Position_worldspace = (M * vec4(vertexPosition_modelspace, 1)).xyz;\n"
// Vector that goes from the vertex to the camera, in camera space.
// In camera space, the camera is at the origin (0,0,0).
"	 vec3 vertexPosition_cameraspace = (V * M * vec4(vertexPosition_modelspace, 1)).xyz; \n"
"	 vec3 EyeDirection_cameraspace = vec3(0, 0, 0) - vertexPosition_cameraspace; \n"
// Vector that goes from the vertex to the light, in camera space. M is ommited because it's identity.
"	 vec3 LightPosition_worldspace = vec3(0, 0, 68);\n"	
"	 vec3 LightPosition_cameraspace = (V * vec4(LightPosition_worldspace, 1)).xyz;\n"
"	 LightDirection_cameraspace = LightPosition_cameraspace + EyeDirection_cameraspace;\n"
// Normal of the the vertex, in camera space
"	 Normal_cameraspace = (V * M * vec4(vertexNormal_modelspace, 0)).xyz;\n" // Only correct if ModelMatrix does not scale the model ! Use its inverse transpose if not.
"}\n";

//In fragment shader:
static const char* fragment_shader_text =
"#version 330 core\n"
"in vec2 UV;\n"
"in vec4 texCoords;\n"
"uniform sampler2D myTextureSampler;\n"
"in vec3 Normal_cameraspace;\n"
"in vec3 LightDirection_cameraspace;\n"
"in vec3 EyeDirection_cameraspace;\n"
"out vec4 color;\n"
"void main()\n"
"{\n"
"    vec2 longitudeLatitude = vec2((atan(texCoords.x, texCoords.z) / 3.1415926 + 1.0) * 0.5, (asin(texCoords.y / (sqrt(texCoords.x * texCoords.x + texCoords.y * texCoords.y + texCoords.z * texCoords.z)))) / 3.1415926 + 0.5);\n"
"    vec4 materialDiffuseColor = texture( myTextureSampler, longitudeLatitude);\n"
// Normal of the computed fragment, in camera space
"    vec3 n = normalize(Normal_cameraspace);\n"
// Direction of the light (from the fragment to the light)
"	 vec3 l = normalize(LightDirection_cameraspace);\n"
"	 float cosTheta = clamp( dot( n,l ), 0,1 );\n"
"	 vec4 lightColor = vec4(1.0, 0.90, 0.85, 1.0);\n"
"	 float lightPower = 1500.0;\n"
"	 float distance = length(LightDirection_cameraspace);\n"
"	 vec4 materialAmbientColor = vec4(0.37,0.37,0.37,1.0) * materialDiffuseColor;"
// Eye vector (towards the camera)
"    vec3 E = normalize(EyeDirection_cameraspace);\n"
// Direction in which the triangle reflects the light
"    vec3 R = reflect(-l, n);\n"
// Cosine of the angle between the Eye vector and the Reflect vector,
// clamped to 0
//  - Looking into the reflection -> 1
//  - Looking elsewhere -> < 1
"    float cosAlpha = clamp(dot(E, R), 0, 1);\n"
"	 color = materialAmbientColor + materialDiffuseColor * lightColor * lightPower * cosTheta / (distance*distance) + materialDiffuseColor * lightColor * lightPower * pow(cosAlpha,5) / (distance*distance);\n"
"}\n";


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

    //Variables for checking shader compilation
    GLint Result = GL_FALSE;
    int InfoLogLength;

    //Compile vertex shader defined as text above to GL object and compile
    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_text, NULL);
    glCompileShader(vertex_shader);

	cout << "Compiling vertex shader" << endl;
    cout << glGetError() << endl;
    cout << gluErrorString(glGetError()) << endl;

    // Check Vertex Shader
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(vertex_shader, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if ( InfoLogLength > 0 )
      {
	std::vector<char> VertexShaderErrorMessage(InfoLogLength+1);
	glGetShaderInfoLog(vertex_shader, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
	printf("%s\n", &VertexShaderErrorMessage[0]);
      }

    cout << glGetError() << endl;
    cout << gluErrorString(glGetError()) << endl;

    //Compile fragment shader defined as text above to GL object and compile
    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_text, NULL);
    glCompileShader(fragment_shader);

	cout << "Compiling fragment shader" << endl;

    cout << glGetError() << endl;
    cout << gluErrorString(glGetError()) << endl;

    // Check Fragment Shader
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(fragment_shader, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if ( InfoLogLength > 0 )
      {
    	std::vector<char> FragmentShaderErrorMessage(InfoLogLength+1);
    	glGetShaderInfoLog(fragment_shader, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
    	printf("%s\n", &FragmentShaderErrorMessage[0]);
      }

    cout << glGetError() << endl;
    cout << gluErrorString(glGetError()) << endl;

    //Create program handle and attach shaders defined above
    program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);
    glUseProgram(program);

    //Only at init:
    //Get handles to MatrixViewProjection matrix, and Pos and Color attributes
	GLint mvp_location = glGetUniformLocation(program, "MVP");
	GLint m_location = glGetUniformLocation(program, "M");
	GLint v_location = glGetUniformLocation(program, "V");
    //We also have to tell OpenGL to which texture unit each shader sampler belongs to by setting each sampler
    glUniform1i(glGetUniformLocation(program, "myTextureSampler"), 0);

	chrono::system_clock::time_point oldTime = chrono::system_clock::now();
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
		glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}
