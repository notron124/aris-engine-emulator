//
// File: ICE.cpp
//
// Code generated for Simulink model 'ICE'.
//
// Model version                  : 1.460
// Simulink Coder version         : 25.2 (R2025b) 28-Jul-2025
// C/C++ source code generated on : Mon May 11 16:22:59 2026
//
// Target selection: ert.tlc
// Embedded hardware selection: Intel->x86-64 (Linux 64)
// Code generation objectives: Unspecified
// Validation result: Not run
//
#include "ICE.h"
#include "rtwtypes.h"
#include <cmath>
#include "ICE_private.h"
#include "ICE_types.h"
#include <cstring>

// System initialize for referenced model: 'ICE'
void ICE_Init(real_T *rty_temperature, ICE_DW_f *localDW)
{
  // Start for InitialCondition: '<S1>/IC'
  *rty_temperature = 25.0;
  localDW->IC_FirstOutputTime = (rtMinusInf);

  // Start for If: '<S2>/If'
  localDW->If_ActiveSubsystem = -1;

  // InitializeConditions for UnitDelay: '<S1>/Unit Delay'
  localDW->UnitDelay_DSTATE_j = 25.0;

  // InitializeConditions for RateLimiter: '<S1>/Rate Limiter'
  localDW->LastMajorTime = (rtInf);
}

// Outputs for referenced model: 'ICE'
void ICE(ICE_RT_MODEL * const ICErtM, const real_T *rtu_target_n_rpm, const
         real_T *rtu_n_rpm_max_run, const real_T *rtu_M_AD_torque, const real_T *
         rtu_P_oil_max, const real_T *rtu_P_oil_min, const real_T *rtu_set_n_rpm,
         const real_T *rtu_fan_enabled, real_T *rty_temperature, real_T
         *rty_oil_preasure, real_T *rty_n_rpm_ICE, ICE_B_c *localB, ICE_DW_f
         *localDW)
{
  real_T deltaT;
  real_T deltaT_tmp;
  real_T riseValLimit;
  real_T rtb_Switch;
  real_T thresh1;
  real_T thresh2;
  boolean_T limitedCache;
  boolean_T tmp;
  tmp = ICErtM->isMajorTimeStep();
  if (tmp) {
    // UnitDelay: '<Root>/Unit Delay' incorporates:
    //   DataStoreWrite: '<Root>/Data Store Write1'

    localDW->n_rpm = localDW->UnitDelay_DSTATE;

    // Switch: '<Root>/Switch' incorporates:
    //   DataStoreRead: '<Root>/Data Store Read4'

    if (*rtu_set_n_rpm != 0.0) {
      deltaT_tmp = *rtu_set_n_rpm;
    } else {
      deltaT_tmp = localDW->n_rpm;
    }

    // Product: '<Root>/Product2' incorporates:
    //   Constant: '<Root>/Constant2'
    //   DataStoreWrite: '<Root>/Data Store Write'
    //   Switch: '<Root>/Switch'

    localDW->n_rpm = deltaT_tmp * 0.10471975511965977;

    // UnitDelay: '<S1>/Unit Delay'
    localB->UnitDelay = localDW->UnitDelay_DSTATE_j;
  }

  // Product: '<Root>/Product' incorporates:
  //   Constant: '<Root>/Constant'

  rtb_Switch = *rtu_n_rpm_max_run * 0.10471975511965977;

  // MATLAB Function: '<S7>/MATLAB Function' incorporates:
  //   Constant: '<S7>/Constant'
  //   DataStoreRead: '<Root>/Data Store Read'

  thresh1 = rtb_Switch * 0.45;
  thresh2 = rtb_Switch * 0.8;
  if (localDW->n_rpm < thresh1) {
    thresh1 = (localDW->n_rpm / thresh1 * 6.0 + 1.0) * 500.0;
  } else if (localDW->n_rpm < thresh2) {
    thresh1 = 3500.0;
  } else if (localDW->n_rpm < rtb_Switch) {
    thresh1 = (1.0 - (localDW->n_rpm - thresh2) / (rtb_Switch - thresh2) * 0.25)
      * 3500.0;
  } else {
    thresh1 = 2625.0;
  }

  // End of MATLAB Function: '<S7>/MATLAB Function'

  // MinMax: '<S3>/Max' incorporates:
  //   Constant: '<S3>/Constant1'
  //   Constant: '<S3>/Zero'
  //   DataStoreRead: '<Root>/Data Store Read'
  //   Gain: '<S3>/Gain'
  //   MinMax: '<S3>/Min'
  //   Product: '<S3>/Product'
  //   Sum: '<S3>/Sum'

  thresh1 = std::fmax(std::fmin((*rtu_target_n_rpm * 0.10471975511965977 -
    localDW->n_rpm) * 25.0, thresh1), 0.0);

  // Switch: '<S1>/Switch2'
  if (*rtu_fan_enabled > 0.0) {
    // Switch: '<S1>/Switch2' incorporates:
    //   Constant: '<S1>/Zero2'

    thresh2 = 1.0;
  } else {
    // Switch: '<S1>/Switch2' incorporates:
    //   Constant: '<S1>/Zero3'

    thresh2 = 0.4;
  }

  // End of Switch: '<S1>/Switch2'

  // RateLimiter: '<S1>/Rate Limiter'
  if (localDW->LastMajorTime == (rtInf)) {
    // RateLimiter: '<S1>/Rate Limiter'
    localB->RateLimiter = thresh2;
  } else {
    deltaT_tmp = (*(ICErtM->timingBridge->taskTime
                    [ICErtM->Timing.mdlref_GlobalTID[0]]));
    deltaT = deltaT_tmp - localDW->LastMajorTime;
    if (localDW->LastMajorTime == deltaT_tmp) {
      if (localDW->PrevLimited) {
        // RateLimiter: '<S1>/Rate Limiter'
        localB->RateLimiter = localDW->PrevY;
      } else {
        // RateLimiter: '<S1>/Rate Limiter'
        localB->RateLimiter = thresh2;
      }
    } else {
      riseValLimit = deltaT * 0.12;
      deltaT_tmp = thresh2 - localDW->PrevY;
      if (deltaT_tmp > riseValLimit) {
        // RateLimiter: '<S1>/Rate Limiter'
        localB->RateLimiter = localDW->PrevY + riseValLimit;
        limitedCache = true;
      } else {
        deltaT *= -0.12;
        if (deltaT_tmp < deltaT) {
          // RateLimiter: '<S1>/Rate Limiter'
          localB->RateLimiter = localDW->PrevY + deltaT;
          limitedCache = true;
        } else {
          // RateLimiter: '<S1>/Rate Limiter'
          localB->RateLimiter = thresh2;
          limitedCache = false;
        }
      }

      if (rtsiIsModeUpdateTimeStep(ICErtM->solverInfo)) {
        localDW->PrevLimited = limitedCache;
      }
    }
  }

  // End of RateLimiter: '<S1>/Rate Limiter'

  // Sum: '<S1>/Plus3' incorporates:
  //   Constant: '<S1>/Constant5'
  //   DataStoreRead: '<Root>/Data Store Read3'
  //   Product: '<S1>/Product'
  //   Product: '<S1>/Product2'
  //   Product: '<S1>/Product3'

  thresh2 = thresh1 * localDW->n_rpm * ICErtConstB.Divide1 * localB->RateLimiter
    + 25.0;

  // Sum: '<S5>/Plus' incorporates:
  //   Product: '<S5>/Product'
  //   Sum: '<S5>/Minus'

  thresh2 += (localB->UnitDelay - thresh2) * ICErtConstB.Exp;

  // InitialCondition: '<S1>/IC'
  deltaT_tmp = (*(ICErtM->timingBridge->taskTime[ICErtM->
                  Timing.mdlref_GlobalTID[0]]));
  if ((localDW->IC_FirstOutputTime == (rtMinusInf)) ||
      (localDW->IC_FirstOutputTime == deltaT_tmp)) {
    localDW->IC_FirstOutputTime = deltaT_tmp;
    *rty_temperature = 25.0;
  } else {
    *rty_temperature = thresh2;
  }

  // End of InitialCondition: '<S1>/IC'

  // If: '<S2>/If'
  if (rtb_Switch > 0.0) {
    localDW->If_ActiveSubsystem = 0;

    // Outputs for IfAction SubSystem: '<S2>/If Action Subsystem' incorporates:
    //   ActionPort: '<S6>/Action Port'

    // Product: '<S6>/Product' incorporates:
    //   Constant: '<S2>/Constant3'
    //   DataStoreRead: '<Root>/Data Store Read2'
    //   Product: '<S2>/Product'
    //   Product: '<S6>/Divide1'
    //   Sum: '<S6>/Minus1'

    localB->Product = localDW->n_rpm * 0.10471975511965977 / rtb_Switch *
      (*rtu_P_oil_max - *rtu_P_oil_min);

    // End of Outputs for SubSystem: '<S2>/If Action Subsystem'
  } else {
    localDW->If_ActiveSubsystem = 1;
  }

  // End of If: '<S2>/If'

  // Product: '<S2>/Product2' incorporates:
  //   Constant: '<S2>/Constant'
  //   Constant: '<S2>/Constant1'
  //   Constant: '<S2>/Constant2'
  //   Constant: '<S2>/Constant5'
  //   MinMax: '<S2>/Max'
  //   Product: '<S2>/Divide'
  //   Product: '<S2>/Divide1'
  //   Sum: '<S2>/Minus'
  //   Sum: '<S2>/Plus'
  //   Sum: '<S2>/Plus1'

  *rty_oil_preasure = std::fmax(0.1, 1.0 / ((*rty_temperature - 80.0) / 50.0 +
    1.0)) * (localB->Product + *rtu_P_oil_min);
  if (tmp) {
    // UnitDelay: '<S4>/Unit Delay'
    localB->UnitDelay_o = localDW->UnitDelay_DSTATE_h;

    // Sum: '<S4>/Plus' incorporates:
    //   Constant: '<S4>/Constant'
    //   Constant: '<S4>/Constant1'
    //   Product: '<S4>/Product'

    localB->M_internal_fric = 0.1 * localB->UnitDelay_o + 50.0;

    // DataStoreRead: '<Root>/Data Store Read1'
    *rty_n_rpm_ICE = localDW->n_rpm;
  }

  // MinMax: '<S4>/Max' incorporates:
  //   Constant: '<S4>/Constant2'
  //   Constant: '<S4>/Zero'
  //   Gain: '<S4>/Gain'
  //   Product: '<S4>/Divide'
  //   Sum: '<S4>/Plus1'
  //   Sum: '<S4>/Plus2'

  localB->omega_ice = std::fmax(((thresh1 - localB->M_internal_fric) +
    *rtu_M_AD_torque) / 3.5 * 0.001 + localB->UnitDelay_o, 0.0);
}

// Update for referenced model: 'ICE'
void ICE_Update(ICE_RT_MODEL * const ICErtM, real_T *rty_temperature, ICE_B_c
                *localB, ICE_DW_f *localDW)
{
  if (ICErtM->isMajorTimeStep()) {
    // Update for UnitDelay: '<Root>/Unit Delay'
    localDW->UnitDelay_DSTATE = localB->omega_ice;

    // Update for UnitDelay: '<S1>/Unit Delay'
    localDW->UnitDelay_DSTATE_j = *rty_temperature;

    // Update for UnitDelay: '<S4>/Unit Delay'
    localDW->UnitDelay_DSTATE_h = localB->omega_ice;
  }

  // Update for RateLimiter: '<S1>/Rate Limiter'
  localDW->PrevY = localB->RateLimiter;
  localDW->LastMajorTime = (*(ICErtM->timingBridge->taskTime
    [ICErtM->Timing.mdlref_GlobalTID[0]]));
}

// Model initialize function
void ICE_initialize(const char_T **rt_errorStatus, RTWSolverInfo *rt_solverInfo,
                    const rtTimingBridge *timingBridge, int_T mdlref_TID0, int_T
                    mdlref_TID1, ICE_RT_MODEL *const ICErtM, ICE_B_c *localB,
                    ICE_DW_f *localDW)
{
  // Registration code

  // setup the global timing engine
  ICErtM->Timing.mdlref_GlobalTID[0] = mdlref_TID0;
  ICErtM->Timing.mdlref_GlobalTID[1] = mdlref_TID1;
  ICErtM->timingBridge = (timingBridge);

  // initialize error status
  ICErtM->setErrorStatusPointer(rt_errorStatus);

  // initialize RTWSolverInfo
  ICErtM->solverInfo = (rt_solverInfo);

  // Set the Timing fields to the appropriate data in the RTWSolverInfo
  ICErtM->setSimTimeStepPointer(rtsiGetSimTimeStepPtr(ICErtM->solverInfo));
  ICErtM->Timing.stepSize0 = (rtsiGetStepSize(ICErtM->solverInfo));

  // block I/O
  (void) std::memset((static_cast<void *>(localB)), 0,
                     sizeof(ICE_B_c));

  // states (dwork)
  (void) std::memset(static_cast<void *>(localDW), 0,
                     sizeof(ICE_DW_f));
}

time_T ICE_RT_MODEL::getT() const
{
  return (*(timingBridge->taskTime[0]));
}

SimTimeStep ICE_RT_MODEL::getSimTimeStep() const
{
  return (*(Timing.simTimeStep));
}

boolean_T ICE_RT_MODEL::isSampleHit(int32_T sti) const
{
  return (timingBridge->taskCounter[Timing.mdlref_GlobalTID[sti]] == 0);
}

const char_T** ICE_RT_MODEL::getErrorStatusPointer() const
{
  return errorStatus;
}

void ICE_RT_MODEL::setErrorStatusPointer(const char_T** aErrorStatusPointer)
{
  (errorStatus = aErrorStatusPointer);
}

time_T ICE_RT_MODEL::getClockTickH1() const
{
  return ( *(timingBridge->clockTickH[Timing.mdlref_GlobalTID[1]]) );
}

time_T ICE_RT_MODEL::getClockTickH0() const
{
  return ( *(timingBridge->clockTickH[Timing.mdlref_GlobalTID[0]]) );
}

boolean_T ICE_RT_MODEL::isMinorTimeStep() const
{
  return ((getSimTimeStep()) == MINOR_TIME_STEP);
}

boolean_T ICE_RT_MODEL::isMajorTimeStep() const
{
  return ((getSimTimeStep()) == MAJOR_TIME_STEP);
}

time_T ICE_RT_MODEL::getClockTick1() const
{
  return ( *((timingBridge->clockTick[Timing.mdlref_GlobalTID[1]])) );
}

SimTimeStep* ICE_RT_MODEL::getSimTimeStepPointer() const
{
  return Timing.simTimeStep;
}

void ICE_RT_MODEL::setSimTimeStepPointer(SimTimeStep* aSimTimeStepPointer)
{
  (Timing.simTimeStep = aSimTimeStepPointer);
}

const char_T* ICE_RT_MODEL::getErrorStatus() const
{
  return (*(errorStatus));
}

void ICE_RT_MODEL::setErrorStatus(const char_T* const aErrorStatus) const
{
  (*(errorStatus) = aErrorStatus);
}

time_T ICE_RT_MODEL::getClockTick0() const
{
  return ( *((timingBridge->clockTick[Timing.mdlref_GlobalTID[0]])) );
}

//
// File trailer for generated code.
//
// [EOF]
//
