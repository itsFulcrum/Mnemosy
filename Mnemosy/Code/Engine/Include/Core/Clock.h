#ifndef CLOCK_H
#define CLOCK_H


namespace mnemosy::core
{
	class Clock
	{
	public:
		Clock() = default;
		~Clock() = default;

		void Update();

		double GetTimeSeconds() { return m_currentTime; }
		double GetDeltaSeconds() { return m_deltaSeconds; }
		//double GetDeltaMiliseconds() { return m_deltaMili; }

		int GetFPS() { return m_currentFPS; }


		bool capDeltaTime = true;

	private:

		double m_deltaLowLimit = 0.006f;	// Keep Below 165fps // 1/165
		double m_deltaHighLimit = 0.1;		// Keep Above 10fps // 1/10

		// time is all handled in seconds

		double m_currentTime = 0.0;
		double m_timeLastFrame = 0.0;
		double m_deltaSeconds = 0.0;
		
		//double m_currentTimeMili = 0.0f;
		//double m_timeLastFrameMili = 0.0f;
		//double m_deltaMili = 0.0f;



		int m_frameCount = 0;
		float m_fpsCounter = 0.f;
		int m_frameCountAtLastSecondStep = 0;
		int m_currentFPS = 0;

	};


}
#endif // !CLOCK_H