#include <string>
#include <vector>
#include <cmath>
#include "../loadTex.hpp"

class Planet
  {
	private:
	  unsigned int index;
	  //xPos, yPos, zPos defined as distance from sun (which is at (0,0,0) in this system)
	  double xPos;
	  double yPos;
	  double zPos;
	  double mu;
	  double a;
	  
	public:
      GLuint texID;
	  double scale;
	
	  static double scaleConst;
	  static vector<string> planetNames = vector<string>{"sun", "mercury", "venus", "earth", "mars", "jupiter", "saturn", "uranus", "neptune"};
	  static vector<double> planetRadii = vector<double>{695510.0, 2439.7, 6051.8, 6371.0, 3389.5, 69911.0, 58232.0, 25362.0, 24622.0}; //km
	  static vector<double> planetPerihelia = vector<double>{0.0, .307, .718, .98, 1.38, 4.95, 9.05, 18.4, 29.8}; //AU
	  static vector<double> planetAphelia = vector<double>{0.0, .466, .728, 1.01, 1.66, 5.46, 10.12, 20.1, 30.4}; //AU
	  static vector<string> planetTextures = {"../assets/sun.dds", "../assets/mercury.dds", "../assets/venus.dds", "../assets/earth.dds", "../assets/mars.dds", "../assets/jupiter.dds", "../assets/saturn.dds", "../assets/uranus.dds", "../assets/neptune.dds"};
	  
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
	  
	  void updateLocation(double dt)
	  {
		  //Normal to orbital plane is defined as (0, 1, 0)
		  glm::vec3 normal = {0.0, 1.0, 0.0};
		  glm::vec3 pos = {xPos, yPos, zPos};
		  double r = glm::length(pos);
		  double magnitudeV = sqrt(mu*(2/r - 1/a));
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
		  glm::vec3 localPos = {xPos, yPos, zPos};
		  glm::vec3 worldPos;
		  
		  
		  return worldPos;
	  }
	
  }