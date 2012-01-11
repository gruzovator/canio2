/*****************************************************************************
 IXXAT Automation GmbH
******************************************************************************

 File    : VCITYPE.H
 Summary : VCI specific constants, data types and macros.

 Date    : 2003-09-04
 Author  : Hartmut Heim

 Compiler: MSVC

 Remarks : This file is shared between user and kernel mode components.
           Request a kernel mode developer before changing the contents
           of this file.

******************************************************************************
 all rights reserved
*****************************************************************************/

#ifndef _VCITYPE_H_
#define _VCITYPE_H_

#include <stdtype.h>
#include <cantype.h>
#include <lintype.h>
#include <pshpack1.h>

/*****************************************************************************
 * unique VCI object identifier
 ****************************************************************************/

#include <pshpack8.h>

typedef union _VCIID
{
  LUID  AsLuid;
  INT64 AsInt64;
} VCIID, *PVCIID;

#include <poppack.h>

#if defined(__cplusplus)
typedef const VCIID& REFVCIID;
#else
typedef const VCIID* const REFVCIID;
#endif

/*****************************************************************************
 * VCI version information
 ****************************************************************************/

typedef struct _VCIVERSIONINFO
{
  UINT32 VciMajorVersion;    // major VCI server version number
  UINT32 VciMinorVersion;    // minor VCI server version number
  UINT32 OsMajorVersion;     // major os version number
  UINT32 OsMinorVersion;     // minor os version number
  UINT32 OsBuildNumber;      // os build number
  UINT32 OsPlatformId;       // os platform id
} VCIVERSIONINFO, *PVCIVERSIONINFO;

typedef struct _VCIVERSIONINFO2
{
  UINT32 VciMajorVersion;    // major VCI server version number
  UINT32 VciMinorVersion;    // minor VCI server version number
  UINT32 VciReleaseVersion;  // release VCI server version number
  UINT32 VciBuildVersion;    // build VCI server version number
  UINT32 OsMajorVersion;     // major os version number
  UINT32 OsMinorVersion;     // minor os version number
  UINT32 OsBuildNumber;      // os build number
  UINT32 OsPlatformId;       // os platform id
} VCIVERSIONINFO2, *PVCIVERSIONINFO2;

/*****************************************************************************
 * VCI driver information
 ****************************************************************************/

typedef struct _VCIDRIVERINFO
{
  VCIID  VciObjectId;    // unique VCI object identifier
  GUID   DriverClass;    // driver class identifier
  UINT16 MajorVersion;   // minor driver version number
  UINT16 MinorVersion;   // minor driver version number
  UINT16 ReleaseVersion; // build driver version number
  UINT16 BuildVersion;   // build driver version number
} VCIDRIVERINFO, *PVCIDRIVERINFO;


/*****************************************************************************
 * VCI device information
 ****************************************************************************/

typedef struct _VCIDEVICEINFO
{
  VCIID  VciObjectId;          // unique VCI object identifier
  GUID   DeviceClass;          // device class identifier

  UINT8  DriverMajorVersion;   // major driver version number
  UINT8  DriverMinorVersion;   // minor driver version number 
  UINT16 DriverBuildVersion;   // build driver version number

  UINT8  HardwareBranchVersion;// branch hardware version number
  UINT8  HardwareMajorVersion; // major hardware version number
  UINT8  HardwareMinorVersion; // minor hardware version number
  UINT8  HardwareBuildVersion; // build hardware version number

  union _UniqueHardwareId      // unique hardware identifier
  {
    CHAR AsChar[16];
    GUID AsGuid;
  } UniqueHardwareId;

  CHAR Description [128];       // device description (e.g: "PC-I04-PCI")
  CHAR Manufacturer[126];       // device manufacturer (e.g: "IXXAT Automation")

  UINT16 DriverReleaseVersion; // minor driver version number
} VCIDEVICEINFO, *PVCIDEVICEINFO;


/*****************************************************************************
 * VCI bus controller types
 *
 * +-------+--------+
 * | 15..8 | 7 .. 0 |
 * +-------+--------+
 * Bit 15..8 : bus type (0 is reserved)
 * Bit  7..0 : controller type
 ****************************************************************************/

#define VCI_BUS_RES  0 // reserved
#define VCI_BUS_CAN  1 // CAN
#define VCI_BUS_LIN  2 // LIN
#define VCI_BUS_FLX  3 // FlexRay

#define VCI_BUS_CTRL(Bus,Ctrl) (UINT16) ( ((Bus)<<8) | (Ctrl) )
#define VCI_BUS_TYPE(BusCtrl)  (UINT8)  ( ((BusCtrl) >> 8) & 0x00FF )
#define VCI_CTL_TYPE(BusCtrl)  (UINT8)  ( ((BusCtrl) >> 0) & 0x00FF )

//------------------------------------------------------------------------
// CAN controller types
//------------------------------------------------------------------------

// unknown CAN controller
#define VCI_CAN_UNKNOWN      VCI_BUS_CTRL(VCI_BUS_CAN, CAN_CTRL_UNKNOWN)

// Intel 82527
#define VCI_CAN_82527        VCI_BUS_CTRL(VCI_BUS_CAN, CAN_CTRL_82527)

// Intel 82C200
#define VCI_CAN_82C200       VCI_BUS_CTRL(VCI_BUS_CAN, CAN_CTRL_82C200)

// Intel 82C90
#define VCI_CAN_82C90        VCI_BUS_CTRL(VCI_BUS_CAN, CAN_CTRL_82C90)

// Intel 82C92
#define VCI_CAN_82C92        VCI_BUS_CTRL(VCI_BUS_CAN, CAN_CTRL_82C92)

// Philips SJA 1000
#define VCI_CAN_SJA1000      VCI_BUS_CTRL(VCI_BUS_CAN, CAN_CTRL_SJA1000)

// Infinion 82C900 (TwinCAN)
#define VCI_CAN_82C900       VCI_BUS_CTRL(VCI_BUS_CAN, CAN_CTRL_82C900)

// Motorola TOUCAN
#define VCI_CAN_TOUCAN       VCI_BUS_CTRL(VCI_BUS_CAN, CAN_CTRL_TOUCAN)


//------------------------------------------------------------------------
// LIN controller types
//------------------------------------------------------------------------

// unknown LIN controller
#define VCI_LIN_UNKNOWN      VCI_BUS_CTRL(VCI_BUS_LIN, LIN_CTRL_UNKNOWN)


//------------------------------------------------------------------------
// FlexRay controller types
//------------------------------------------------------------------------

// unknown FlexRay controller
#define VCI_FLX_UNKNOWN      VCI_BUS_CTRL(VCI_BUS_FLX, 0)


/*****************************************************************************
 * VCI device capabilities
 ****************************************************************************/

#define VCI_MAX_BUSCTRL 32   // maximum number of supported bus controllers

typedef struct _VCIDEVICECAPS
{
  UINT16 BusCtrlCount;       // number of supported bus controllers
  UINT16 BusCtrlTypes[32];   // array of supported bus controllers
} VCIDEVICECAPS, *PVCIDEVICECAPS;


#include <poppack.h>
#endif //_VCITYPE_H_
