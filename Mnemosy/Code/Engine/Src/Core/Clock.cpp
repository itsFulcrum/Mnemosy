#include "Include/Core/Clock.h"

#include "Include/MnemosyConfig.h"


#include <GLFW/glfw3.h>

#define DeltaLowLimit	0.006f	// Keep Below 165fps // 1/165
#define DeltaHighLimit	0.1f	// Keep Above 10fps // 1/10

namespace mnemosy::core
{
	void Clock::Init() {

		//capDeltaTime = true;
		
		//m_deltaLowLimit = 0.006f;	// Keep Below 165fps // 1/165
		//m_deltaHighLimit = 0.1;		// Keep Above 10fps // 1/10

		// time is all handled in seconds

		m_currentTime = 0.0;
		m_timeLastFrame = 0.0;
		m_deltaSeconds = 0.0;
		m_uncappedDeltaSeconds = 0.0;

		m_fpsCounter = 0.0f;
		m_frameCount = 0;
		m_frameCountAtLastSecondStep = 0;
		m_currentFPS = 0;
	}
	void Clock::Update()
	{
		m_currentTime = static_cast<double>(glfwGetTime());
		m_deltaSeconds = m_currentTime - m_timeLastFrame;

		m_uncappedDeltaSeconds = m_deltaSeconds;


#ifdef MNSY_CONFIG_CAP_DELTA_TIME

		if (m_deltaSeconds < DeltaLowLimit) {
			m_deltaSeconds = DeltaLowLimit;
		}
		else if (m_deltaSeconds > DeltaHighLimit) {
			m_deltaSeconds = DeltaHighLimit;
		}
#endif // MNSY_CONFIG_CAP_DELTA_TIME


		m_timeLastFrame = m_currentTime;

		m_fpsCounter += (float)m_uncappedDeltaSeconds;
		if (m_fpsCounter >= 1.f)
		{
			m_fpsCounter = 0;
			m_currentFPS = m_frameCount - m_frameCountAtLastSecondStep;
			m_frameCountAtLastSecondStep = m_frameCount;
		}

		m_frameCount++;
	}

	const double Clock::GetTimeSinceLaunch()
	{
		return glfwGetTime();
	}

	const float Clock::GetFrameTime()
	{
		return float(m_uncappedDeltaSeconds * 1000.0);
	}

} // mnemosy::core