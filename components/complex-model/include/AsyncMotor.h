//
// File: AsyncMotor.h
//
// Code generated for Simulink model 'AsyncMotor'.
//
// Model version                  : 1.349
// Simulink Coder version         : 25.2 (R2025b) 28-Jul-2025
// C/C++ source code generated on : Mon May 11 04:07:43 2026
//
// Target selection: ert.tlc
// Embedded hardware selection: Intel->x86-64 (Linux 64)
// Code generation objectives: Unspecified
// Validation result: Not run
//
#ifndef AsyncMotor_h_
#define AsyncMotor_h_
#include <cmath>
#include "rtwtypes.h"
#include "rtw_continuous.h"
#include "rtw_solver.h"
#include "rt_nonfinite.h"
#include "AsyncMotor_types.h"
#include "rtGetInf.h"
#include "model_reference_types.h"
#include <cstring>

// Block signals for model 'AsyncMotor'
struct AsyncMotor_B_c {
  real_T RateLimiter;                  // '<S7>/Rate Limiter'
  real_T UnitDelay;                    // '<S3>/Unit Delay'
  real_T OutportBufferForOut1;         // '<S2>/Zero'
  real_T Divide;                       // '<S1>/Divide'
};

// Block states (default storage) for model 'AsyncMotor'
struct AsyncMotor_DW_f {
  real_T UnitDelay_DSTATE;             // '<S3>/Unit Delay'
  real_T PrevY;                        // '<S7>/Rate Limiter'
  real_T LastMajorTime;                // '<S7>/Rate Limiter'
  real_T IC_FirstOutputTime;           // '<S3>/IC'
  int8_T If_ActiveSubsystem;           // '<Root>/If'
  boolean_T PrevLimited;               // '<S7>/Rate Limiter'
};

// Invariant block signals for model 'AsyncMotor'
struct AsyncMotor_ConstB_h {
  real_T Product;                      // '<Root>/Product'
  real_T Zero;                         // '<S2>/Zero'
};

// Real-time Model Data Structure
struct AsyncMotor_tag_RTM {
  const char_T **errorStatus;
  RTWSolverInfo *solverInfo;
  const rtTimingBridge *timingBridge;

  //
  //  Timing:
  //  The following substructure contains information regarding
  //  the timing information for the model.

  struct {
    time_T stepSize0;
    int_T mdlref_GlobalTID[2];
    SimTimeStep *simTimeStep;
  } Timing;

  time_T getT() const;
  SimTimeStep getSimTimeStep() const;
  boolean_T isSampleHit(int32_T sti) const;
  const char_T** getErrorStatusPointer() const;
  void setErrorStatusPointer(const char_T** aErrorStatusPointer);
  time_T getClockTickH1() const;
  time_T getClockTickH0() const;
  boolean_T isMinorTimeStep() const;
  boolean_T isMajorTimeStep() const;
  time_T getClockTick1() const;
  SimTimeStep* getSimTimeStepPointer() const;
  void setSimTimeStepPointer(SimTimeStep* aSimTimeStepPointer);
  const char_T* getErrorStatus() const;
  void setErrorStatus(const char_T* const aErrorStatus) const;
  time_T getClockTick0() const;
};

struct AsyncMotor_MdlrefDW {
  AsyncMotor_B_c rtb;
  AsyncMotor_DW_f rtdw;
  AsyncMotor_RT_MODEL rtm;
};

// Model reference registration function
extern void AsyncMotor_initialize(const char_T **rt_errorStatus, RTWSolverInfo
  *rt_solverInfo, const rtTimingBridge *timingBridge, int_T mdlref_TID0, int_T
  mdlref_TID1, AsyncMotor_RT_MODEL *const AsyncMotorrtM, AsyncMotor_B_c *localB,
  AsyncMotor_DW_f *localDW);
extern void AsyncMotor_Init(real_T *rty_T_AD, AsyncMotor_B_c *localB,
  AsyncMotor_DW_f *localDW);
extern void AsyncMotor_Update(AsyncMotor_RT_MODEL * const AsyncMotorrtM, real_T *
  rty_T_AD, AsyncMotor_B_c *localB, AsyncMotor_DW_f *localDW);
extern void AsyncMotor(AsyncMotor_RT_MODEL * const AsyncMotorrtM, const real_T
  *rtu_omega_sync, const real_T *rtu_omega_ICE, const real_T *rtu_fan_enabled,
  const real_T *rtu_Tmax, real_T *rty_n_rpm, real_T *rty_moment, real_T
  *rty_f_AD, real_T *rty_T_AD, boolean_T *rty_limits_exceeded, AsyncMotor_B_c
  *localB, AsyncMotor_DW_f *localDW);

//-
//  The generated code includes comments that allow you to trace directly
//  back to the appropriate location in the model.  The basic format
//  is <system>/block_name, where system is the system number (uniquely
//  assigned by Simulink) and block_name is the name of the block.
//
//  Use the MATLAB hilite_system command to trace the generated code back
//  to the model.  For example,
//
//  hilite_system('<S3>')    - opens system 3
//  hilite_system('<S3>/Kp') - opens and selects block Kp which resides in S3
//
//  Here is the system hierarchy for this model
//
//  '<Root>' : 'AsyncMotor'
//  '<S1>'   : 'AsyncMotor/If Action Subsystem'
//  '<S2>'   : 'AsyncMotor/If Action Subsystem1'
//  '<S3>'   : 'AsyncMotor/Subsystem1'
//  '<S4>'   : 'AsyncMotor/rad_to_hz'
//  '<S5>'   : 'AsyncMotor/Subsystem1/P_loss_calculate'
//  '<S6>'   : 'AsyncMotor/Subsystem1/Subsystem1'
//  '<S7>'   : 'AsyncMotor/Subsystem1/update_fan_factor'

#endif                                 // AsyncMotor_h_

//
// File trailer for generated code.
//
// [EOF]
//
