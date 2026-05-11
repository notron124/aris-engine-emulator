//
// File: FrequencyConverter.h
//
// Code generated for Simulink model 'FrequencyConverter'.
//
// Model version                  : 1.285
// Simulink Coder version         : 25.2 (R2025b) 28-Jul-2025
// C/C++ source code generated on : Mon May 11 19:29:22 2026
//
// Target selection: ert.tlc
// Embedded hardware selection: Intel->x86-64 (Linux 64)
// Code generation objectives: Unspecified
// Validation result: Not run
//
#ifndef FrequencyConverter_h_
#define FrequencyConverter_h_
#include <cmath>
#include "rtwtypes.h"
#include "rtw_continuous.h"
#include "rtw_solver.h"
#include "FrequencyConverter_types.h"
#include <cstring>

// Block states (default storage) for model 'FrequencyConverter'
struct FrequencyConverter_DW_f {
  real_T ballast_power;                // '<S1>/Data Store Memory'
  real_T omega_sync;                   // '<Root>/Data Store Memory'
};

// Invariant block signals for model 'FrequencyConverter'
struct FrequencyConverter_ConstB_h {
  real_T Product;                      // '<S2>/Product'
  real_T Gain;                         // '<S3>/Gain'
  real_T Square;                       // '<S6>/Square'
};

// Real-time Model Data Structure
struct FrequencyConverter_tag_RTM {
  const char_T **errorStatus;
  const char_T** getErrorStatusPointer() const;
  void setErrorStatusPointer(const char_T** aErrorStatusPointer);
  const char_T* getErrorStatus() const;
  void setErrorStatus(const char_T* const aErrorStatus) const;
};

struct FrequencyConverter_MdlrefDW {
  FrequencyConverter_DW_f rtdw;
  FrequencyConverter_RT_MODEL rtm;
};

// Model reference registration function
extern void FrequencyConverter_initialize(const char_T **rt_errorStatus,
  FrequencyConverter_RT_MODEL *const FrequencyConverterrtM,
  FrequencyConverter_DW_f *localDW);
extern void FrequencyConverter(const real_T *rtu_target_torque, const real_T
  *rtu_M_AD, const real_T *rtu_n_rpm_rotor, const real_T *rtu_set_omega_sync,
  real_T *rty_ballast_power, real_T *rty_omega_sync, real_T *rty_torque,
  FrequencyConverter_DW_f *localDW);

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
//  '<Root>' : 'FrequencyConverter'
//  '<S1>'   : 'FrequencyConverter/calc_ballast_power'
//  '<S2>'   : 'FrequencyConverter/set_ad_parameters'
//  '<S3>'   : 'FrequencyConverter/set_target_torque'
//  '<S4>'   : 'FrequencyConverter/calc_ballast_power/If Action Subsystem'
//  '<S5>'   : 'FrequencyConverter/calc_ballast_power/If Action Subsystem1'
//  '<S6>'   : 'FrequencyConverter/set_target_torque/Subsystem'

#endif                                 // FrequencyConverter_h_

//
// File trailer for generated code.
//
// [EOF]
//
