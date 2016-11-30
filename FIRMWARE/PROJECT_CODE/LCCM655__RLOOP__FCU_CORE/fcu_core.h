/**
 * @file		FCU_CORE.H
 * @brief		Main header for the FCU
 * @author		Lachlan Grogan
 * @copyright	rLoop Inc.
 * @st_fileID	LCCM655R0.FILE.001
 */

#ifndef _FCU_CORE_H_
#define _FCU_CORE_H_
	#include <localdef.h>
	#if C_LOCALDEF__LCCM655__ENABLE_THIS_MODULE == 1U

		//state machine types
		#include <LCCM655__RLOOP__FCU_CORE/fcu_core__types.h>
		#include <LCCM655__RLOOP__FCU_CORE/fcu_core__defines.h>
		#include <LCCM655__RLOOP__FCU_CORE/fcu_core__enums.h>
		#include <LCCM655__RLOOP__FCU_CORE/PI_COMMS/fcu__pi_comms__types.h>

		#include <LCCM655__RLOOP__FCU_CORE/fcu_core__fault_flags.h>
		#include <LCCM655__RLOOP__FCU_CORE/BRAKES/fcu__brakes__fault_flags.h>
		#include <LCCM655__RLOOP__FCU_CORE/ACCELEROMETERS/fcu__accel__fault_flags.h>

		//for software fault tree handling
		#include <MULTICORE/LCCM284__MULTICORE__FAULT_TREE/fault_tree__public.h>

		/*******************************************************************************
		Defines
		*******************************************************************************/
		#define C_MLP__MAX_AVERAGE_SIZE				(8U)


		/*******************************************************************************
		Structures
		*******************************************************************************/
		/** main flight control structure */
		struct _strFCU
		{
			/** Structure guard 1*/
			Luint32 u32Guard1;

			/** The main state machine for run mode */
			E_FCU__RUN_STATE_T eRunState;

			/** The init statemachine */
			E_FCU__INIT_STATE_TYPES eInitStates;

			/** The brakes state machine */
			E_FCU_BRAKES__STATES_T eBrakeStates;

			/** Fault handling subsystem */
			struct
			{
				/** top level fault tree subsystem for the flight controller */
				FAULT_TREE__PUBLIC_T sTopLevel;

				/** Accel subsystem faults */
				FAULT_TREE__PUBLIC_T sAccel;


			}sFaults;


			/** Brake Substructure */
			struct
			{
				/** Limit switch structure
				 * There are two limit switches per brake assy
				 */
				struct
				{

					/** An edge was captured on the interrupt subsystem
					 * Its up to some other layer to clear the flag once its used.
					 */
					Luint8 u8EdgeSeen;

					#if C_LOCALDEF__LCCM655__ENABLE_DEBUG_BRAKES == 1U
						/** Debugs how many interrupts have been received
						 */
						Luint32 u8EdgeSeenCnt;
					#endif

					/** The program index for N2HET, even if not used on both channels */
					Luint16 u16N2HET_Prog;

					/** The current state of the switch */
					E_FCU__SWITCH_STATE_T eSwitchState;


				}sLimits[BRAKE_SW__MAX_SWITCHES];

				/** Linear position sensor detail */
				struct
				{
					/** This is the sample from the ADC converter */
					Luint16 u16ADC_Sample;

					/** This is the filtered result from the raw sample */
					Luint16 u16ADC_FilteredSample;

					/** The zero value when the brakes are fully retracted */
					Luint16 u16ADC_Zero;

					/** Current ADC span to convert ADC units into engineering units */
					Lfloat32 f32SystemSpan;

					/** ADC value - Zero Value */
					Lint32 s32ADC_Minus_Zero;

					/** Percent of braking from 0.0 to 100.0*/
					Lfloat32 f32BrakePosition_Percent;

					/** Average Counter	for MLP filter function				 */
					Luint16 u16AverageCounter;

					/** Average Array for MLP filter function				 */
					Luint16 u16AverageArray[C_MLP__MAX_AVERAGE_SIZE];

					/** Lowest MLP Value				 */
					Luint16 lowest_value;

					/** Highest MLP Value				 */
					Luint16 highest_value;

					#if C_LOCALDEF__LCCM655__ENABLE_DEBUG_BRAKES == 1U
						/** Debug how many times we get a zero */
						Luint32 zero_count;
					#endif

					/** Temporary use; identify when startup sequence has ended and mlp won't return 0. */
					Luint8 u8Running;
				}sMLP;


				/** Movement Planner */
				struct
				{

					/** The required linear veloc for the move */
					Lint32 s32LinearVeloc;

					/** The required linear accel for the move */
					Lint32 s32LinearAccel;

					/** The position that we have requested the brake to move to */
					Lint32 s32MoveToPos;

					/** The current position of the lead screw */
					Lint32 s32currentPos;

				}sMove;


				/** Current Details */
				struct
				{

					/** the screw position in mm */
					Lfloat32 f32ScrewPos_mm;

					/** I-Beam distance in mm */
					Lfloat32 f32IBeam_mm;

					/** I-Beam distance in mm */
					Lfloat32 f32MLP_mm;


				}sCurrent;


				/** individual brake fault flags */
				FAULT_TREE__PUBLIC_T sFaultFlags;

				Luint8 u8BrakeSWErr;

			}sBrakes[C_FCU__NUM_BRAKES];


			/** Accel subsystem */
			struct
			{

				/** individual accel channels */
				struct
				{
					/** most recent recorded sample from the Accel in raw units */
					Lint16 s16LastSample[3];

					/** Last sample of the G-Force*/
					Lfloat32 f32LastG[3];

				}sChannels[C_LOCALDEF__LCCM418__NUM_DEVICES];

			}sAccel;

			/** Pi Comms Layer */
			struct
			{

				//the current state
				E_FCU_PICOM__STATE_T eState;

				/** 100ms timer tick */
				Luint8 u8100MS_Timer;

			}sPiComms;


			/** Pusher Interface Layer */
			struct
			{
				/** Guard variable 1*/
				Luint32 u32Guard1;

				/** The pusher subsystem state machine */
				E_FCU_PUSHER__STATES_T eState;

				/** Interlock switch status */
				Luint8 u8Pusher_Status;



				/** Timer of 10ms ticks used for switch state timing */
				Luint32 u32SwtichTimer;

				/** Switch interfaces */
				struct
				{
					/** N2HET Program index for edge interrupts*/
					Luint16 u16N2HET_Prog;

					/** The state of the switch based on the last sample */
					Luint8 u8SwitchState;

					/** Edge interrupt has occurred, meaning there has been a switch transition */
					Luint8 u8EdgeFlag;

				}sSwitches[2];

				/** Guard variable 2*/
				Luint32 u32Guard2;

			}sPusher;

			/** Overall structure for the laser interfaces */
			struct
			{
				/** state machine for processing the OptoNCDT systems */
				E_FCU_OPTOLASER__STATE_T eOptoNCDTState;

				/** A 100ms counter to wait until the lasers have powered up*/
				Luint32 u32LaserPOR_Counter;

				/** The opto NCDT laser interfaces */
				struct
				{
					/** RX byte state machine */
					E_OPTONCDT__RX_STATE_T eRxState;

					/** A new packet is available for distance processing */
					Luint8 u8NewPacket;

					/** Array to hold new bytes received */
					Luint8 u8NewByteArray[3];

					/** The most recent distance*/
					Lfloat32 f32Distance;

					/** New distance has been measured, other layer to clear it */
					Luint8 u8NewDistanceAvail;

				}sOptoLaser[C_LOCALDEF__LCCM655__NUM_LASER_OPTONCDT];

			}sLasers;

			/** Ethernet comms structure */
			struct
			{
				/** our hardware MAC */
				Luint8 u8MACAddx[6];

				/** our locally assigned IP*/
				Luint8 u8IPAddx[4];

			}sEthernet;


			/** Structure guard 2*/
			Luint32 u32Guard2;
			

			/** Input data for throttle layer */
			// (added by @gsweriduk on 23 NOV 2016)

			struct strThrottleInterfaceData
			{
				// Ground Station command
				E_GS_COMMANDS eGS_Command;

				// Flight Control Unit mode
				E_FCU_MODES eFCU_Mode;

				// throttle command units (0 for RPM*10, 1 for Percent), needs to be sent by GS
				Luint8 u8CommandUnits;

				// speeds of HE1 to HE8
				Luint16 u16HE_Speeds[8];

				// duration of ramp command *** ASSUMING throttleStartRampDuration IS IN UNITS of MILLISECONDS ***
				Luint16 u16throttleStartRampDuration;

				// maximum speed of HEs in RPM*10
				Luint16 u16HE_MAX_SPD;

				// minimum speed of HEs in RPM*10
				Luint16 u16HE_MIN_SPD;

				// HE speeds for static hover
				Luint16 u16rpmHEStaticHoveringSpeed;

				// HE speeds for standby mode
				Luint16 u16maxRunModeStandbySpeed;

				// Throttle command values:
				// [0] contains a command for all HEs, [1] - [8] contain commands for individual HEs
				Luint16 u16ThrottleCommands[9];

				// Number of the HE being given a command:
				// A value of 0 signifies all HEs, 1 - 8 indicates a specific HE
				Luint8 u8EngineNumber;

				// state variable
				E_THROTTLE_STATES_T eState;

				// timer state
				Luint8 u8100MS_Timer;

			} sThrottle;


		};

		/*******************************************************************************
		Function Prototypes
		*******************************************************************************/
		//core
		DLL_DECLARATION void vFCU__Init(void);
		DLL_DECLARATION void vFCU__Process(void);
		void vFCU__RTI_100MS_ISR(void);
		void vFCU__RTI_10MS_ISR(void);

		//network
		void vFCU_NET__Init(void);
		void vFCU_NET__Process(void);
		Luint8 u8FCU_NET__Is_LinkUp(void);
		void vFCU_NET_RX__RxUDP(Luint8 * pu8Buffer, Luint16 u16Length, Luint16 u16DestPort);
		void vFCU_NET_RX__RxSafeUDP(Luint8 *pu8Payload, Luint16 u16PayloadLength, Luint16 ePacketType, Luint16 u16DestPort, Luint16 u16Fault);

		//fault handling layer
		void vFCU_FAULTS__Init(void);
		void vFCU_FAULTS__Process(void);
		Luint8 u8FCU_FAULTS__Get_IsFault(void);
		Luint32 u32FCU_FAULTS__Get_FaultFlags(void);

		//main state machine
		void vFCU_MAINSM__Init(void);
		void vFCU_MAINSM__Process(void);

		//lasers for OptoNCDT inerface
		void vFCU_LASEROPTO__Init(void);
		void vFCU_LASEROPTO__Process(void);
		Lfloat32 f32FCU_LASEROPTO__Get_Distance(Luint8 u8LaserIndex);
		void vFCU_LASEROPTO__100MS_ISR(void);

		//pi comms
		void vFCU_PICOMMS__Init(void);
		void vFCU_PICOMMS__Process(void);
		void vFCU_PICOMMS__100MS_ISR(void);

		//brakes
		void vFCU_BRAKES__Init(void);
		void vFCU_BRAKES__Process(void);
		void vFCU_BRAKES__Move_IBeam_Distance_mm(Luint32 u32Distance);
		Lfloat32 f32FCU_BRAKES__Get_ScrewPos(E_FCU__BRAKE_INDEX_T eBrake);
		E_FCU__SWITCH_STATE_T eFCU_BRAKES__Get_SwtichState(E_FCU__BRAKE_INDEX_T eBrake, E_FCU__BRAKE_LIMSW_INDEX_T eSwitch);
		Luint16 u16FCU_BRAKES__Get_ADC_Raw(E_FCU__BRAKE_INDEX_T eBrake);
		Lfloat32 f32FCU_BRAKES__Get_IBeam_mm(E_FCU__BRAKE_INDEX_T eBrake);
		Lfloat32 f32FCU_BRAKES__Get_MLP_mm(E_FCU__BRAKE_INDEX_T eBrake);

		//stepper drive
		void vFCU_BRAKES_STEP__Init(void);
		void vFCU_BRAKES_STEP__Process(void);
		void vFCU_BRAKES_STEP__Move(Lint32 s32Brake0Pos, Lint32 s32Brake1Pos);
		Lint32 s32FCU_BRAKES__Get_CurrentPos(E_FCU__BRAKE_INDEX_T eBrake);

		//brake switches
		void vFCU_BRAKES_SW__Init(void);
		void vFCU_BRAKES_SW__Process(void);
		void vFCU_BRAKES_SW__Left_SwitchExtend_ISR(void);
		void vFCU_BRAKES_SW__Left_SwitchRetract_ISR(void);
		void vFCU_BRAKES_SW__Right_SwitchExtend_ISR(void);
		void vFCU_BRAKES_SW__Right_SwitchRetract_ISR(void);
		E_FCU__SWITCH_STATE_T eFCU_BRAKES_SW__Get_Switch(E_FCU__BRAKE_INDEX_T eBrake, E_FCU__BRAKE_LIMSW_INDEX_T eSwitch);
		Luint8 u8FCU_BRAKES_SW__Get_FaultFlag(E_FCU__BRAKE_INDEX_T eBrake);


		//brakes MLP sensor
		void vFCU_BRAKES_MLP__Init(void);
		void vFCU_BRAKES_MLP__Process(void);

		//accelerometer layer
		void vFCU_ACCEL__Init(void);
		void vFCU_ACCEL__Process(void);
		Lint16 s16FCU_ACCEL__Get_LastSample(Luint8 u8Index, Luint8 u8Axis);
		Lfloat32 f32FCU_ACCEL__Get_LastG(Luint8 u8Index, Luint8 u8Axis);

		//Pusher interface
		void vFCU_PUSHER__Init(void);
		void vFCU_PUSHER__Process(void);
		void vFCU_PUSHER__InterlockA_ISR(void);
		void vFCU_PUSHER__InterlockB_ISR(void);
		Luint8 u8FCU_PUSHER__Get_InterlockA(void);
		Luint8 u8FCU_PUSHER__Get_InterlockB(void);
		void vFCU_PUSHER__10MS_ISR(void);
		Luint8 u8FCU_PUSHER__Get_Switch(Luint8 u8Switch);
		Luint8 u8FCU_PUSHER__Get_PusherState(void);


		//ASI interface
		void vFCU_ASI__Init(void);

		//throttle layer
		void vFCU_THROTTLE__Init(void);
		void vFCU_THROTTLE__Process(void);
		Lint16 s16FCU_THROTTLE__Step_Command(void);
		Lint16 s16FCU_THROTTLE__Ramp_Command(void);
		Lint16 s16FCU_THROTTLE__Write_HEx_Throttle_Command_to_DAC(Luint16 u16ThrottleCommand, Luint8 u8EngineNumber);
		Lint16 s16FCU_THROTTLE__Write_All_HE_Throttle_Commands_to_DAC(Luint16 u16ThrottleCommand);
		Lint16 s16FCU_THROTTLE__Hold(void);
		void vFCU_THROTTLE__100MS_ISR(void);


	#endif //#if C_LOCALDEF__LCCM655__ENABLE_THIS_MODULE == 1U
	//safetys
	#ifndef C_LOCALDEF__LCCM655__ENABLE_THIS_MODULE
		#error
	#endif
#endif //_FCU_CORE_H_
