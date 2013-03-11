/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * \file      CAN_STUB/CAN_STUB.h
 * \author    Pradeep Kadoor
 * \copyright Copyright (c) 2011, Robert Bosch Engineering and Business Solutions. All rights reserved.
 */

#pragma once

#include "VCiViewIF.h"
#include "include/Error.h"
#include "Include/Struct_CAN.h"
#include "Include/BaseDefs.h"
#include "Include/DIL_CommonDefs.h"
#include "DataTypes/Base_WrapperErrorLogger.h"
#include "DataTypes/MsgBufAll_DataTypes.h"
#include "BusEmulation/BusEmulation.h"
#include "BusEmulation/BusEmulation_i.c"
#include "DataTypes/DIL_DataTypes.h"
#include "Utility/Utility_Thread.h"
#include "Utility/Utility.h"
#include "HardwareListing.h"
#include "ChangeRegisters.h"
#include "DIL_Interface/BaseDIL_CAN_Controller.h"

/**
 * Starts code for the state machine
 */
enum State_e
{
	STATE_DRIVER_SELECTED    = 0x0,
	STATE_HW_INTERFACE_LISTED,
	STATE_HW_INTERFACE_SELECTED,
	STATE_CONNECTED
};

/**
 * \brief Buffer to read msg from the PIPE
 *
 * This buffer include CAN Msg + 1 Byte to indicate TX/RX + UINT64 to indicate timestamp
 */
struct tagPIPE_CANMSG
{
	BYTE m_byTxRxFlag;
	UINT64 m_unTimeStamp;
	STCAN_MSG m_sCanMsg;
};

typedef tagPIPE_CANMSG SPIPE_CANMSG;

const int SIZE_PIPE_CANMSG       = sizeof(BYTE) + sizeof(UINT64) + sizeof(STCAN_MSG);
const BYTE SIZE_TIMESTAMP = sizeof(UINT64);

#define MAX_CLIENT_ALLOWED	16
#define MAX_BUFF_ALLOWED	16

/**
 * See CAN_STUB.cpp for the implementation of this class
 */
class CCAN_i_VIEW_DLL : public CWinApp
{
public:
	CCAN_i_VIEW_DLL();

public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};

/**
 * VCI Rx Class
 */
class VCIRx :
	public CVCiViewHostIF
{
public:
	VCIRx(){};
private:
	T_PDU_ERROR RxData(vci_data_record_with_data* VCIRec);
	T_PDU_ERROR RxEvent(vci_event_record_s* VCIEvent);
};

typedef VCIRx* pVCIRx_t;

/**
 * VCI HW (Channel)
 */
class VCI
{
public:
	VCI(	const string&	Name,
		UNUM32		TypeId,
		UNUM32		CAN );
	UNUM32 Id()
	{
		return m_Id;
	}
	std::string& Name()
	{
		return m_Name;
	}
	void VCiIF( CVCiViewIF* VCiIF )
	{
		m_VCiIF = VCiIF;
	}
	CVCiViewIF* VCiIF()
	{
		return m_VCiIF;
	}
	UNUM32 TypeId()
	{
		return m_TypeId;
	}
	UNUM32 CAN()
	{
		return m_CAN;
	}
	std::string& Firmware()
	{
		return m_Firmware;
	}
	/** Connect
	 * \brief Connect to either CAN 0 or CAN 1
	 * \return T_PDU_ERROR
	 * \retval PDU_STATUS_NOERROR		Function call successful.
	 * \retval PDU_ERR_FCT_FAILED		Function call failed.
	 */
	virtual T_PDU_ERROR Connect();
private:
	static UNUM32	m_NextId;
	UNUM32		m_Id;
	CVCiViewIF*	m_VCiIF;
	string		m_Name;
	UNUM32		m_TypeId;
	UNUM32		m_CAN;
	string		m_Firmware;
	UNUM32		m_ControllerState;
	UNUM32		m_TxErrorCounter;
	UNUM32		m_RxErrorCounter;
	UNUM32		m_PeakRxErrorCounter;
	UNUM32		m_PeakTxErrorCounter;
};

typedef VCI* pVCI_t;

/**
 * Client and Client Buffer set
 */
class Client
{
	typedef set<CBaseCANBufFSE*> Buffers_t;
public:
	Client() :
		m_ClientID(m_NextClientID++){};
	Client(		const string&	Name ) :
		m_ClientID( m_NextClientID++ ),
		m_ClientName( Name ){};

	BOOL BufferExists(
		CBaseCANBufFSE*	pBuf);

	BOOL AddClientBuffer(
		CBaseCANBufFSE*	Buffer);

	void WriteClientBuffers(
		STCANDATA& CanData);

	BOOL RemoveClientBuffer(
		CBaseCANBufFSE*	Buffer);

	void RemoveClientBuffers();

	static DWORD		m_NextClientID;
	DWORD			m_ClientID;
	Buffers_t		m_ClientBuf;
	string			m_ClientName;
};

typedef Client* pClient_t;

/**
 * CDIL_CAN_i_VIEW class definition
 */

class CDIL_CAN_i_VIEW :
	public CBaseDIL_CAN_Controller,
	public CmDNSIF,
	public CVCiViewHostIF
{
	typedef tr1::array<pVCI_t,defNO_OF_CHANNELS> Channels_t;
	typedef std::map<UNUM32, pVCI_t>	pVCIMap_t;
	typedef std::map<DWORD,pClient_t>	pClientMap_t;
public:
	CDIL_CAN_i_VIEW() :
		m_CreateCCommTCP(NULL),
		m_CreateCVCiViewIF(NULL),
		m_hDll( NULL ),
		m_CurrState(STATE_DRIVER_SELECTED),
		m_hOwnerWnd(NULL),
		m_nChannels(0)
		{
			m_Channel.assign(NULL);
		};
	/* STARTS IMPLEMENTATION OF THE INTERFACE FUNCTIONS... */
	HRESULT CAN_PerformInitOperations(void);
	HRESULT CAN_PerformClosureOperations(void);
	HRESULT CAN_GetTimeModeMapping(SYSTEMTIME& CurrSysTime, UINT64& TimeStamp, LARGE_INTEGER* QueryTickCount = NULL);
	HRESULT CAN_ListHwInterfaces(INTERFACE_HW_LIST& sSelHwInterface, INT& nCount);
	HRESULT CAN_SelectHwInterface(const INTERFACE_HW_LIST& sSelHwInterface, INT nCount);
	HRESULT CAN_DeselectHwInterface(void);
	HRESULT CAN_DisplayConfigDlg(PSCONTROLLER_DETAILS InitData, int& Length);
	HRESULT CAN_SetConfigData(PSCONTROLLER_DETAILS InitData, int Length);
	HRESULT CAN_StartHardware(void);
	HRESULT CAN_StopHardware(void);
	HRESULT CAN_ResetHardware(void);
	HRESULT CAN_GetCurrStatus(s_STATUSMSG& StatusData);
	HRESULT CAN_SendMsg(DWORD dwClientID, const STCAN_MSG& sCanTxMsg);
	HRESULT CAN_GetLastErrorString(string& acErrorStr);
	HRESULT CAN_GetControllerParams(LONG& lParam, UINT nChannel, ECONTR_PARAM eContrParam);
	HRESULT CAN_SetControllerParams(int nValue, ECONTR_PARAM eContrparam);
	HRESULT CAN_GetErrorCount(SERROR_CNT& sErrorCnt, UINT nChannel, ECONTR_PARAM eContrParam);

	// Specific function set
	HRESULT CAN_SetAppParams(HWND hWndOwner, Base_WrapperErrorLogger* pILog);
	HRESULT CAN_ManageMsgBuf(BYTE bAction, DWORD ClientID, CBaseCANBufFSE* pBufObj);
	HRESULT CAN_RegisterClient(BOOL bRegister, DWORD& ClientID, char* pacClientName);
	HRESULT CAN_GetCntrlStatus(const HANDLE& hEvent, UINT& unCntrlStatus);
	HRESULT CAN_LoadDriverLibrary(void);
	HRESULT CAN_UnloadDriverLibrary(void);
	
private:
	void GetSystemErrorString();
	void mDNSResolver( mDNS_Event_t	State,
			std::string&	Service,
			std::string&	Type,
			std::string&	Hostname,
			std::string&	Address,
			UNUM16		Port,
			std::map< std::string, std::string >& TxtList );
	int ListHardwareInterfaces(
			HWND		hParent,
			DWORD		/*dwDriver*/,
			INTERFACE_HW*	psInterfaces,
			int*		pnSelList,
			int&		nCount );

	T_PDU_ERROR RxData(
			UNUM32				Id,
			vci_data_record_with_data*	VCIRec);
	T_PDU_ERROR RxEvent(
			UNUM32				Id,
			vci_event_record_s*		VCIEvent );

	pClient_t GetClient(
			string		ClientName );

	pClient_t GetClient(
			DWORD		ClientID );

	BOOL AddClient(
			pClient_t	Client );

	BOOL RemoveClient(
			DWORD		ClientId);

	CreatemDNS_t			m_CreatemDNS;
	CreateCCommTCP_t		m_CreateCCommTCP;
	CreateCVCiViewIF_t		m_CreateCVCiViewIF;
	HINSTANCE			m_hDll;
	CRITICAL_SECTION		m_Mutex;
	CmDNS*				m_CmDNS;
	State_e				m_CurrState;
	string				m_Err;
	HWND				m_hOwnerWnd;
	pVCIMap_t			m_VCI;
	UINT				m_nChannels;
	Channels_t			m_Channel;
	pClientMap_t			m_Clients;
	INT				m_SelectedVCI[CHANNEL_ALLOWED];
	/**
	 * Timer variables
	 */
	SYSTEMTIME			m_CurrSysTime;
	UINT64				m_TimeStamp;
	LARGE_INTEGER			m_QueryTickCount;
	LARGE_INTEGER			m_lnFrequency;
};
