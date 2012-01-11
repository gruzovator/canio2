/*****************************************************************************
 IXXAT Automation GmbH
******************************************************************************

 File    : VCINPL.H
 Summary : Native VCI Programming Library.

 Date    : 2005-04-20
 Author  : Hartmut Heim

 Compiler: MSVC

******************************************************************************
 all rights reserved
*****************************************************************************/

#ifndef _VCINPL_H_
#define _VCINPL_H_

#include <vcisdk.h>
#include <pshpack4.h>


/*##########################################################################*/
/*##                                                                      ##*/
/*##   exported API functions                                             ##*/
/*##                                                                      ##*/
/*##########################################################################*/

/*****************************************************************************
 * general VCI functions
 ****************************************************************************/

EXTERN_C HRESULT VCIAPI
  vciInitialize ( void );

EXTERN_C void VCIAPI
  vciFormatError( IN  HRESULT hrError,
                  OUT PCHAR   pszText,
                  IN  UINT32  dwSize );

EXTERN_C void VCIAPI
  vciDisplayError( IN HWND    hwndParent,
                   IN PCHAR   pszCaption,
                   IN HRESULT hrError );

EXTERN_C HRESULT VCIAPI
  vciGetVersion( OUT PUINT32 pdwMajorVersion,
                 OUT PUINT32 pdwMinorVersion );

EXTERN_C HRESULT VCIAPI
  vciLuidToChar( IN  REFVCIID rVciid,
                 OUT PCHAR    pszLuid,
                 IN  LONG     cbSize );

EXTERN_C HRESULT VCIAPI
  vciCharToLuid( IN  PCHAR  pszLuid,
                 OUT PVCIID pVciid );

EXTERN_C HRESULT VCIAPI
  vciGuidToChar( IN  REFGUID rGuid,
                 OUT PCHAR   pszGuid,
                 IN  LONG    cbSize );

EXTERN_C HRESULT VCIAPI
  vciCharToGuid( IN  PCHAR pszGuid,
                 OUT PGUID pGuid );


/*****************************************************************************
 * event specific functions
 ****************************************************************************/

EXTERN_C HRESULT VCIAPI
  vciEventCreate( IN  BOOL    fManReset,
                  IN  BOOL    fInitState,
                  OUT PHANDLE phEvent );

EXTERN_C HRESULT VCIAPI
  vciEventDelete( IN HANDLE hEvent );

EXTERN_C HRESULT VCIAPI
  vciEventSignal( IN HANDLE hEvent );

EXTERN_C HRESULT VCIAPI
  vciEventReset( HANDLE hEvent );

EXTERN_C HRESULT VCIAPI
  vciEventWaitFor( IN HANDLE hEvent,
                   IN UINT32 dwMsTimeout );


/*****************************************************************************
 * device manager specific functions
 ****************************************************************************/

EXTERN_C HRESULT VCIAPI
  vciEnumDeviceOpen( OUT PHANDLE hEnum );

EXTERN_C HRESULT VCIAPI
  vciEnumDeviceClose ( IN HANDLE hEnum );

EXTERN_C HRESULT VCIAPI
  vciEnumDeviceNext( IN  HANDLE         hEnum,
                     OUT PVCIDEVICEINFO pInfo );

EXTERN_C HRESULT VCIAPI
  vciEnumDeviceReset( IN HANDLE hEnum );

EXTERN_C HRESULT VCIAPI
  vciEnumDeviceWaitEvent( IN HANDLE hEnum,
                          IN UINT32 dwMsTimeout );

EXTERN_C HRESULT VCIAPI
  vciFindDeviceByHwid( IN  REFGUID rHwid,
                       OUT PVCIID  pVciid );

EXTERN_C HRESULT VCIAPI
  vciFindDeviceByClass( IN  REFGUID rClass,
                        IN  UINT32  dwInst,
                        OUT PVCIID  pVciid );

EXTERN_C HRESULT VCIAPI
  vciSelectDeviceDlg( IN  HWND   hwndParent,
                      OUT PVCIID pVciid );


/*****************************************************************************
 * device specific functions
 ****************************************************************************/

EXTERN_C HRESULT VCIAPI
  vciDeviceOpen( IN  REFVCIID rVciid,
                 OUT PHANDLE  phDevice );

EXTERN_C HRESULT VCIAPI
  vciDeviceOpenDlg( IN  HWND    hwndParent,
                    OUT PHANDLE phDevice );

EXTERN_C HRESULT VCIAPI
  vciDeviceClose( IN HANDLE hDevice );

EXTERN_C HRESULT VCIAPI
  vciDeviceGetInfo( IN  HANDLE         hDevice,
                    OUT PVCIDEVICEINFO pInfo );

EXTERN_C HRESULT VCIAPI
  vciDeviceGetCaps( IN  HANDLE         hDevice,
                    OUT PVCIDEVICECAPS pCaps );


/*****************************************************************************
 * CAN controller specific functions
 ****************************************************************************/

EXTERN_C HRESULT VCIAPI
  canControlOpen( IN  HANDLE  hDevice,
                  IN  UINT32  dwCanNo,
                  OUT PHANDLE phCanCtl );

EXTERN_C HRESULT VCIAPI
  canControlClose( IN HANDLE hCanCtl );

EXTERN_C HRESULT VCIAPI
  canControlGetCaps( IN  HANDLE           hCanCtl,
                     OUT PCANCAPABILITIES pCanCaps );

EXTERN_C HRESULT VCIAPI
  canControlGetStatus( IN  HANDLE         hCanCtl,
                       OUT PCANLINESTATUS pStatus );

EXTERN_C HRESULT VCIAPI
  canControlDetectBitrate( IN  HANDLE  hCanCtl,
                           IN  UINT16  wMsTimeout,
                           IN  UINT32  dwCount,
                           IN  PUINT8  pabBtr0,
                           IN  PUINT8  pabBtr1,
                           OUT PINT32  plIndex );

EXTERN_C HRESULT VCIAPI
  canControlInitialize( IN HANDLE hCanCtl,
                        IN UINT8  bMode,
                        IN UINT8  bBtr0,
                        IN UINT8  bBtr1 );

EXTERN_C HRESULT VCIAPI
  canControlReset( IN HANDLE hCanCtl );

EXTERN_C HRESULT VCIAPI
  canControlStart( IN HANDLE hCanCtl,
                   IN BOOL   fStart );

EXTERN_C HRESULT VCIAPI
  canControlSetAccFilter( IN HANDLE hCanCtl,
                          IN BOOL   fExtend,
                          IN UINT32 dwCode,
                          IN UINT32 dwMask );

EXTERN_C HRESULT VCIAPI
  canControlAddFilterIds( IN HANDLE hCanCtl,
                          IN BOOL   fExtend,
                          IN UINT32 dwCode,
                          IN UINT32 dwMask );

EXTERN_C HRESULT VCIAPI
  canControlRemFilterIds( IN HANDLE hCanCtl,
                          IN BOOL   fExtend,
                          IN UINT32 dwCode,
                          IN UINT32 dwMask );


/*****************************************************************************
 * CAN message channel specific functions
 ****************************************************************************/

EXTERN_C HRESULT VCIAPI
  canChannelOpen( IN  HANDLE  hDevice,
                  IN  UINT32  dwCanNo,
                  IN  BOOL    fExclusive,
                  OUT PHANDLE phCanChn );

EXTERN_C HRESULT VCIAPI
  canChannelClose( IN HANDLE hCanChn );

EXTERN_C HRESULT VCIAPI
  canChannelGetCaps( IN  HANDLE           hCanChn,
                     OUT PCANCAPABILITIES pCanCaps );

EXTERN_C HRESULT VCIAPI
  canChannelGetStatus( IN  HANDLE         hCanChn,
                       OUT PCANCHANSTATUS pStatus );

EXTERN_C HRESULT VCIAPI
  canChannelInitialize( IN HANDLE hCanChn,
                        IN UINT16 wRxFifoSize,
                        IN UINT16 wRxThreshold,
                        IN UINT16 wTxFifoSize,
                        IN UINT16 wTxThreshold );

EXTERN_C HRESULT VCIAPI
  canChannelActivate( IN HANDLE hCanChn,
                      IN BOOL   fEnable );

EXTERN_C HRESULT VCIAPI
  canChannelPeekMessage( IN  HANDLE  hCanChn,
                         OUT PCANMSG pCanMsg );

EXTERN_C HRESULT VCIAPI
  canChannelPostMessage( IN HANDLE  hCanChn,
                         IN PCANMSG pCanMsg );

EXTERN_C HRESULT VCIAPI
  canChannelWaitRxEvent( IN HANDLE hCanChn,
                         IN UINT32 dwMsTimeout );

EXTERN_C HRESULT VCIAPI
  canChannelWaitTxEvent( IN HANDLE hCanChn,
                         IN UINT32 dwMsTimeout );

EXTERN_C HRESULT VCIAPI
  canChannelReadMessage( IN  HANDLE  hCanChn,
                         IN  UINT32  dwMsTimeout,
                         OUT PCANMSG pCanMsg );

EXTERN_C HRESULT VCIAPI
  canChannelSendMessage( IN HANDLE  hCanChn,
                         IN UINT32  dwMsTimeout,
                         IN PCANMSG pCanMsg );


/*****************************************************************************
 * cyclic CAN message scheduler specific functions
 ****************************************************************************/

EXTERN_C HRESULT VCIAPI
  canSchedulerOpen( IN  HANDLE  hDevice,
                    IN  UINT32  dwCanNo,
                    OUT PHANDLE phCanShd );

EXTERN_C HRESULT VCIAPI
  canSchedulerClose( IN HANDLE hCanShd );

EXTERN_C HRESULT VCIAPI
  canSchedulerGetCaps( IN  HANDLE           hCanShd,
                       OUT PCANCAPABILITIES pCanCaps );

EXTERN_C HRESULT VCIAPI
  canSchedulerGetStatus( IN  HANDLE              hCanShd,
                         OUT PCANSCHEDULERSTATUS pStatus );

EXTERN_C HRESULT VCIAPI
  canSchedulerActivate( IN HANDLE hCanShd,
                        IN BOOL   fEnable );

EXTERN_C HRESULT VCIAPI
  canSchedulerReset( IN HANDLE hCanShd );

EXTERN_C HRESULT VCIAPI
  canSchedulerAddMessage( IN  HANDLE          hCanShd,
                          IN  PCANCYCLICTXMSG pMessage,
                          OUT PUINT32         pdwIndex );

EXTERN_C HRESULT VCIAPI
  canSchedulerRemMessage( IN HANDLE hCanShd,
                          IN UINT32 dwIndex );

EXTERN_C HRESULT VCIAPI
  canSchedulerStartMessage( IN HANDLE hCanShd,
                            IN UINT32 dwIndex,
                            IN UINT16 wRepeat );

EXTERN_C HRESULT VCIAPI
  canSchedulerStopMessage( IN HANDLE hCanShd,
                           IN UINT32 dwIndex );


/*****************************************************************************
 * LIN controller specific functions
 ****************************************************************************/

EXTERN_C HRESULT VCIAPI
  linControlOpen( IN  HANDLE  hDevice,
                  IN  UINT32  dwLinNo,
                  OUT PHANDLE phLinCtl );

EXTERN_C HRESULT VCIAPI
  linControlClose( IN HANDLE hLinCtl );

EXTERN_C HRESULT VCIAPI
  linControlGetCaps( IN  HANDLE           hLinCtl,
                     OUT PLINCAPABILITIES pLinCaps );

EXTERN_C HRESULT VCIAPI
  linControlGetStatus( IN  HANDLE         hLinCtl,
                       OUT PLINLINESTATUS pStatus );

EXTERN_C HRESULT VCIAPI
  linControlInitialize( IN HANDLE hLinCtl,
                        IN UINT8  bMode,
                        IN UINT16 wBitrate );

EXTERN_C HRESULT VCIAPI
  linControlReset( IN HANDLE hLinCtl );

EXTERN_C HRESULT VCIAPI
  linControlStart( IN HANDLE hLinCtl,
                   IN BOOL   fStart );

EXTERN_C HRESULT VCIAPI
  linControlWriteMessage( IN HANDLE  hLinCtl,
                          IN BOOL    fSend,
                          IN PLINMSG pLinMsg );


/*****************************************************************************
 * LIN message monitor specific functions
 ****************************************************************************/

EXTERN_C HRESULT VCIAPI
  linMonitorOpen( IN  HANDLE  hDevice,
                  IN  UINT32  dwLinNo,
                  IN  BOOL    fExclusive,
                  OUT PHANDLE phLinMon );

EXTERN_C HRESULT VCIAPI
  linMonitorClose( IN HANDLE hLinMon );

EXTERN_C HRESULT VCIAPI
  linMonitorGetCaps( IN  HANDLE           hLinMon,
                     OUT PLINCAPABILITIES pLinCaps );

EXTERN_C HRESULT VCIAPI
  linMonitorGetStatus( IN  HANDLE            hLinMon,
                       OUT PLINMONITORSTATUS pStatus );

EXTERN_C HRESULT VCIAPI
  linMonitorInitialize( IN HANDLE hLinMon,
                        IN UINT16 wFifoSize,
                        IN UINT16 wThreshold );

EXTERN_C HRESULT VCIAPI
  linMonitorActivate( IN HANDLE hLinMon,
                      IN BOOL   fEnable );

EXTERN_C HRESULT VCIAPI
  linMonitorPeekMessage( IN  HANDLE  hLinMon,
                         OUT PLINMSG pLinMsg );

EXTERN_C HRESULT VCIAPI
  linMonitorWaitRxEvent( IN HANDLE hLinMon,
                         IN UINT32 dwMsTimeout );

EXTERN_C HRESULT VCIAPI
  linMonitorReadMessage( IN  HANDLE  hLinMon,
                         IN  UINT32  dwMsTimeout,
                         OUT PLINMSG pLinMsg );


#include <poppack.h>
#endif //_VCINPL_H_
