#ifndef CLOCK_H
#define CLOCK_H


namespace mnemosy::core
{
	class Clock
	{
	public:
		Clock() = default;
		~Clock() = default;

		void Init();
		void Update();

		const double GetTimeSeconds() { return m_currentTime; }
		const double GetTimeSinceLaunch();
		const double GetDeltaSeconds() { return m_deltaSeconds; }
		//double GetDeltaMiliseconds() { return m_deltaMili; }

		const int GetFPS() { return m_currentFPS; }
		const float GetFrameTime();

		//bool capDeltaTime = true;

	private:

		//double m_deltaLowLimit = 0.006f;	// Keep Below 165fps // 1/165
		//double m_deltaHighLimit = 0.1;		// Keep Above 10fps // 1/10

		// time is all handled in seconds

		double m_currentTime = 0.0;
		double m_timeLastFrame = 0.0;
		double m_deltaSeconds = 0.0;
		double m_uncappedDeltaSeconds = 0.0;

		float m_fpsCounter = 0.0f;
		unsigned int m_frameCount = 0;
		int m_frameCountAtLastSecondStep = 0;
		int m_currentFPS = 0;

	};


}
#endif // !CLOCK_H
