//
// File: ICE.h
//
// Code generated for Simulink model 'ICE'.
//
// Model version                  : 1.460
// Simulink Coder version         : 25.2 (R2025b) 28-Jul-2025
// C/C++ source code generated on : Mon May 11 04:07:54 2026
//
// Target selection: ert.tlc
// Embedded hardware selection: Intel->x86-64 (Linux 64)
// Code generation objectives: Unspecified
// Validation result: Not run
//
#ifndef ICE_h_
#define ICE_h_
#include <cmath>
#include "rtwtypes.h"
#include "rtw_continuous.h"
#include "rtw_solver.h"
#include "rt_nonfinite.h"
#include "ICE_types.h"
#include "rtGetInf.h"
#include "model_reference_types.h"
#include <cstring>

// Block signals for model 'ICE'
struct ICE_B_c {
  real_T UnitDelay;                    // '<S1>/Unit Delay'
  real_T RateLimiter;                  // '<S1>/Rate Limiter'
  real_T UnitDelay_o;                  // '<S4>/Unit Delay'
  real_T M_internal_fric;              // '<S4>/Plus'
  real_T omega_ice;                    // '<S4>/Max'
  real_T Product;                      // '<S6>/Product'
};

// Block states (default storage) for model 'ICE'
struct ICE_DW_f {
  real_T UnitDelay_DSTATE;             // '<Root>/Unit Delay'
  real_T UnitDelay_DSTATE_j;           // '<S1>/Unit Delay'
  real_T UnitDelay_DSTATE_h;           // '<S4>/Unit Delay'
  real_T PrevY;                        // '<S1>/Rate Limiter'
  real_T LastMajorTime;                // '<S1>/Rate Limiter'
  real_T IC_FirstOutputTime;           // '<S1>/IC'
  real_T n_rpm;                        // '<Root>/Data Store Memory'
  int8_T If_ActiveSubsystem;           // '<S2>/If'
  boolean_T PrevLimited;               // '<S1>/Rate Limiter'
};

// Invariant block signals for model 'ICE'
struct ICE_ConstB_h {
  real_T Divide1;                      // '<S1>/Divide1'
  real_T Divide;                       // '<S5>/Divide'
  real_T Gain;                         // '<S5>/Gain'
  real_T Exp;                          // '<S5>/Exp'
};

// Real-time Model Data Structure
struct ICE_tag_RTM {
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

struct ICE_MdlrefDW {
  ICE_B_c rtb;
  ICE_DW_f rtdw;
  ICE_RT_MODEL rtm;
};

// Model reference registration function
extern void ICE_initialize(const char_T **rt_errorStatus, RTWSolverInfo
  *rt_solverInfo, const rtTimingBridge *timingBridge, int_T mdlref_TID0, int_T
  mdlref_TID1, ICE_RT_MODEL *const ICErtM, ICE_B_c *localB, ICE_DW_f *localDW);
extern void ICE_Init(real_T *rty_temperature, ICE_DW_f *localDW);
extern void ICE_Update(ICE_RT_MODEL * const ICErtM, real_T *rty_temperature,
  ICE_B_c *localB, ICE_DW_f *localDW);
extern void ICE(ICE_RT_MODEL * const ICErtM, const real_T *rtu_target_n_rpm,
                const real_T *rtu_n_rpm_max_run, const real_T *rtu_M_AD_torque,
                const real_T *rtu_P_oil_max, const real_T *rtu_P_oil_min, const
                real_T *rtu_set_n_rpm, const real_T *rtu_fan_enabled, real_T
                *rty_temperature, real_T *rty_oil_preasure, real_T
                *rty_n_rpm_ICE, ICE_B_c *localB, ICE_DW_f *localDW);

//-
//  These blocks were eliminated from the model due to optimizations:
//
//  Block '<Root>/Constant1' : Unused code path elimination
//  Block '<Root>/Product1' : Unused code path elimination


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
//  '<Root>' : 'ICE'
//  '<S1>'   : 'ICE/Heat part'
//  '<S2>'   : 'ICE/Preasure_part'
//  '<S3>'   : 'ICE/Subsystem'
//  '<S4>'   : 'ICE/motion_part'
//  '<S5>'   : 'ICE/Heat part/Subsystem1'
//  '<S6>'   : 'ICE/Preasure_part/If Action Subsystem'
//  '<S7>'   : 'ICE/Subsystem/get_max_torque_at_speed'
//  '<S8>'   : 'ICE/Subsystem/get_max_torque_at_speed/MATLAB Function'

#endif                                 // ICE_h_

//
// File trailer for generated code.
//
// [EOF]
//
