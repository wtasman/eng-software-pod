/**
 * @file		FCU__BRAKES__MLP.C
 * @brief		MLP interface for the brakes.
 * @author		Lachlan Grogan
 * @copyright	rLoop Inc.
 */
/**
 * @addtogroup RLOOP
 * @{ */
/**
 * @addtogroup FCU
 * @ingroup RLOOP
 * @{ */
/**
 * @addtogroup FCU__BRAKES__MLP
 * @ingroup FCU
 * @{ */

#include "../fcu_core.h"

#if C_LOCALDEF__LCCM655__ENABLE_THIS_MODULE == 1U
#if C_LOCALDEF__LCCM655__ENABLE_BRAKES == 1U

//the structure
extern struct _strFCU sFCU;

//locals
static void vFCU_BRAKES_MLP__Sample_ADC(E_FCU__BRAKE_INDEX_T eBrake);
static Lint16 s16FCU_BRAKES_MLP__Check_ADC_Limits(E_FCU__BRAKE_INDEX_T eBrake);
static void vFCU_BRAKES_MLP__Apply_Zero(E_FCU__BRAKE_INDEX_T eBrake);
static void vFCU_BRAKES_MLP__Apply_Span(E_FCU__BRAKE_INDEX_T eBrake);
static Lint16 s16FCU_BRAKES_MLP__Filter_ADC_Value(E_FCU__BRAKE_INDEX_T eBrake);

/***************************************************************************//**
 * @brief
 * init the systems specifically to the MLP
 * 
 * @st_funcMD5		A4B3B673117DD0A42C704C2512DC604D
 * @st_funcID		LCCM655R0.FILE.024.FUNC.001
 */
void vFCU_BRAKES_MLP__Init(void)
{

	Luint8 u8Counter;
	Luint32 u32Header;
	Luint8 u8Test;


	//loop through each brake.
	for(u8Counter = 0U; u8Counter < C_FCU__NUM_BRAKES; u8Counter++)
	{

		//setup the fault flags
		vFAULTTREE__Init(&sFCU.sBrakes[u8Counter].sFaultFlags);

		//clear the current ADC sample
		sFCU.sBrakes[u8Counter].sMLP.u16ADC_Sample = 0U;

		//clear the zero value.
		sFCU.sBrakes[u8Counter].sMLP.u16ADC_Zero = 0U;

		//clear the ADC minus zero intermediate equation result.
		sFCU.sBrakes[u8Counter].sMLP.s32ADC_Minus_Zero = 0;

		//init the span
		sFCU.sBrakes[u8Counter].sMLP.f32SystemSpan = 1.0F;

		//brake pos.
		sFCU.sBrakes[u8Counter].sMLP.f32BrakePosition_Percent = 0.0F;

		//set lowest mlp value to a high flag
		sFCU.sBrakes[u8Counter].sMLP.lowest_value = 9999U;

		//set higest mlp value to 0
		sFCU.sBrakes[u8Counter].sMLP.highest_value = 0U;

		#if C_LOCALDEF__LCCM655__ENABLE_DEBUG_BRAKES == 1U
			//debug only
			sFCU.sBrakes[(Luint32)u8Counter].sMLP.zero_count = 0U;
		#endif

		// temporary use; identify when the startup sequence is over and ADC won't be 0.
		sFCU.sBrakes[(Luint32)u8Counter].sMLP.u8Running = 0U;

	}

	//check the CRC
	u8Test = u8EEPARAM_CRC__Is_CRC_OK(	C_LOCALDEF__LCCM655__EEPROM_OFFSET__BRAKES_HEADER,
										C_LOCALDEF__LCCM655__EEPROM_OFFSET__BRAKE1_SPAN,
										C_LOCALDEF__LCCM655__EEPROM_OFFSET__BRAKES_CRC);
	if(u8Test == 1U)
	{
		//valid
		sFCU.sBrakes[0].sMLP.u16ADC_Zero = u16EEPARAM__Read(C_LOCALDEF__LCCM655__EEPROM_OFFSET__BRAKE0_ZERO);
		sFCU.sBrakes[1].sMLP.u16ADC_Zero = u16EEPARAM__Read(C_LOCALDEF__LCCM655__EEPROM_OFFSET__BRAKE1_ZERO);

		sFCU.sBrakes[0].sMLP.f32SystemSpan = f32EEPARAM__Read(C_LOCALDEF__LCCM655__EEPROM_OFFSET__BRAKE0_SPAN);
		sFCU.sBrakes[1].sMLP.f32SystemSpan = f32EEPARAM__Read(C_LOCALDEF__LCCM655__EEPROM_OFFSET__BRAKE1_SPAN);

	}//if(u8Test == 1U)
	else
	{
		//CRC is invalid
		//rewrite.
		vEEPARAM__WriteU32(C_LOCALDEF__LCCM655__EEPROM_OFFSET__BRAKES_HEADER, 0xAABBCCDD, 1U);

		//save the zero
		vEEPARAM__WriteU16(C_LOCALDEF__LCCM655__EEPROM_OFFSET__BRAKE0_ZERO, 0U, 1U);
		vEEPARAM__WriteU16(C_LOCALDEF__LCCM655__EEPROM_OFFSET__BRAKE1_ZERO, 0U, 1U);

		//do the span
		vEEPARAM__WriteF32(C_LOCALDEF__LCCM655__EEPROM_OFFSET__BRAKE0_SPAN, 1.0F, 1U);
		vEEPARAM__WriteF32(C_LOCALDEF__LCCM655__EEPROM_OFFSET__BRAKE1_SPAN, 1.0F, 0U);

		//redo the CRC;
		vEEPARAM_CRC__Calculate_And_Store_CRC(	C_LOCALDEF__LCCM655__EEPROM_OFFSET__BRAKES_HEADER,
												C_LOCALDEF__LCCM655__EEPROM_OFFSET__BRAKE1_SPAN,
												C_LOCALDEF__LCCM655__EEPROM_OFFSET__BRAKES_CRC);

		//1. Reload the structures.
		sFCU.sBrakes[0].sMLP.u16ADC_Zero = u16EEPARAM__Read(C_LOCALDEF__LCCM655__EEPROM_OFFSET__BRAKE0_ZERO);
		sFCU.sBrakes[1].sMLP.u16ADC_Zero = u16EEPARAM__Read(C_LOCALDEF__LCCM655__EEPROM_OFFSET__BRAKE1_ZERO);
		sFCU.sBrakes[0].sMLP.f32SystemSpan = f32EEPARAM__Read(C_LOCALDEF__LCCM655__EEPROM_OFFSET__BRAKE0_SPAN);
		sFCU.sBrakes[1].sMLP.f32SystemSpan = f32EEPARAM__Read(C_LOCALDEF__LCCM655__EEPROM_OFFSET__BRAKE1_SPAN);

		//set the flags for a general fault and cal data reload fault.
		vFAULTTREE__Set_Flag(&sFCU.sBrakes[0].sFaultFlags, C_LCCM655__BRAKES__FAULT_INDEX__00);
		vFAULTTREE__Set_Flag(&sFCU.sBrakes[0].sFaultFlags, C_LCCM655__BRAKES__FAULT_INDEX__03);
		vFAULTTREE__Set_Flag(&sFCU.sBrakes[1].sFaultFlags, C_LCCM655__BRAKES__FAULT_INDEX__00);
		vFAULTTREE__Set_Flag(&sFCU.sBrakes[1].sFaultFlags, C_LCCM655__BRAKES__FAULT_INDEX__03);



	}//else if(u8Test == 1U)

	//now that we are ready, start the conversions.
	vRM4_ADC_USER__StartConversion();
}

/***************************************************************************//**
 * @brief
 * resample the MLP sensors
 * 
 * @st_funcMD5		B0CAF27A0006C9DD7D4CD5096CE28252
 * @st_funcID		LCCM655R0.FILE.024.FUNC.002
 */
void vFCU_BRAKES_MLP__Process(void)
{
	Luint8 u8Counter;
	Lint16 s16Limit;
	Lint16 s16Return;

	//loop through each brake.
	for(u8Counter = 0U; u8Counter < C_FCU__NUM_BRAKES; u8Counter++)
	{
		//Update the ADC:
		vFCU_BRAKES_MLP__Sample_ADC((E_FCU__BRAKE_INDEX_T)u8Counter);

		// sometimes sample hits 0 but isn't a real value, throw it out
		if (sFCU.sBrakes[(Luint32)u8Counter].sMLP.u16ADC_Sample == 0U && sFCU.sBrakes[(Luint32)u8Counter].sMLP.u8Running == 0U)
		{
			#if C_LOCALDEF__LCCM655__ENABLE_DEBUG_BRAKES == 1U
				sFCU.sBrakes[(Luint32)u8Counter].sMLP.zero_count++;
			#endif
			continue;
		}
		else
		{
			// start up time ended
			sFCU.sBrakes[(Luint32)u8Counter].sMLP.u8Running = 1U;
		}

		//check the limits
		s16Limit = s16FCU_BRAKES_MLP__Check_ADC_Limits((E_FCU__BRAKE_INDEX_T)u8Counter);
		if(s16Limit >= 0)
		{

			//the adc limits are safe, proceed

			//filter the data.
			s16Return = s16FCU_BRAKES_MLP__Filter_ADC_Value((E_FCU__BRAKE_INDEX_T)u8Counter);
			if( s16Return < 0 )
			{
				//handle error
			}
			else
			{
				sFCU.sBrakes[(Luint32)u8Counter].sMLP.u16ADC_FilteredSample = (Luint16) s16Return;
			}

			//zero the data.
			vFCU_BRAKES_MLP__Apply_Zero((E_FCU__BRAKE_INDEX_T)u8Counter);

			//apply the span.
			vFCU_BRAKES_MLP__Apply_Span((E_FCU__BRAKE_INDEX_T)u8Counter);

			//at this point here, the value in f32BrakePosition_Percent is the calibrated brake position.


		}
		else
		{
			//todo:
			//something bad happened with the ADC,
		}

	}


}


/***************************************************************************//**
 * @brief
 *  Call this function to sample the ADC channel associated with the brake
 * 
 * @param[in]		eBrake				The brake index
 * @st_funcMD5		6C5FC47D0A1DFC4991301F047C4FB3B5
 * @st_funcID		LCCM655R0.FILE.024.FUNC.003
 */
void vFCU_BRAKES_MLP__Sample_ADC(E_FCU__BRAKE_INDEX_T eBrake)
{
	Luint8 u8New;

	//check the ADC converter process
	u8New = u8RM4_ADC_USER__Is_NewDataAvailable();
	if(u8New == 1U)
	{

		//determine the brake index
		switch(eBrake)
		{

			case FCU_BRAKE__LEFT:
				//read from the ADC channel 0
				sFCU.sBrakes[(Luint32)FCU_BRAKE__LEFT].sMLP.u16ADC_Sample = u16RM4_ADC_USER__Get_RawData(0U);
				if (sFCU.sBrakes[(Luint32)FCU_BRAKE__LEFT].sMLP.u16ADC_Sample > sFCU.sBrakes[(Luint32)FCU_BRAKE__LEFT].sMLP.highest_value) {
					sFCU.sBrakes[(Luint32)FCU_BRAKE__LEFT].sMLP.highest_value = sFCU.sBrakes[(Luint32)FCU_BRAKE__LEFT].sMLP.u16ADC_Sample;
				}
				// sometimes sample hits 0 but isn't a real value, throw it out
				if (sFCU.sBrakes[(Luint32)FCU_BRAKE__LEFT].sMLP.u16ADC_Sample < sFCU.sBrakes[(Luint32)FCU_BRAKE__LEFT].sMLP.lowest_value && sFCU.sBrakes[(Luint32)FCU_BRAKE__LEFT].sMLP.u16ADC_Sample != 0U) {
					sFCU.sBrakes[(Luint32)FCU_BRAKE__LEFT].sMLP.lowest_value = sFCU.sBrakes[(Luint32)FCU_BRAKE__LEFT].sMLP.u16ADC_Sample;
				}
				break;

			case FCU_BRAKE__RIGHT:
				//read from the ADC channel 1
				sFCU.sBrakes[(Luint32)FCU_BRAKE__RIGHT].sMLP.u16ADC_Sample = u16RM4_ADC_USER__Get_RawData(1U);
				if (sFCU.sBrakes[(Luint32)FCU_BRAKE__RIGHT].sMLP.u16ADC_Sample > sFCU.sBrakes[(Luint32)FCU_BRAKE__RIGHT].sMLP.highest_value) {
					sFCU.sBrakes[(Luint32)FCU_BRAKE__RIGHT].sMLP.highest_value = sFCU.sBrakes[(Luint32)FCU_BRAKE__RIGHT].sMLP.u16ADC_Sample;
				}
				// sometimes sample hits 0 but isn't a real value, throw it out
				if (sFCU.sBrakes[(Luint32)FCU_BRAKE__RIGHT].sMLP.u16ADC_Sample < sFCU.sBrakes[(Luint32)FCU_BRAKE__RIGHT].sMLP.lowest_value && sFCU.sBrakes[(Luint32)FCU_BRAKE__RIGHT].sMLP.u16ADC_Sample != 0U) {
					sFCU.sBrakes[(Luint32)FCU_BRAKE__RIGHT].sMLP.lowest_value = sFCU.sBrakes[(Luint32)FCU_BRAKE__RIGHT].sMLP.u16ADC_Sample;
				}
				break;


			default:
				//todo, log the error.
				break;

		}//switch(eBrake)

		//taken the data now
		vRM4_ADC_USER__Clear_NewDataAvailable();
	}
	else
	{
		//no new ADC, don't do anything
	}



}

/***************************************************************************//**
 * @brief
 * Check the limits of the ADC data
 *
 * @param[in]		eBrake				The brake index
 * @return			-1 = Error, limits out of range\n
 * 					0 = success
 * @st_funcMD5		BE00FC48D260311DF0578D1DA2E875DB
 * @st_funcID		LCCM655R0.FILE.024.FUNC.004
 */
Lint16 s16FCU_BRAKES_MLP__Check_ADC_Limits(E_FCU__BRAKE_INDEX_T eBrake)
{
	Lint16 s16Return;

	//init vars
	s16Return = -1;


	//Todo

	//1. determine the brake index
	//2. check if the data is in range.
	//hint: sFCU.sBrakes[].u16ADC_Sample
	if (		(sFCU.sBrakes[(Luint32)eBrake].sMLP.u16ADC_Sample < C_LOCALDEF__LCCM655__ADC_SAMPLE__LOWER_BOUND) ||
			(sFCU.sBrakes[(Luint32)eBrake].sMLP.u16ADC_Sample > C_LOCALDEF__LCCM655__ADC_SAMPLE__UPPER_BOUND))
	{
		s16Return = -1;
	}
	else
		{
			s16Return = 0;
		}

	return s16Return;

}

/***************************************************************************//**
 * @brief
 * Filtering the MLP sensor value
 *
 * @param[in]		eBrake					The brake index
 * @return			Filtered Value
 * @st_funcMD5		5026035D4C7154FA483D9F60FA67916B
 * @st_funcID		LCCM655R0.FILE.024.FUNC.005
 */
Lint16 s16FCU_BRAKES_MLP__Filter_ADC_Value(E_FCU__BRAKE_INDEX_T eBrake)
{
	Lint16 s16Return;

	s16Return = u16NUMERICAL_FILTERING__Add_U16(	sFCU.sBrakes[(Luint32)eBrake].sMLP.u16ADC_Sample,
												&sFCU.sBrakes[(Luint32)eBrake].sMLP.u16AverageCounter,
												C_MLP__MAX_AVERAGE_SIZE,
												&sFCU.sBrakes[(Luint32)eBrake].sMLP.u16AverageArray[0]);

	return s16Return;
}


/***************************************************************************//**
 * @brief
 * Apply the system span value.
 *
 * @example
 * Brake_Pos = (ADCValue - Zero) * Span
 * 
 * @param[in]		eBrake				The brake index
 * @st_funcMD5		C18091983749952AB5441691700A6A98
 * @st_funcID		LCCM655R0.FILE.024.FUNC.006
 */
void vFCU_BRAKES_MLP__Apply_Span(E_FCU__BRAKE_INDEX_T eBrake)
{
	Lfloat32 f32Temp;

	//protect the array
	if((Luint32)eBrake < C_FCU__NUM_BRAKES)
	{

		//cast to F32
		f32Temp = (Lfloat32)sFCU.sBrakes[(Luint32)eBrake].sMLP.s32ADC_Minus_Zero;

		//apply the span
		f32Temp *= sFCU.sBrakes[(Luint32)eBrake].sMLP.f32SystemSpan;

		//assign
		sFCU.sBrakes[(Luint32)eBrake].sMLP.f32BrakePosition_Percent = f32Temp;

	}
	else
	{
		//error
		//log this error
	}
}

/***************************************************************************//**
 * @brief
 * Apply the zero value to the ADC sample
 * 
 * @param[in]		eBrake					The brake index
 * @st_funcMD5		91B14636179F6DEA07A2A6B2DD7B2C37
 * @st_funcID		LCCM655R0.FILE.024.FUNC.007
 */
void vFCU_BRAKES_MLP__Apply_Zero(E_FCU__BRAKE_INDEX_T eBrake)
{
	Lint32 s32Temp;

	//protect the array
	if((Luint32)eBrake < C_FCU__NUM_BRAKES)
	{

		//convert ADC sample to s32
		s32Temp = (Lint32)sFCU.sBrakes[(Luint32)eBrake].sMLP.u16ADC_FilteredSample;

		//subtract the zero
		s32Temp -= (Lint32)sFCU.sBrakes[(Luint32)eBrake].sMLP.u16ADC_Zero;

		//assign to the intermediate result
		sFCU.sBrakes[(Luint32)eBrake].sMLP.s32ADC_Minus_Zero = s32Temp;

	}
	else
	{
		//error
	}
}

#endif //C_LOCALDEF__LCCM655__ENABLE_BRAKES
#endif //#if C_LOCALDEF__LCCM655__ENABLE_THIS_MODULE == 1U
//safetys
#ifndef C_LOCALDEF__LCCM655__ENABLE_THIS_MODULE
	#error
#endif
/** @} */
/** @} */
/** @} */

