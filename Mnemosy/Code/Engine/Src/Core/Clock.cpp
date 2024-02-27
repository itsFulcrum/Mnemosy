#include "Include/Core/Clock.h"

#include <GLFW/glfw3.h>

namespace mnemosy::core
{
	
	void Clock::Update()
	{
		m_currentTime = static_cast<double>(glfwGetTime());
		m_deltaSeconds = m_currentTime - m_timeLastFrame;

		if (capDeltaTime)
		{
			if (m_deltaSeconds < m_deltaLowLimit)
				m_deltaSeconds = m_deltaLowLimit;
			else if (m_deltaSeconds > m_deltaHighLimit)
				m_deltaSeconds = m_deltaHighLimit;
		}

		m_timeLastFrame = m_currentTime;


		//m_currentTimeMili = static_cast<double>(glfwGetTime()) / 1000;
		//m_deltaMili = m_currentTimeMili - m_timeLastFrameMili;

		//double m_deltaLowLimit = 0.006f;	// Keep Below 165fps // 0/165
		//double m_deltaHighLimit = 0.1;		// Keep Above 10fps // 1/10

		//double deltaLow = 0.006;
		//double deltaHigh = 0.1;

		//if (capDeltaTime)
		//{
		//	if (m_deltaMili < deltaLow)
		//		m_deltaMili = deltaLow;
		//	else if (m_deltaMili > deltaHigh)
		//		m_deltaMili = deltaHigh;
		//}
		//m_timeLastFrameMili = m_currentTimeMili;



		m_fpsCounter += (float)m_deltaSeconds;
		if (m_fpsCounter >= 1.f)
		{
			m_fpsCounter = 0;
			m_currentFPS = m_frameCount - m_frameCountAtLastSecondStep;
			m_frameCountAtLastSecondStep = m_frameCount;
		}

		m_frameCount++;
	}

	float Clock::GetFrameTime()
	{
		return float(m_deltaSeconds * 1000.0);
	}

} // mnemosy::core