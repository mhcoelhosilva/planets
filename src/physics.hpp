#pragma once

#include <vector>
#include <glm.hpp>

#include "../libs/bullet3/src/BulletCollision/CollisionDispatch/btCollisionDispatcher.h"
#include "../libs/bullet3/src/BulletCollision/CollisionDispatch/btCollisionWorld.h"
#include "../libs/bullet3/src/BulletCollision/CollisionDispatch/btCollisionObject.h"
#include "../libs/bullet3/src/BulletCollision/CollisionDispatch/btDefaultCollisionConfiguration.h"
#include "../libs/bullet3/src/BulletCollision/CollisionShapes/btBoxShape.h"
#include "../libs/bullet3/src/BulletCollision/CollisionShapes/btSphereShape.h"
#include "../libs/bullet3/src/BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h"
#include "../libs/bullet3/src/BulletDynamics/Dynamics/btRigidBody.h"
#include "../libs/bullet3/src/BulletDynamics/ConstraintSolver/btSequentialImpulseConstraintSolver.h"
#include "../libs/bullet3/src/BulletCollision/BroadphaseCollision/btDbvtBroadphase.h"
#include "../libs/bullet3/src/LinearMath/btDefaultMotionState.h"

#include "GLDebugDrawer.hpp"

using namespace std;

class Physics
{
private:
	
	vector<btRigidBody*> rigidBodies;

public:

	btDiscreteDynamicsWorld* dynamicsWorld;
	GLDebugDrawer MyDrawer;

	Physics()
		: MyDrawer()
	{
		// Build the broadphase
		btBroadphaseInterface* broadphase = new btDbvtBroadphase();

		// Set up the collision configuration and dispatcher
		btDefaultCollisionConfiguration* collisionConfiguration = new btDefaultCollisionConfiguration();
		btCollisionDispatcher* dispatcher = new btCollisionDispatcher(collisionConfiguration);

		// The actual physics solver
		btSequentialImpulseConstraintSolver* solver = new btSequentialImpulseConstraintSolver;

		// The world.
		dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfiguration);
		dynamicsWorld->setGravity(btVector3(0, -9.81f, 0));

		//Debug drawing
		MyDrawer.setDebugMode(btIDebugDraw::DBG_DrawWireframe);
		//MyDrawer.setDebugMode(1);
		dynamicsWorld->setDebugDrawer(&MyDrawer);
	}

	~Physics()
	{
		delete dynamicsWorld;
		dynamicsWorld = nullptr;
	}

	void addRigidBody(btRigidBody *rigidBody, void * planet)
	{
		this->dynamicsWorld->addRigidBody(rigidBody);
		rigidBodies.push_back(rigidBody);
		rigidBody->setUserPointer(planet);
	}

	bool checkCollision()
	{
		return false;
	}

	void * checkRaycast(glm::mat4 projectionMatrix, glm::mat4 viewMatrix, int screenWidth, int screenHeight, int mouseX, int mouseY)
	{
		void * result;

		// The ray Start and End positions, in Normalized Device Coordinates
		glm::vec4 lRayStart_NDC(
			((float)mouseX / (float)screenWidth - 0.5f) * 2.0f, // [0,1024] -> [-1,1]
			((float)mouseY / (float)screenHeight - 0.5f) * 2.0f, // [0, 768] -> [-1,1]
			-1.0, // The near plane maps to Z=-1 in Normalized Device Coordinates
			1.0f
		);
		glm::vec4 lRayEnd_NDC(
			((float)mouseX / (float)screenWidth - 0.5f) * 2.0f,
			((float)mouseY / (float)screenHeight - 0.5f) * 2.0f,
			0.0,
			1.0f
		);

		//Get ray in world position
		glm::mat4 M = glm::inverse(projectionMatrix * viewMatrix);
		glm::vec4 lRayStart_world = M * lRayStart_NDC; lRayStart_world/=lRayStart_world.w;
		glm::vec4 lRayEnd_world   = M * lRayEnd_NDC  ; lRayEnd_world  /=lRayEnd_world.w;

		glm::vec3 lRayDir_world(lRayEnd_world - lRayStart_world);
		lRayDir_world = glm::normalize(lRayDir_world);

		glm::vec3 out_origin(lRayStart_world);
		glm::vec3 out_direction(lRayDir_world);
		glm::vec3 out_end = out_origin + out_direction * 100000.0f;

		//Test for ray collision with one of the rigid bodies
		btCollisionWorld::ClosestRayResultCallback RayCallback(
			btVector3(out_origin.x, out_origin.y, out_origin.z),
			btVector3(out_end.x, out_end.y, out_end.z)
		);
		dynamicsWorld->rayTest(
			btVector3(out_origin.x, out_origin.y, out_origin.z),
			btVector3(out_end.x, out_end.y, out_end.z),
			RayCallback
		);

		if (RayCallback.hasHit()) 
		{
			result = (void*)RayCallback.m_collisionObject->getUserPointer();
		}
		else
		{
			cout << "No hit" << endl;
			bool resultBool = false;
			result = (void*)&resultBool;
		}

		return result;
	}
};

