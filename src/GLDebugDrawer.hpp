#pragma once

#include "../libs/bullet3/src/LinearMath/btIDebugDraw.h"

#include <GL/glew.h>
#include <glfw3.h>

class GLDebugDrawer : public btIDebugDraw
{
	int m_debugMode;

public:

	GLDebugDrawer()
		:m_debugMode(0)
	{
	}

	virtual void   drawLine(const btVector3& from, const btVector3& to, const btVector3& color)
	{
		if (m_debugMode > 0)
		{
			float tmp[6] = { from.getX(), from.getY(), from.getZ(),
						  to.getX(), to.getY(), to.getZ() };

			glPushMatrix();
			{
				glColor4f(color.getX(), color.getY(), color.getZ(), 1.0f);
				glVertexPointer(3,
					GL_FLOAT,
					0,
					&tmp);

				glPointSize(5.0f);
				glDrawArrays(GL_POINTS, 0, 2);
				glDrawArrays(GL_LINES, 0, 2);
			}
			glPopMatrix();
		}
	}

	virtual void drawLine(const btVector3& from, const btVector3& to, const btVector3& fromColor, const btVector3& toColor)
	{
		/*
		SColorf fromC;  fromC.set(fromColor[0], fromColor[1], fromColor[2], fromColor[3]);
		SColorf toC;    toC.set(toColor[0], toColor[1], toColor[2], toColor[3]);

		matrix4 id;
		id.makeIdentity();
		m_Driver->setTransform(video::ETS_WORLD, id);
		m_Driver->draw3DLine(from, to, fromColor.toSColor());
		*/
	}

	virtual void   drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color)
	{
		{
			//btVector3 to=pointOnB+normalOnB*distance;
			//const btVector3&from = pointOnB;
			//glColor4f(color.getX(), color.getY(), color.getZ(), 1.0f);   

			//GLDebugDrawer::drawLine(from, to, color);

			//glRasterPos3f(from.x(),  from.y(),  from.z());
			//char buf[12];
			//sprintf(buf," %d",lifeTime);
			//BMF_DrawString(BMF_GetFont(BMF_kHelvetica10),buf);
		}
	}

	virtual void   reportErrorWarning(const char* warningString)
	{
		printf(warningString);
	}

	virtual void   draw3dText(const btVector3& location, const char* textString)
	{
		//glRasterPos3f(location.x(),  location.y(),  location.z());
		//BMF_DrawString(BMF_GetFont(BMF_kHelvetica10),textString);
	}

	virtual void   setDebugMode(int debugMode)
	{
		m_debugMode = debugMode;
	}

	virtual int      getDebugMode() const 
	{ 
		return m_debugMode; 
	}

};