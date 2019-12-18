#pragma once

#include <string>
#include <vector>
#include <cmath>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include "loadTex.hpp"

using namespace std;

class Planet
  {
	private:
	  unsigned int index;
	  //xPos, yPos, zPos defined as distance from sun (which is at (0,0,0) in this system)
	  float xPos;
	  float yPos;
	  float zPos;
	  float mu;
	  float a;
	  
	public:
      GLuint texID;
	  float scale;
	
	  static float scaleConst;
	  static vector<string> planetNames;
	  static vector<float> planetRadii;
	  static vector<float> planetPerihelia;
	  static vector<float> planetAphelia;
	  static vector<string> planetTextures;
	  static vector<float> planetMus;
	  
	  Planet(char * name)
		: yPos(0), zPos(0)
	  {
		for (unsigned int i = 0; i < planetNames.size(); ++i)
		{
			if (strcmp(name, planetNames[i].c_str()) == 0)
			{
				index = i;
			}
		}
		
		texID = loadTex(planetTextures[index].c_str());
		xPos = planetAphelia[index]; //starting position is at aphelion
		scale = scaleConst * (planetRadii[index] / planetRadii[2]); //scale relative to Earth
		mu = planetMus[index];
		a = (planetPerihelia[index] + planetAphelia[index]) / 2.0;
		
	  }
	  
	  ~Planet()
	  {
		  
	  }
	  
	  void updateLocation(float dt)
	  {
		  //Normal to orbital plane is defined as (0, 1, 0)
		  glm::vec3 normal = {0.0, 1.0, 0.0};
		  glm::vec3 pos = {xPos, yPos, zPos};
		  float r = glm::length(pos);
		  float magnitudeV = sqrt(mu*(2/r - 1/a));
		  //Get unit vector direction of velocity
		  //Initial V is in +z dir
		  glm::vec3 directionV = glm::cross(pos, normal);
		  directionV = directionV / glm::length(directionV);
		  //Finally, the velocity vector
		  glm::vec3 velocity = magnitudeV * directionV;

		  //Lastly, update location
		  pos = pos + velocity * dt;
		  xPos = pos.x;
		  yPos = pos.y;
		  zPos = pos.z;
	  }
	  
	  glm::vec3 getPosInWorldCoordinates()
	  {
		  //Extract position in local coordinate system
		  glm::vec3 localPos = {xPos, yPos, zPos};

		  //Set up transformation matrix (rotate about x direction and move back into -z)
		  glm::mat4 transform = glm::mat4(1.0f);
		  transform = glm::rotate(transform, glm::radians(30.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		  transform = glm::translate(transform, glm::vec3(0.0, 0.0, -5.0));

		  //Get transform coordinates and return as vec3
		  glm::vec4 worldPos = transform * glm::vec4(localPos, 1.0);
		  glm::vec3 worldPosv3(worldPos);
		  
		  return worldPosv3;
	  }
	
};

vector<string> Planet::planetNames = { "sun", "mercury", "venus", "earth", "mars", "jupiter", "saturn", "uranus", "neptune" };
vector<float> Planet::planetRadii = { 695510.0, 2439.7, 6051.8, 6371.0, 3389.5, 69911.0, 58232.0, 25362.0, 24622.0 }; //km
vector<float> Planet::planetPerihelia = { 0.0, .307, .718, .98, 1.38, 4.95, 9.05, 18.4, 29.8 }; //AU
vector<float> Planet::planetAphelia = { 0.0, .466, .728, 1.01, 1.66, 5.46, 10.12, 20.1, 30.4 }; //AU
vector<string> Planet::planetTextures = { "../assets/sun.dds", "../assets/mercury.dds", "../assets/venus.dds", "../assets/earth.dds", "../assets/mars.dds", "../assets/jupiter.dds", "../assets/saturn.dds", "../assets/uranus.dds", "../assets/neptune.dds" };
vector<float> Planet::planetMus = { 1.327 * pow(10.0, 20.0), 2.203 * pow(10.0, 13.0), 3.249 * pow(10.0, 14.0), 3.986 * pow(10.0, 14.0), 4.282 * pow(10.0, 13.0), 1.267 * pow(10.0, 17.0), 3.793 * pow(10.0, 16.0), 5.793 * pow(10.0, 15.0), 6.837 * pow(10.0, 15.0) };
