#pragma once

#include <string>
#include <vector>
#include <cmath>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include "loadTex.hpp"

using namespace std;
#define pi 3.142857

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
	  float rotationRate;
	  int counter;
	  
	public:
      GLuint texID;
	  glm::mat4 model;
	  float scale;
	
	  static float scaleConst;
	  static float timeScale;
	  static vector<string> planetNames;
	  static vector<float> planetRadii;
	  static vector<float> planetPerihelia;
	  static vector<float> planetAphelia;
	  static vector<string> planetTextures;
	  static vector<float> planetMus;
	  static vector<float> planetRotRates;
	  
	  Planet(char * name)
		: yPos(0), zPos(0), counter(0)
	  {
		for (unsigned int i = 0; i < planetNames.size(); ++i)
		{
			if (strcmp(name, planetNames[i].c_str()) == 0)
			{
				index = i;
			}
		}
		
		//Load texture
		texID = loadTex(planetTextures[index].c_str());

		//Set up planetary data
		xPos = 1.496 * pow(10.0f, 8.0f) * planetAphelia[index]; //starting position is at aphelion
		if (index != 0)
		{
			scale = scaleConst * (planetRadii[index] / planetRadii[3]); //scale relative to Earth
		}
		else
		{
			scale = 0.005 * scaleConst * (planetRadii[index] / planetRadii[3]); //scale relative to Sun
		}
		mu = planetMus[index];
		a = 1.496 * pow(10.0f, 8.0f) * (planetPerihelia[index] + planetAphelia[index]) / 2.0;
		rotationRate = planetRotRates[index];

		//Set up model matrix
		model = glm::mat4(1.0f);
		glm::vec3 scaleVec = scale * glm::vec3(0.5f, 0.5f, 0.5f);
		model = glm::scale(model, scaleVec);
	  }

	  Planet(int index)
		  : yPos(0), zPos(0), counter(0), index(index)
	  {
		  //Load texture
		  texID = loadTex(planetTextures[index].c_str());

		  //Set up planetary data
		  xPos = 1.496 * pow(10.0f, 8.0f) * planetAphelia[index]; //starting position is at aphelion
		  if (index != 0)
		  {
			  scale = scaleConst * (planetRadii[index] / planetRadii[3]); //scale relative to Sun
		  }
		  else
		  {
			  scale = 0.005 * scaleConst * (planetRadii[index] / planetRadii[3]); //scale relative to Sun
		  }
		  
		  mu = planetMus[index];
		  a = 1.496 * pow(10.0f, 8.0f) * (planetPerihelia[index] + planetAphelia[index]) / 2.0;
		  rotationRate = planetRotRates[index];

		  //Set up model matrix
		  model = glm::mat4(1.0f);
		  glm::vec3 scaleVec = scale * glm::vec3(0.5f, 0.5f, 0.5f);
		  model = glm::scale(model, scaleVec);
	  }
	  
	  ~Planet()
	  {
		  
	  }
	  
	  void updateLocation(float dt)
	  {
		  if (counter == 0)
		  {
			  //Translate model to starting position
			  glm::vec3 translation = { xPos, yPos, zPos };
			  translation = getPosInWorldCoordinates(translation);
			  model = glm::translate(model, translation);
			  counter++;
		  }
		  else
		  {
			  //Rotate model in y-axis
			  //model = glm::rotate(model, rotationRate * dt, glm::vec3(0, 1.0f, 0));

			  //Normal to orbital plane is defined as (0, 1, 0)
			  glm::vec3 normal = { 0.0, 1.0, 0.0 };
			  glm::vec3 pos = { xPos, yPos, zPos };
			  float r = glm::length(pos);

			  if (abs(r) > 0.0f)
			  {
				  float magnitudeV = timeScale * sqrt(mu*(2.0f / r - 1.0f / a));
				  //cout << "mag V: " << magnitudeV;
				  //Get unit vector direction of velocity
				  //Initial V is in +z dir
				  glm::vec3 directionV = glm::cross(pos, normal);
				  //cout << "index: " << index;
				  //cout << "Direction v: " << directionV.x << " " << directionV.y << " " << directionV.z << endl;
				  directionV = directionV / glm::length(directionV);
				  //cout << "Direction v: " << directionV.x << " " << directionV.y << " " << directionV.z << endl;
				  //Finally, the velocity vector
				  glm::vec3 velocity = magnitudeV * directionV;
				  //cout << "velocity: " << velocity.x << " " << velocity.y << " " << velocity.z << endl;

				  //Update location in local coordinates
				  glm::vec3 newPos = pos + velocity * dt / 1000.0f;
				  //cout << "Pos: " << pos.x << " " << pos.y << " " << pos.z << endl;
				  //cout << "newPos: " << newPos.x << " " << newPos.y << " " << newPos.z << endl;


				  //Translate model to new position
				  glm::vec3 translation = { newPos.x - pos.x, newPos.y - pos.y, newPos.z - pos.z };
				  translation = getPosInWorldCoordinates(translation);
				  model = glm::translate(model, translation);

				  //Update stored position
				  xPos = newPos.x;
				  yPos = newPos.y;
				  zPos = newPos.z;
			  }
		  }
	  }
	  
	  glm::vec3 getPosInWorldCoordinates(glm::vec3 localPos)
	  {
		  //Set up transformation matrix (rotate about x direction and move back into -z)
		  glm::mat4 transform = glm::mat4(1.0f);
		  transform = glm::rotate(transform, glm::radians(10.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		  transform = glm::translate(transform, glm::vec3(0.0, 0.0, -50.0));

		  //Get transform coordinates and return as vec3
		  glm::vec4 worldPos = transform * glm::vec4(localPos, 1.0);
		  glm::vec3 worldPosv3(worldPos);
		  worldPosv3 = worldPosv3 / 3000500.0f;
		  
		  return worldPosv3;
	  }

	  void updateScale()
	  {
		  if (index != 0)
		  {
			  scale = scaleConst * (planetRadii[index] / planetRadii[3]); //scale relative to Sun
		  }
		  else
		  {
			  scale = 0.005 * scaleConst * (planetRadii[index] / planetRadii[3]); //scale relative to Sun
		  }
	  }
	
};

float Planet::scaleConst = 25.0f;
float Planet::timeScale = 75000.0f;
vector<string> Planet::planetNames = { "sun", "mercury", "venus", "earth", "mars", "jupiter", "saturn", "uranus", "neptune" };
vector<float> Planet::planetRadii = { 695510.0, 2439.7, 6051.8, 6371.0, 3389.5, 69911.0, 58232.0, 25362.0, 24622.0 }; //km
vector<float> Planet::planetPerihelia = { 0.0, .307, .718, .98, 1.38, 4.95, 9.05, 18.4, 29.8 }; //AU
vector<float> Planet::planetAphelia = { 0.0, .466, .728, 1.01, 1.66, 5.46, 10.12, 20.1, 30.4 }; //AU
vector<string> Planet::planetTextures = { "../assets/sun.dds", "../assets/mercury.dds", "../assets/venus.dds", "../assets/earth.dds", "../assets/mars.dds", "../assets/jupiter.dds", "../assets/saturn.dds", "../assets/uranus.dds", "../assets/neptune.dds" };
vector<float> Planet::planetMus = { 1.327 * pow(10.0, 20.0), 2.203 * pow(10.0, 13.0), 3.249 * pow(10.0, 14.0), 3.986 * pow(10.0, 14.0), 4.282 * pow(10.0, 13.0), 1.267 * pow(10.0, 17.0), 3.793 * pow(10.0, 16.0), 5.793 * pow(10.0, 15.0), 6.837 * pow(10.0, 15.0) };
vector<float> Planet::planetRotRates = { 0.041667f * (2.0f * pi / 86400.0f), 58.0f * (2.0f * pi / 86400.0f), -244.0f * (2.0f * pi / 86400.0f), (2.0f * pi / 86400.0f), 1.03f * (2.0f * pi / 86400.0f), 0.415f * (2.0f * pi / 86400.0f), 0.445f * (2.0f * pi / 86400.0f), -0.720f * (2.0f * pi / 86400.0f), 0.673f * (2.0f * pi / 86400.0f) };
