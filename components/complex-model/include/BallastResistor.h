//
// File: BallastResistor.h
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
#ifndef BallastResistor_h_
#define BallastResistor_h_
#include <cmath>
#include "rtwtypes.h"
#include "rtw_continuous.h"
#include "rtw_solver.h"
#include "rt_nonfinite.h"
#include "BallastResistor_types.h"
#include "rtGetInf.h"
#include "model_reference_types.h"
#include <cstring>

// Block signals for model 'BallastResistor'
struct BallastResistor_B_c {
  real_T UnitDelay;                    // '<S1>/Unit Delay'
  real_T RateLimiter;                  // '<S2>/Rate Limiter'
};

// Block states (default storage) for model 'BallastResistor'
struct BallastResistor_DW_f {
  real_T UnitDelay_DSTATE;             // '<S1>/Unit Delay'
  real_T PrevY;                        // '<S2>/Rate Limiter'
  real_T LastMajorTime;                // '<S2>/Rate Limiter'
  boolean_T PrevLimited;               // '<S2>/Rate Limiter'
};

// Invariant block signals for model 'BallastResistor'
struct BallastResistor_ConstB_h {
  real_T Divide;                       // '<S3>/Divide'
  real_T Gain;                         // '<S3>/Gain'
  real_T Exp;                          // '<S3>/Exp'
};

// Real-time Model Data Structure
struct BallastResistor_tag_RTM {
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

struct BallastResistor_MdlrefDW {
  BallastResistor_B_c rtb;
  BallastResistor_DW_f rtdw;
  BallastResistor_RT_MODEL rtm;
};

// Model reference registration function
extern void BallastResistor_initialize(const char_T **rt_errorStatus,
  RTWSolverInfo *rt_solverInfo, const rtTimingBridge *timingBridge, int_T
  mdlref_TID0, int_T mdlref_TID1, BallastResistor_RT_MODEL *const
  BallastResistorrtM, BallastResistor_B_c *localB, BallastResistor_DW_f *localDW);
extern void BallastResistor_Init(BallastResistor_DW_f *localDW);
extern void BallastResistor_Update(BallastResistor_RT_MODEL * const
  BallastResistorrtM, real_T *rty_Temperature, BallastResistor_B_c *localB,
  BallastResistor_DW_f *localDW);
extern void BallastResistor(BallastResistor_RT_MODEL * const BallastResistorrtM,
  const real_T *rtu_Tmax, const real_T *rtu_P_ballast, const real_T
  *rtu_fan_enabled, real_T *rty_Temperature, boolean_T *rty_limits_exceeded,
  BallastResistor_B_c *localB, BallastResistor_DW_f *localDW);

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
//  '<Root>' : 'BallastResistor'
//  '<S1>'   : 'BallastResistor/Subsystem1'
//  '<S2>'   : 'BallastResistor/update_fan_factor'
//  '<S3>'   : 'BallastResistor/Subsystem1/Subsystem'

#endif                                 // BallastResistor_h_

//
// File trailer for generated code.
//
// [EOF]
//
