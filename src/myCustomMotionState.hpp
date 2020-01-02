#pragma once

#include <glm.hpp>

class myCustomMotionState : public btDefaultMotionState
{
private:


public:
	myCustomMotionState(btTransform& worldTrans)
		: btDefaultMotionState(worldTrans)
	{
	}

	~myCustomMotionState()
	{}

	void getWorldTransform(btTransform& worldTrans)
	{

	}

	void setWorldTransform(const btTransform& worldTrans)
	{
		//Do nothing (kinematic object)
	}

};