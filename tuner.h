/*
 * tuner.h: SAT>IP plugin for the Video Disk Recorder
 *
 * See the README file for copyright information and how to reach the author.
 *
 */

#ifndef __SATIP_TUNER_H
#define __SATIP_TUNER_H

#include <vdr/thread.h>
#include <vdr/tools.h>

#include "deviceif.h"
#include "rtp.h"
#include "rtcp.h"
#include "rtsp.h"
#include "server.h"
#include "statistics.h"


#if APIVERSNUM >= 20107
#define SATIP_ENHANCED_VECTOR
#endif

class cSatipPid : public cVector<int> {
private:
  int PidIndex(const int &pidP)
  {
#ifdef SATIP_ENHANCED_VECTOR
    return indexOf(pidP);
#else
    for (int i = 0; i < Size(); ++i) {
        if (pidP == At(i))
           return i;
        }
    return -1;
#endif
  }
#ifndef SATIP_ENHANCED_VECTOR
  static int PidCompare(const void *aPidP, const void *bPidP)
  {
    return (*(int*)aPidP - *(int*)bPidP);
  }
#endif

public:
  void RemovePid(const int &pidP)
  {
#ifdef SATIP_ENHANCED_VECTOR
    RemoveElement(pidP);
#else
    int i = PidIndex(pidP);
    if (i >= 0) {
       Remove(i);
       Sort(PidCompare);
       }
#endif
  }
  void AddPid(int pidP)
  {
#ifdef SATIP_ENHANCED_VECTOR
     return AppendUnique(pidP);
#else
    if (PidIndex(pidP) < 0) {
       Append(pidP);
       Sort(PidCompare);
       }
#endif
  }
  cString ListPids(void)
  {
    cString list = "";
    if (Size()) {
       for (int i = 0; i < Size(); ++i)
           list = cString::sprintf("%s%d,", *list, At(i));
       list = list.Truncate(-1);
       }
    return list;
  }
};

class cSatipTuner : public cThread, public cSatipTunerStatistics, public cSatipTunerIf
{
private:
  enum {
    eDummyPid               = 100,
    eDefaultSignalStrength  = 15,
    eDefaultSignalQuality   = 224,
    eSleepTimeoutMs         = 250,   // in milliseconds
    eStatusUpdateTimeoutMs  = 1000,  // in milliseconds
    ePidUpdateIntervalMs    = 250,   // in milliseconds
    eConnectTimeoutMs       = 5000,  // in milliseconds
    eMinKeepAliveIntervalMs = 30000  // in milliseconds
  };
  enum eTunerState { tsIdle, tsRelease, tsSet, tsTuned, tsLocked };
  enum eStateMode { smInternal, smExternal };

  cCondWait sleepM;
  cSatipDeviceIf* deviceM;
  int deviceIdM;
  cSatipRtsp rtspM;
  cSatipRtp rtpM;
  cSatipRtcp rtcpM;
  cString streamAddrM;
  cString streamParamM;
  cSatipServer *currentServerM;
  cSatipServer *nextServerM;
  cMutex mutexM;
  cTimeMs reConnectM;
  cTimeMs keepAliveM;
  cTimeMs statusUpdateM;
  cTimeMs pidUpdateCacheM;
  cString sessionM;
  eTunerState currentStateM;
  cVector<eTunerState> internalStateM;
  cVector<eTunerState> externalStateM;
  int timeoutM;
  bool hasLockM;
  int signalStrengthM;
  int signalQualityM;
  int streamIdM;
  int pmtPidM;
  cSatipPid addPidsM;
  cSatipPid delPidsM;
  cSatipPid pidsM;

  bool Connect(void);
  bool Disconnect(void);
  bool KeepAlive(bool forceP = false);
  bool ReadReceptionStatus(bool forceP = false);
  bool UpdatePids(bool forceP = false);
  void UpdateCurrentState(void);
  bool StateRequested(void);
  bool RequestState(eTunerState stateP, eStateMode modeP);
  const char *StateModeString(eStateMode modeP);
  const char *TunerStateString(eTunerState stateP);

protected:
  virtual void Action(void);

public:
  cSatipTuner(cSatipDeviceIf &deviceP, unsigned int packetLenP);
  virtual ~cSatipTuner();
  bool IsTuned(void) const { return (currentStateM >= tsTuned); }
  bool SetSource(cSatipServer *serverP, const char *parameterP, const int indexP);
  bool SetPid(int pidP, int typeP, bool onP);
  bool Open(void);
  bool Close(void);
  int SignalStrength(void);
  int SignalQuality(void);
  bool HasLock(void);
  cString GetSignalStatus(void);
  cString GetInformation(void);

  // for internal tuner interface
public:
  virtual void ProcessVideoData(u_char *bufferP, int lengthP);
  virtual void ProcessApplicationData(u_char *bufferP, int lengthP);
  virtual void SetStreamId(int streamIdP);
  virtual void SetSessionTimeout(const char *sessionP, int timeoutP);
  virtual int GetId(void);
};

#endif // __SATIP_TUNER_H
