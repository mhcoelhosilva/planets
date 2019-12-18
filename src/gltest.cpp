#include <stdio.h>

//First glad (needed?)
//#include <glad/gl.h>

#include <GL/glew.h>
#include <glfw3.h>

//GLM (add other headers which might be needed)
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

#include <math.h>
#include <stdlib.h>
#include <chrono>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <cstring>
//#include "linmath.h"
#include <Windows.h>

#include <SOIL.h>

#ifndef MAKEFOURCC
    #define MAKEFOURCC(ch0, ch1, ch2, ch3)                              \
                ((DWORD)(BYTE)(ch0) | ((DWORD)(BYTE)(ch1) << 8) |   \
                ((DWORD)(BYTE)(ch2) << 16) | ((DWORD)(BYTE)(ch3) << 24 ))
#endif //defined(MAKEFOURCC)

/*
 * FOURCC codes for DX compressed-texture pixel formats
 */
#define FOURCC_DXT1  (MAKEFOURCC('D','X','T','1'))
#define FOURCC_DXT2  (MAKEFOURCC('D','X','T','2'))
#define FOURCC_DXT3  (MAKEFOURCC('D','X','T','3'))
#define FOURCC_DXT4  (MAKEFOURCC('D','X','T','4'))
#define FOURCC_DXT5  (MAKEFOURCC('D','X','T','5'))

using namespace std;

#define pi 3.142857

double offset = 0.0;

//In vertex shader: transforming vertex positions as position = MVP * position (in model space)
static const char* vertex_shader_text =
"#version 330 core\n"
"uniform mat4 MVP;\n"
"layout(location = 0) in vec3 vertexPosition_modelspace;\n"
"layout(location = 1) in vec3 vertexUV;\n"
"out vec2 UV;\n"
"out vec4 texCoords;\n"
"void main()\n"
"{\n"
"    gl_Position = MVP * vec4(vertexPosition_modelspace, 1.0);\n"
"    UV = vec2(vertexUV.x, 1.0 - vertexUV.y);\n"
"    texCoords = vec4(vertexPosition_modelspace, 1.0);\n"
"}\n";

//In fragment shader:
static const char* fragment_shader_text =
"#version 330 core\n"
"in vec2 UV;\n"
"in vec4 texCoords;\n"
"uniform sampler2D myTextureSampler;\n"
"out vec4 color;\n"
"void main()\n"
"{\n"
"    vec2 longitudeLatitude = vec2((atan(texCoords.z, texCoords.x) / 3.1415926 + 1.0) * 0.5, (asin(texCoords.y / (sqrt(texCoords.x * texCoords.x + texCoords.y * texCoords.y + texCoords.z * texCoords.z)))) / 3.1415926 + 0.5);\n"
"    color = texture( myTextureSampler, longitudeLatitude);\n"
"}\n";


static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

GLuint loadTex(const char* filename)
  {
    GLuint tex_ID = SOIL_load_OGL_texture(filename, SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, (SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT | SOIL_FLAG_TEXTURE_REPEATS));
    //Just bind and do not create a new texture
    glBindTexture(GL_TEXTURE_2D, tex_ID);

    //Get texture width and height
    int width, height;
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);

    //Unbind Texture
    glBindTexture(GL_TEXTURE_2D, 0);

    return tex_ID;
  }

GLuint loadDDS(const char* path) {
  // lay out variables to be used
	unsigned char* header;

	unsigned int width;
	unsigned int height;
	unsigned int mipMapCount;

	unsigned int blockSize;
	unsigned int format;

	unsigned int offset = 0;
	unsigned int size = 0;

	unsigned int w;
	unsigned int h;

	unsigned char* buffer = 0;

	GLuint tid = 0;

  // open the DDS file for binary reading and get file size
	FILE* f;
	if((f = fopen(path, "rb")) == 0)
		return 0;
	fseek(f, 0, SEEK_END);
	long file_size = ftell(f);
	fseek(f, 0, SEEK_SET);

  // allocate new unsigned char space with 4 (file code) + 124 (header size) bytes
  // read in 128 bytes from the file
	header = (unsigned char*) malloc(128);
	fread(header, 1, 128, f);

  // compare the `DDS ` signature
	if(memcmp(header, "DDS ", 4) != 0)
		goto exit;

  // extract height, width, and amount of mipmaps - yes it is stored height then width
	height = (header[12]) | (header[13] << 8) | (header[14] << 16) | (header[15] << 24);
	width = (header[16]) | (header[17] << 8) | (header[18] << 16) | (header[19] << 24);
	mipMapCount = (header[28]) | (header[29] << 8) | (header[30] << 16) | (header[31] << 24);

  // figure out what format to use for what fourCC file type it is
  // block size is about physical chunk storage of compressed data in file (important)
	if(header[84] == 'D') {
		switch(header[87]) {
			case '1': // DXT1
			  {
				cout << "DX1" << endl;
				format = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
				blockSize = 8;
				break;
			  }
			case '3': // DXT3
			  {
				cout << "DX3" << endl;
				format = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
				blockSize = 16;
				break;
			  }
			case '5': // DXT5
			  {
				cout << "DX5" << endl;
				format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
				blockSize = 16;
				break;
			  }
			case '0': // DX10
			  {
				// unsupported, else will error
				// as it adds sizeof(struct DDS_HEADER_DXT10) between pixels
				// so, buffer = malloc((file_size - 128) - sizeof(struct DDS_HEADER_DXT10));
			  }
			default: goto exit;
		}
	} else // BC4U/BC4S/ATI2/BC55/R8G8_B8G8/G8R8_G8B8/UYVY-packed/YUY2-packed unsupported
		goto exit;

  // allocate new unsigned char space with file_size - (file_code + header_size) magnitude
  // read rest of file
	buffer = (unsigned char*) malloc(file_size - 128);
	if(buffer == 0)
		goto exit;
	fread(buffer, 1, file_size, f);

  // prepare new incomplete texture
	glGenTextures(1, &tid);
	if(tid == 0)
		goto exit;

  // bind the texture
  // make it complete by specifying all needed parameters and ensuring all mipmaps are filled
	glBindTexture(GL_TEXTURE_2D, tid);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, ((mipMapCount == 0) ? 0 : mipMapCount-1)); // opengl likes array length of mipmaps
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // don't forget to enable mipmaping
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); //GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); //GL_CLAMP_TO_EDGE);

    // prepare some variables
		w = width;
		h = height;

    // loop through sending block at a time with the magic formula
    // upload to opengl properly, note the offset transverses the pointer
    // assumes each mipmap is 1/2 the size of the previous mipmap
		for (unsigned int i=0; i<mipMapCount; i++) {
			if(w == 0 || h == 0) { // discard any odd mipmaps 0x1 0x2 resolutions
				mipMapCount--;
				continue;
			}
			size = ((w+3)/4) * ((h+3)/4) * blockSize;
			glCompressedTexImage2D(GL_TEXTURE_2D, i, format, w, h, 0, size, buffer + offset);
			offset += size;
			w /= 2;
			h /= 2;
		}
	    // discard any odd mipmaps, ensure a complete texture
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, mipMapCount-1);
    // unbind
	glBindTexture(GL_TEXTURE_2D, 0);

  // easy macro to get out quick and uniform (minus like 15 lines of bulk)
exit:
	free(buffer);
	free(header);
	fclose(f);
	return tid;
}

//GLuint loadDDS(const char * imagepath)
//{
//    unsigned char header[124];
//
//    FILE *fp;
//
//    /* try to open the file */
//    fp = fopen(imagepath, "rb");
//    if (fp == NULL)
//        return 0;
//
//    /* verify the type of file */
//    char filecode[4];
//    fread(filecode, 1, 4, fp);
//    if (strncmp(filecode, "DDS ", 4) != 0) {
//        fclose(fp);
//        return 0;
//    }
//
//    /* get the surface desc */
//    fread(&header, 124, 1, fp);
//
//    unsigned int height      = *(unsigned int*)&(header[8 ]);
//    unsigned int width         = *(unsigned int*)&(header[12]);
//    unsigned int linearSize     = *(unsigned int*)&(header[16]);
//    unsigned int mipMapCount = *(unsigned int*)&(header[24]);
//    unsigned int fourCC      = *(unsigned int*)&(header[80]);
//
//    unsigned char * buffer;
//    unsigned int bufsize;
//    /* how big is it going to be including all mipmaps? */
//    bufsize = mipMapCount > 1 ? linearSize * 2 : linearSize;
//    buffer = (unsigned char*)malloc(bufsize * sizeof(unsigned char));
//    fread(buffer, 1, bufsize, fp);
//    /* close the file pointer */
//    fclose(fp);
//
//    unsigned int components  = (fourCC == FOURCC_DXT1) ? 3 : 4;
//    unsigned int format;
//    switch(fourCC)
//    {
//    case FOURCC_DXT1:
//        format = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
//        break;
//    case FOURCC_DXT3:
//        format = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
//        break;
//    case FOURCC_DXT5:
//        format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
//        break;
//    default:
//        free(buffer);
//        return 0;
//    }
//
//    // Create one OpenGL texture
//    GLuint textureID;
//    glGenTextures(1, &textureID);
//
//    // "Bind" the newly created texture : all future texture functions will modify this texture
//    glBindTexture(GL_TEXTURE_2D, textureID);
//
//    unsigned int blockSize = (format == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT) ? 8 : 16;
//    unsigned int offset = 0;
//
//    /* load the mipmaps */
//    for (unsigned int level = 0; level < mipMapCount && (width || height); ++level)
//    {
//        unsigned int size = ((width+3)/4)*((height+3)/4)*blockSize;
//        glCompressedTexImage2D(GL_TEXTURE_2D, level, format, width, height,
//            0, size, buffer + offset);
//
//        offset += size;
//        width  /= 2;
//        height /= 2;
//    }
//    free(buffer);
//
//    return textureID;
//
//}

bool loadOBJ(
    const char * path,
    std::vector < glm::vec3 > & out_vertices,
    std::vector < glm::vec2 > & out_uvs,
    std::vector < glm::vec3 > & out_normals
)
{
  std::vector< unsigned int > vertexIndices, uvIndices, normalIndices;
  std::vector< glm::vec3 > temp_vertices;
  std::vector< glm::vec2 > temp_uvs;
  std::vector< glm::vec3 > temp_normals;

  FILE * file = fopen(path, "r");
  if( file == NULL ){
      printf("Impossible to open the file !\n");
      return false;
  }

  while( 1 ){

      char lineHeader[128];
      // read the first word of the line
      int res = fscanf(file, "%s", lineHeader);
      if (res == EOF)
          break; // EOF = End Of File. Quit the loop.

      // else : parse lineHeader
      if ( strcmp( lineHeader, "v" ) == 0 ){
          glm::vec3 vertex;
          fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z );
          temp_vertices.push_back(vertex);
      }else if ( strcmp( lineHeader, "vt" ) == 0 ){
          glm::vec2 uv;
          fscanf(file, "%f %f\n", &uv.x, &uv.y );
          temp_uvs.push_back(uv);
      }else if ( strcmp( lineHeader, "vn" ) == 0 ){
          glm::vec3 normal;
          fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z );
          temp_normals.push_back(normal);
      }else if ( strcmp( lineHeader, "f" ) == 0 ){
          std::string vertex1, vertex2, vertex3;
          unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
          int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2] );
          if (matches != 9){
              printf("File can't be read by our simple parser : ( Try exporting with other options\n");
              return false;
          }
          vertexIndices.push_back(vertexIndex[0]);
          vertexIndices.push_back(vertexIndex[1]);
          vertexIndices.push_back(vertexIndex[2]);
          uvIndices    .push_back(uvIndex[0]);
          uvIndices    .push_back(uvIndex[1]);
          uvIndices    .push_back(uvIndex[2]);
          normalIndices.push_back(normalIndex[0]);
          normalIndices.push_back(normalIndex[1]);
          normalIndices.push_back(normalIndex[2]);
      }
  }

  // Lastly, convert vectors into glm::vec3 with indices removed
  for (unsigned int i = 0; i < vertexIndices.size(); i++)
    {
      unsigned int vertexIndex = vertexIndices[i];
      glm::vec3 vertex = temp_vertices[ vertexIndex-1 ];
      out_vertices.push_back(vertex);
    }

  for (unsigned int i = 0; i < uvIndices.size(); i++)
    {
      unsigned int uvIndex = uvIndices[i];
      glm::vec2 uv = temp_uvs[ uvIndex-1 ];
      out_uvs.push_back(uv);
    }

  for (unsigned int i = 0; i < normalIndices.size(); i++)
    {
      unsigned int normalIndex = normalIndices[i];
      glm::vec3 normal = temp_normals[ normalIndex-1 ];
      out_normals.push_back(normal);
    }

  return true;
}

int main(void)
{
    //Declare window/context
    GLFWwindow* window;
    //Declare handlers for vertex array, vertex buffer, shaders, and program
    GLuint vertex_shader, fragment_shader, program;
    //Declare location for MVP and Position and Color attributes
    GLint mvp_location;
//    GLint vpos_location, vcol_location;

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
    window = glfwCreateWindow(1024, 768, "Simple example", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    cout << glGetError() << endl;
    cout << gluErrorString(glGetError()) << endl;

    //Set key callback function
    glfwSetKeyCallback(window, key_callback);

    //Set the created context as current
    glfwMakeContextCurrent(window);

    glewExperimental=true; // Needed in core profile
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        return -1;
    }

    cout << glGetError() << endl;
    cout << gluErrorString(glGetError()) << endl;

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

    // NOTE: OpenGL error checks have been omitted for brevity


    // Read our .obj file
    std::vector< glm::vec3 > vertices;
    std::vector< glm::vec2 > uvs;
    std::vector< glm::vec3 > normals; // Won't be used at the moment.
    bool res = loadOBJ("../assets/sphere.obj", vertices, uvs, normals);

    cout << "vertices.size() = " << vertices.size() << endl;
    cout << "uvs.size() = " << uvs.size() << endl;

    GLuint pos_buffer;

    //Position vertex data:
    //Generate 1 buffer and store it in vertex_buffer
    glGenBuffers(1, &pos_buffer);

    //Bind it to an array of vertex attributes
    glBindBuffer(GL_ARRAY_BUFFER, pos_buffer);

    //Assign memory to buffer and initialize it to values in vertices array
    /* usage is a hint to the GL implementation as to how a buffer object's data store will
     * be accessed. This enables the GL implementation to make more intelligent decisions
     * that may significantly impact buffer object performance. It does not, however, constrain
     * the actual usage of the data store. usage can be broken down into two parts: first, the
     * frequency of access (modification and usage), and second, the nature of that access.
     *
     * The frequency of access may be one of these:
     * -Stream: The data store contents will be modified once and used at most a few times.
     * -Static: The data store contents will be modified once and used many times.
     * -Dynamic: The data store contents will be modified repeatedly and used many times.
     *
     * The nature of access may be one of these:
     * -Draw: The data store contents are modified by the application, and used as the source
     * for GL drawing and image specification commands.
     * -Read: The data store contents are modified by reading data from the GL, and used to
     * return that data when queried by the application.
     * -Copy: The data store contents are modified by reading data from the GL, and used as
     * the source for GL drawing and image specification commands.
     * */
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

    //glEnableVertexAttribArray — Enable or disable a generic vertex attribute array
    //glVertexAttribPointer — define an array of generic vertex attribute data
    //e.g. enabling vPos, 2 components per vertex attribute, of type float, not to
    //be normalized (taken as passed), with a sizeof(member) offset btwn consecutive
    //components within the attribute. Last arg gives how many bytes into the vertex
    //you'll find this attribute
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

    //Texture vertex data:
    glEnable(GL_TEXTURE_2D);
    glActiveTexture(GL_TEXTURE0);
    GLuint textureID = loadTex("../assets/Earthmap720x360_grid.jpg");
    //Set active texture unit (for shader)
    //The main purpose of texture units is to allow us to use more than 1 texture in our shaders.
    //By assigning texture units to the samplers, we can bind to multiple textures at once as long
    //as we activate the corresponding texture unit first.
    glBindTexture(GL_TEXTURE_2D, textureID);

    //Variables for checking shader compilation
    GLint Result = GL_FALSE;
    int InfoLogLength;

    cout << glGetError() << endl;
    cout << gluErrorString(glGetError()) << endl;

    //Compile vertex shader defined as text above to GL object and compile
    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_text, NULL);
    glCompileShader(vertex_shader);

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
    mvp_location = glGetUniformLocation(program, "MVP");
    //We also have to tell OpenGL to which texture unit each shader sampler belongs to by setting each sampler
    glUniform1i(glGetUniformLocation(program, "myTextureSampler"), 0);

    // Model matrix : an identity matrix (model will be at the origin)
    glm::mat4 Model = glm::mat4(1.0f);
    glm::vec3 scale = glm::vec3(0.5f, 0.5f, 0.5f);
    Model = glm::scale(Model, scale);
    //==Main loop==//
    while (!glfwWindowShouldClose(window))
    {
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
        glm::mat4 Projection = glm::perspective(glm::radians(45.0f), (float) width / (float)height, 0.1f, 100.0f);

        // View (camera) matrix
        glm::mat4 View = glm::lookAt(
            glm::vec3(4,3,3), // Camera is at (5,3,3), in World Space
            glm::vec3(0,0,0), // and looks at the origin
            glm::vec3(0,1,0)  // Head is up (set to 0,-1,0 to look upside-down)
            );

        //Rotate model in y-axis
        Model = glm::rotate(Model, 0.04f, glm::vec3(0, 1.0f, 0));

        // Our ModelViewProjection : multiplication of our 3 matrices
        glm::mat4 mvp = Projection * View * Model; // Remember, matrix multiplication is the other way around

        // Send our transformation to the currently bound shader, in the "MVP" uniform
        // This is done in the main loop since each model will have a different MVP matrix (At least for the M part)
        glUniformMatrix4fv(mvp_location, 1, GL_FALSE, &mvp[0][0]);

        glDrawArrays(GL_TRIANGLES, 0, vertices.size() * sizeof(glm::vec3)); // Starting from vertex 0; 3 vertices = 1 triangle
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}
