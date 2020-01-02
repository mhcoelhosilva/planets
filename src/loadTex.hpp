#pragma once

#include <stdio.h>

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

using namespace std;

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
