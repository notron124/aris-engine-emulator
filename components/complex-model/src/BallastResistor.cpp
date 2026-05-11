//
// File: BallastResistor.cpp
//
// Code generated for Simulink model 'BallastResistor'.
//
// Model version                  : 1.239
// Simulink Coder version         : 25.2 (R2025b) 28-Jul-2025
// C/C++ source code generated on : Mon May 11 16:22:45 2026
//
// Target selection: ert.tlc
// Embedded hardware selection: Intel->x86-64 (Linux 64)
// Code generation objectives: Unspecified
// Validation result: Not run
//
#include "BallastResistor.h"
#include "rtwtypes.h"
#include "BallastResistor_private.h"
#include "BallastResistor_types.h"
#include <cstring>

// System initialize for referenced model: 'BallastResistor'
void BallastResistor_Init(BallastResistor_DW_f *localDW)
{
  // InitializeConditions for UnitDelay: '<S1>/Unit Delay'
  localDW->UnitDelay_DSTATE = 25.0;

  // InitializeConditions for RateLimiter: '<S2>/Rate Limiter'
  localDW->LastMajorTime = (rtInf);
}

// Outputs for referenced model: 'BallastResistor'
void BallastResistor(BallastResistor_RT_MODEL * const BallastResistorrtM, const
                     real_T *rtu_Tmax, const real_T *rtu_P_ballast, const real_T
                     *rtu_fan_enabled, real_T *rty_Temperature, boolean_T
                     *rty_limits_exceeded, BallastResistor_B_c *localB,
                     BallastResistor_DW_f *localDW)
{
  real_T Switch1;
  real_T deltaT;
  real_T deltaT_tmp;
  real_T riseValLimit;
  boolean_T limitedCache;
  if (BallastResistorrtM->isMajorTimeStep()) {
    // UnitDelay: '<S1>/Unit Delay'
    localB->UnitDelay = localDW->UnitDelay_DSTATE;
  }

  // Switch: '<S2>/Switch1'
  if (*rtu_fan_enabled > 0.0) {
    // Switch: '<S2>/Switch1' incorporates:
    //   Constant: '<S2>/Zero1'

    Switch1 = 1.0;
  } else {
    // Switch: '<S2>/Switch1' incorporates:
    //   Constant: '<S2>/Zero3'

    Switch1 = 0.4;
  }

  // End of Switch: '<S2>/Switch1'

  // RateLimiter: '<S2>/Rate Limiter'
  if (localDW->LastMajorTime == (rtInf)) {
    // RateLimiter: '<S2>/Rate Limiter'
    localB->RateLimiter = Switch1;
  } else {
    deltaT_tmp = (*(BallastResistorrtM->timingBridge->
                    taskTime[BallastResistorrtM->Timing.mdlref_GlobalTID[0]]));
    deltaT = deltaT_tmp - localDW->LastMajorTime;
    if (localDW->LastMajorTime == deltaT_tmp) {
      if (localDW->PrevLimited) {
        // RateLimiter: '<S2>/Rate Limiter'
        localB->RateLimiter = localDW->PrevY;
      } else {
        // RateLimiter: '<S2>/Rate Limiter'
        localB->RateLimiter = Switch1;
      }
    } else {
      riseValLimit = deltaT * 0.12;
      deltaT_tmp = Switch1 - localDW->PrevY;
      if (deltaT_tmp > riseValLimit) {
        // RateLimiter: '<S2>/Rate Limiter'
        localB->RateLimiter = localDW->PrevY + riseValLimit;
        limitedCache = true;
      } else {
        deltaT *= -0.12;
        if (deltaT_tmp < deltaT) {
          // RateLimiter: '<S2>/Rate Limiter'
          localB->RateLimiter = localDW->PrevY + deltaT;
          limitedCache = true;
        } else {
          // RateLimiter: '<S2>/Rate Limiter'
          localB->RateLimiter = Switch1;
          limitedCache = false;
        }
      }

      if (rtsiIsModeUpdateTimeStep(BallastResistorrtM->solverInfo)) {
        localDW->PrevLimited = limitedCache;
      }
    }
  }

  // End of RateLimiter: '<S2>/Rate Limiter'

  // Sum: '<S2>/Plus2' incorporates:
  //   Constant: '<S2>/Constant'
  //   Constant: '<S2>/Constant1'
  //   Product: '<S2>/Divide'
  //   Product: '<S2>/Product'

  Switch1 = *rtu_P_ballast / 400000.0 * localB->RateLimiter + 1.0;

  // Switch: '<S1>/Switch' incorporates:
  //   Constant: '<S1>/Constant2'
  //   Product: '<S1>/Divide'
  //   Product: '<S1>/Product'

  if (Switch1 > 0.0) {
    Switch1 = *rtu_P_ballast / (Switch1 * 100.0);
  } else {
    Switch1 = 0.0;
  }

  // End of Switch: '<S1>/Switch'

  // Sum: '<S3>/Plus' incorporates:
  //   Constant: '<S1>/Constant1'
  //   Product: '<S3>/Product'
  //   Sum: '<S1>/Plus'
  //   Sum: '<S3>/Minus'

  *rty_Temperature = (localB->UnitDelay - (Switch1 + 25.0)) *
    BallastResistorrtConstB.Exp + (Switch1 + 25.0);

  // RelationalOperator: '<Root>/GreaterThan'
  *rty_limits_exceeded = (*rty_Temperature > *rtu_Tmax);
}

// Update for referenced model: 'BallastResistor'
void BallastResistor_Update(BallastResistor_RT_MODEL * const BallastResistorrtM,
  real_T *rty_Temperature, BallastResistor_B_c *localB, BallastResistor_DW_f
  *localDW)
{
  if (BallastResistorrtM->isMajorTimeStep()) {
    // Update for UnitDelay: '<S1>/Unit Delay'
    localDW->UnitDelay_DSTATE = *rty_Temperature;
  }

  // Update for RateLimiter: '<S2>/Rate Limiter'
  localDW->PrevY = localB->RateLimiter;
  localDW->LastMajorTime = (*(BallastResistorrtM->timingBridge->
    taskTime[BallastResistorrtM->Timing.mdlref_GlobalTID[0]]));
}

// Model initialize function
void BallastResistor_initialize(const char_T **rt_errorStatus, RTWSolverInfo
  *rt_solverInfo, const rtTimingBridge *timingBridge, int_T mdlref_TID0, int_T
  mdlref_TID1, BallastResistor_RT_MODEL *const BallastResistorrtM,
  BallastResistor_B_c *localB, BallastResistor_DW_f *localDW)
{
  // Registration code

  // setup the global timing engine
  BallastResistorrtM->Timing.mdlref_GlobalTID[0] = mdlref_TID0;
  BallastResistorrtM->Timing.mdlref_GlobalTID[1] = mdlref_TID1;
  BallastResistorrtM->timingBridge = (timingBridge);

  // initialize error status
  BallastResistorrtM->setErrorStatusPointer(rt_errorStatus);

  // initialize RTWSolverInfo
  BallastResistorrtM->solverInfo = (rt_solverInfo);

  // Set the Timing fields to the appropriate data in the RTWSolverInfo
  BallastResistorrtM->setSimTimeStepPointer(rtsiGetSimTimeStepPtr
    (BallastResistorrtM->solverInfo));
  BallastResistorrtM->Timing.stepSize0 = (rtsiGetStepSize
    (BallastResistorrtM->solverInfo));

  // block I/O
  (void) std::memset((static_cast<void *>(localB)), 0,
                     sizeof(BallastResistor_B_c));

  // states (dwork)
  (void) std::memset(static_cast<void *>(localDW), 0,
                     sizeof(BallastResistor_DW_f));
}

time_T BallastResistor_RT_MODEL::getT() const
{
  return (*(timingBridge->taskTime[0]));
}

SimTimeStep BallastResistor_RT_MODEL::getSimTimeStep() const
{
  return (*(Timing.simTimeStep));
}

boolean_T BallastResistor_RT_MODEL::isSampleHit(int32_T sti) const
{
  return (timingBridge->taskCounter[Timing.mdlref_GlobalTID[sti]] == 0);
}

const char_T** BallastResistor_RT_MODEL::getErrorStatusPointer() const
{
  return errorStatus;
}

void BallastResistor_RT_MODEL::setErrorStatusPointer(const char_T
  ** aErrorStatusPointer)
{
  (errorStatus = aErrorStatusPointer);
}

time_T BallastResistor_RT_MODEL::getClockTickH1() const
{
  return ( *(timingBridge->clockTickH[Timing.mdlref_GlobalTID[1]]) );
}

time_T BallastResistor_RT_MODEL::getClockTickH0() const
{
  return ( *(timingBridge->clockTickH[Timing.mdlref_GlobalTID[0]]) );
}

boolean_T BallastResistor_RT_MODEL::isMinorTimeStep() const
{
  return ((getSimTimeStep()) == MINOR_TIME_STEP);
}

boolean_T BallastResistor_RT_MODEL::isMajorTimeStep() const
{
  return ((getSimTimeStep()) == MAJOR_TIME_STEP);
}

time_T BallastResistor_RT_MODEL::getClockTick1() const
{
  return ( *((timingBridge->clockTick[Timing.mdlref_GlobalTID[1]])) );
}

SimTimeStep* BallastResistor_RT_MODEL::getSimTimeStepPointer() const
{
  return Timing.simTimeStep;
}

void BallastResistor_RT_MODEL::setSimTimeStepPointer(SimTimeStep*
  aSimTimeStepPointer)
{
  (Timing.simTimeStep = aSimTimeStepPointer);
}

const char_T* BallastResistor_RT_MODEL::getErrorStatus() const
{
  return (*(errorStatus));
}

void BallastResistor_RT_MODEL::setErrorStatus(const char_T* const aErrorStatus)
  const
{
  (*(errorStatus) = aErrorStatus);
}

time_T BallastResistor_RT_MODEL::getClockTick0() const
{
  return ( *((timingBridge->clockTick[Timing.mdlref_GlobalTID[0]])) );
}

//
// File trailer for generated code.
//
// [EOF]
//
