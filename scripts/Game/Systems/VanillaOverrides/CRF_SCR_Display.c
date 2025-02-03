modded class SCR_VonDisplay
{
	override event void OnCapture(BaseTransceiver transmitter)
	{
		if (!m_wRoot)
			return;
		
		if (SCR_PlayerController.Cast(GetGame().GetPlayerController()).m_eCamera)
			return;

		int frequency;

		if (transmitter)	// can be null when using direct speech
			frequency = transmitter.GetFrequency();

		// update only when first activation / device changed / frequency changed
		if (m_OutTransmission.m_bForceUpdate == true
			|| m_OutTransmission.m_bIsActive == false
			|| m_OutTransmission.m_RadioTransceiver != transmitter
			|| (transmitter && m_OutTransmission.m_fFrequency != transmitter.GetFrequency())
		)
		{
			UpdateTransmission(m_OutTransmission, transmitter, frequency, false);
		}

		m_OutTransmission.m_bIsActive = true;
		m_OutTransmission.m_fActiveTimeout = 0;
	}
}