//
// File: AsyncMotor.cpp
//
// Code generated for Simulink model 'AsyncMotor'.
//
// Model version                  : 1.349
// Simulink Coder version         : 25.2 (R2025b) 28-Jul-2025
// C/C++ source code generated on : Mon May 11 16:22:31 2026
//
// Target selection: ert.tlc
// Embedded hardware selection: Intel->x86-64 (Linux 64)
// Code generation objectives: Unspecified
// Validation result: Not run
//
#include "AsyncMotor.h"
#include "rtwtypes.h"
#include "AsyncMotor_private.h"
#include <cmath>
#include "AsyncMotor_types.h"
#include <cstring>

// System initialize for referenced model: 'AsyncMotor'
void AsyncMotor_Init(real_T *rty_T_AD, AsyncMotor_B_c *localB, AsyncMotor_DW_f
                     *localDW)
{
  // Start for If: '<Root>/If'
  localDW->If_ActiveSubsystem = -1;

  // Start for InitialCondition: '<S3>/IC'
  *rty_T_AD = 25.0;
  localDW->IC_FirstOutputTime = (rtMinusInf);

  // InitializeConditions for RateLimiter: '<S7>/Rate Limiter'
  localDW->LastMajorTime = (rtInf);

  // InitializeConditions for UnitDelay: '<S3>/Unit Delay'
  localDW->UnitDelay_DSTATE = 25.0;

  // SystemInitialize for IfAction SubSystem: '<Root>/If Action Subsystem1'
  // SystemInitialize for SignalConversion generated from: '<S2>/Out1'
  localB->OutportBufferForOut1 = AsyncMotorrtConstB.Zero;

  // End of SystemInitialize for SubSystem: '<Root>/If Action Subsystem1'
}

// Outputs for referenced model: 'AsyncMotor'
void AsyncMotor(AsyncMotor_RT_MODEL * const AsyncMotorrtM, const real_T
                *rtu_omega_sync, const real_T *rtu_omega_ICE, const real_T
                *rtu_fan_enabled, const real_T *rtu_Tmax, real_T *rty_n_rpm,
                real_T *rty_moment, real_T *rty_f_AD, real_T *rty_T_AD,
                boolean_T *rty_limits_exceeded, AsyncMotor_B_c *localB,
                AsyncMotor_DW_f *localDW)
{
  real_T deltaT_tmp;
  real_T riseValLimit;
  real_T rtb_Plus;
  real_T rtb_Plus2;
  boolean_T limitedCache;

  // If: '<Root>/If'
  if (*rtu_omega_sync != 0.0) {
    localDW->If_ActiveSubsystem = 0;

    // Outputs for IfAction SubSystem: '<Root>/If Action Subsystem' incorporates:
    //   ActionPort: '<S1>/Action Port'

    // Product: '<S1>/Divide' incorporates:
    //   Sum: '<S1>/Minus'

    localB->Divide = (*rtu_omega_sync - *rtu_omega_ICE) / *rtu_omega_sync;

    // End of Outputs for SubSystem: '<Root>/If Action Subsystem'
  } else {
    localDW->If_ActiveSubsystem = 1;

    // Outputs for IfAction SubSystem: '<Root>/If Action Subsystem1' incorporates:
    //   ActionPort: '<S2>/Action Port'

    if (AsyncMotorrtM->isMajorTimeStep()) {
      // SignalConversion generated from: '<S2>/Out1'
      localB->OutportBufferForOut1 = AsyncMotorrtConstB.Zero;
    }

    // End of Outputs for SubSystem: '<Root>/If Action Subsystem1'
  }

  // End of If: '<Root>/If'

  // Product: '<Root>/Divide1' incorporates:
  //   Constant: '<Root>/Constant8'
  //   Sum: '<Root>/Sum'

  rtb_Plus2 = (localB->Divide + localB->OutportBufferForOut1) / 0.05;

  // Product: '<Root>/Divide2' incorporates:
  //   Constant: '<Root>/Constant1'
  //   Constant: '<Root>/Constant2'
  //   Math: '<Root>/Square'
  //   Product: '<Root>/Product1'
  //   Sum: '<Root>/Plus'

  *rty_moment = 1.0 / (rtb_Plus2 * rtb_Plus2 + 1.0) * (2.0 * rtb_Plus2 *
    AsyncMotorrtConstB.Product);

  // Switch: '<S7>/Switch2'
  if (*rtu_fan_enabled > 0.0) {
    // Switch: '<S7>/Switch2' incorporates:
    //   Constant: '<S7>/Zero2'

    rtb_Plus2 = 1.0;
  } else {
    // Switch: '<S7>/Switch2' incorporates:
    //   Constant: '<S7>/Zero3'

    rtb_Plus2 = 0.4;
  }

  // End of Switch: '<S7>/Switch2'

  // RateLimiter: '<S7>/Rate Limiter'
  if (localDW->LastMajorTime == (rtInf)) {
    // RateLimiter: '<S7>/Rate Limiter'
    localB->RateLimiter = rtb_Plus2;
  } else {
    deltaT_tmp = (*(AsyncMotorrtM->timingBridge->taskTime
                    [AsyncMotorrtM->Timing.mdlref_GlobalTID[0]]));
    rtb_Plus = deltaT_tmp - localDW->LastMajorTime;
    if (localDW->LastMajorTime == deltaT_tmp) {
      if (localDW->PrevLimited) {
        // RateLimiter: '<S7>/Rate Limiter'
        localB->RateLimiter = localDW->PrevY;
      } else {
        // RateLimiter: '<S7>/Rate Limiter'
        localB->RateLimiter = rtb_Plus2;
      }
    } else {
      riseValLimit = rtb_Plus * 0.12;
      deltaT_tmp = rtb_Plus2 - localDW->PrevY;
      if (deltaT_tmp > riseValLimit) {
        // RateLimiter: '<S7>/Rate Limiter'
        localB->RateLimiter = localDW->PrevY + riseValLimit;
        limitedCache = true;
      } else {
        rtb_Plus *= -0.12;
        if (deltaT_tmp < rtb_Plus) {
          // RateLimiter: '<S7>/Rate Limiter'
          localB->RateLimiter = localDW->PrevY + rtb_Plus;
          limitedCache = true;
        } else {
          // RateLimiter: '<S7>/Rate Limiter'
          localB->RateLimiter = rtb_Plus2;
          limitedCache = false;
        }
      }

      if (rtsiIsModeUpdateTimeStep(AsyncMotorrtM->solverInfo)) {
        localDW->PrevLimited = limitedCache;
      }
    }
  }

  // End of RateLimiter: '<S7>/Rate Limiter'

  // Sum: '<S7>/Plus2' incorporates:
  //   Abs: '<S7>/Abs'
  //   Constant: '<S7>/Constant7'
  //   Constant: '<S7>/M_AD_nom'
  //   Product: '<S7>/Divide'
  //   Product: '<S7>/Product'

  rtb_Plus2 = std::abs(*rty_moment) / 3500.0 * localB->RateLimiter + 1.0;

  // Sum: '<S3>/Plus' incorporates:
  //   Abs: '<S5>/Abs'
  //   Constant: '<S3>/Constant'
  //   Constant: '<S3>/Constant1'
  //   Constant: '<S5>/Constant'
  //   Constant: '<S5>/Constant1'
  //   Product: '<S3>/Divide'
  //   Product: '<S3>/Product'
  //   Product: '<S5>/Product2'
  //   Sum: '<S5>/Plus1'

  rtb_Plus = (*rtu_omega_ICE * std::abs(*rty_moment) * 0.025 + 800.0) / (150.0 *
    rtb_Plus2) + 25.0;
  if (AsyncMotorrtM->isMajorTimeStep()) {
    // UnitDelay: '<S3>/Unit Delay'
    localB->UnitDelay = localDW->UnitDelay_DSTATE;
  }

  // Sum: '<S6>/Plus' incorporates:
  //   Constant: '<S3>/Constant2'
  //   Constant: '<S6>/Constant'
  //   Math: '<S6>/Exp'
  //   Product: '<S3>/Divide1'
  //   Product: '<S6>/Divide'
  //   Product: '<S6>/Product'
  //   Sum: '<S6>/Minus'
  //
  //  About '<S6>/Exp':
  //   Operator: exp

  rtb_Plus += std::exp(-0.001 / (900.0 / rtb_Plus2)) * (localB->UnitDelay -
    rtb_Plus);

  // InitialCondition: '<S3>/IC'
  rtb_Plus2 = (*(AsyncMotorrtM->timingBridge->taskTime
                 [AsyncMotorrtM->Timing.mdlref_GlobalTID[0]]));
  if ((localDW->IC_FirstOutputTime == (rtMinusInf)) ||
      (localDW->IC_FirstOutputTime == rtb_Plus2)) {
    localDW->IC_FirstOutputTime = rtb_Plus2;
    *rty_T_AD = 25.0;
  } else {
    *rty_T_AD = rtb_Plus;
  }

  // End of InitialCondition: '<S3>/IC'

  // RelationalOperator: '<S3>/GreaterThan'
  *rty_limits_exceeded = (*rty_T_AD > *rtu_Tmax);

  // Product: '<Root>/Product2' incorporates:
  //   Constant: '<Root>/Constant4'

  *rty_n_rpm = *rtu_omega_ICE * 9.5492965855137211;

  // Product: '<S4>/Divide' incorporates:
  //   Constant: '<S4>/Constant'
  //   Constant: '<S4>/Pi'
  //   Product: '<S4>/Product'

  *rty_f_AD = *rtu_omega_ICE * 2.0 / 6.2831853071795862;
}

// Update for referenced model: 'AsyncMotor'
void AsyncMotor_Update(AsyncMotor_RT_MODEL * const AsyncMotorrtM, real_T
  *rty_T_AD, AsyncMotor_B_c *localB, AsyncMotor_DW_f *localDW)
{
  // Update for RateLimiter: '<S7>/Rate Limiter'
  localDW->PrevY = localB->RateLimiter;
  localDW->LastMajorTime = (*(AsyncMotorrtM->timingBridge->
    taskTime[AsyncMotorrtM->Timing.mdlref_GlobalTID[0]]));
  if (AsyncMotorrtM->isMajorTimeStep()) {
    // Update for UnitDelay: '<S3>/Unit Delay'
    localDW->UnitDelay_DSTATE = *rty_T_AD;
  }
}

// Model initialize function
void AsyncMotor_initialize(const char_T **rt_errorStatus, RTWSolverInfo
  *rt_solverInfo, const rtTimingBridge *timingBridge, int_T mdlref_TID0, int_T
  mdlref_TID1, AsyncMotor_RT_MODEL *const AsyncMotorrtM, AsyncMotor_B_c *localB,
  AsyncMotor_DW_f *localDW)
{
  // Registration code

  // setup the global timing engine
  AsyncMotorrtM->Timing.mdlref_GlobalTID[0] = mdlref_TID0;
  AsyncMotorrtM->Timing.mdlref_GlobalTID[1] = mdlref_TID1;
  AsyncMotorrtM->timingBridge = (timingBridge);

  // initialize error status
  AsyncMotorrtM->setErrorStatusPointer(rt_errorStatus);

  // initialize RTWSolverInfo
  AsyncMotorrtM->solverInfo = (rt_solverInfo);

  // Set the Timing fields to the appropriate data in the RTWSolverInfo
  AsyncMotorrtM->setSimTimeStepPointer(rtsiGetSimTimeStepPtr
    (AsyncMotorrtM->solverInfo));
  AsyncMotorrtM->Timing.stepSize0 = (rtsiGetStepSize(AsyncMotorrtM->solverInfo));

  // block I/O
  (void) std::memset((static_cast<void *>(localB)), 0,
                     sizeof(AsyncMotor_B_c));

  // states (dwork)
  (void) std::memset(static_cast<void *>(localDW), 0,
                     sizeof(AsyncMotor_DW_f));
}

time_T AsyncMotor_RT_MODEL::getT() const
{
  return (*(timingBridge->taskTime[0]));
}

SimTimeStep AsyncMotor_RT_MODEL::getSimTimeStep() const
{
  return (*(Timing.simTimeStep));
}

boolean_T AsyncMotor_RT_MODEL::isSampleHit(int32_T sti) const
{
  return (timingBridge->taskCounter[Timing.mdlref_GlobalTID[sti]] == 0);
}

const char_T** AsyncMotor_RT_MODEL::getErrorStatusPointer() const
{
  return errorStatus;
}

void AsyncMotor_RT_MODEL::setErrorStatusPointer(const char_T
  ** aErrorStatusPointer)
{
  (errorStatus = aErrorStatusPointer);
}

time_T AsyncMotor_RT_MODEL::getClockTickH1() const
{
  return ( *(timingBridge->clockTickH[Timing.mdlref_GlobalTID[1]]) );
}

time_T AsyncMotor_RT_MODEL::getClockTickH0() const
{
  return ( *(timingBridge->clockTickH[Timing.mdlref_GlobalTID[0]]) );
}

boolean_T AsyncMotor_RT_MODEL::isMinorTimeStep() const
{
  return ((getSimTimeStep()) == MINOR_TIME_STEP);
}

boolean_T AsyncMotor_RT_MODEL::isMajorTimeStep() const
{
  return ((getSimTimeStep()) == MAJOR_TIME_STEP);
}

time_T AsyncMotor_RT_MODEL::getClockTick1() const
{
  return ( *((timingBridge->clockTick[Timing.mdlref_GlobalTID[1]])) );
}

SimTimeStep* AsyncMotor_RT_MODEL::getSimTimeStepPointer() const
{
  return Timing.simTimeStep;
}

void AsyncMotor_RT_MODEL::setSimTimeStepPointer(SimTimeStep* aSimTimeStepPointer)
{
  (Timing.simTimeStep = aSimTimeStepPointer);
}

const char_T* AsyncMotor_RT_MODEL::getErrorStatus() const
{
  return (*(errorStatus));
}

void AsyncMotor_RT_MODEL::setErrorStatus(const char_T* const aErrorStatus) const
{
  (*(errorStatus) = aErrorStatus);
}

time_T AsyncMotor_RT_MODEL::getClockTick0() const
{
  return ( *((timingBridge->clockTick[Timing.mdlref_GlobalTID[0]])) );
}

//
// File trailer for generated code.
//
// [EOF]
//
