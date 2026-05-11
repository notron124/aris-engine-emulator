//
// File: FrequencyConverter.cpp
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
#include "FrequencyConverter.h"
#include "rtwtypes.h"
#include <cmath>
#include "FrequencyConverter_private.h"
#include <cstring>

// Output and update for referenced model: 'FrequencyConverter'
void FrequencyConverter(const real_T *rtu_target_torque, const real_T *rtu_M_AD,
  const real_T *rtu_n_rpm_rotor, const real_T *rtu_set_omega_sync, real_T
  *rty_ballast_power, real_T *rty_omega_sync, real_T *rty_torque,
  FrequencyConverter_DW_f *localDW)
{
  real_T rtb_M_req_abs;

  // MinMax: '<S3>/Max' incorporates:
  //   MinMax: '<S3>/Min'

  *rty_torque = std::fmax(std::fmin(*rtu_target_torque,
    FrequencyConverterrtConstB.Product), FrequencyConverterrtConstB.Gain);

  // Abs: '<S6>/Abs'
  rtb_M_req_abs = std::abs(*rty_torque);

  // Switch: '<Root>/Switch' incorporates:
  //   DataStoreWrite: '<Root>/Data Store Write'
  //   Switch: '<S6>/Switch'

  if (*rtu_set_omega_sync != 0.0) {
    localDW->omega_sync = *rtu_set_omega_sync;
  } else if (rtb_M_req_abs >= 0.001) {
    // Switch: '<S6>/Switch1' incorporates:
    //   Constant: '<S6>/Constant1'
    //   Gain: '<S6>/Gain1'
    //   Math: '<S6>/Square1'
    //   Product: '<S6>/Divide'
    //   Product: '<S6>/Product'
    //   Sqrt: '<S6>/Sqrt'
    //   Sum: '<S6>/Minus'
    //   Sum: '<S6>/Plus'
    //   Switch: '<S6>/Switch'

    if (*rty_torque > 0.0) {
      rtb_M_req_abs = (std::sqrt(FrequencyConverterrtConstB.Square -
        rtb_M_req_abs * rtb_M_req_abs) + FrequencyConverterrtConstB.Product) /
        rtb_M_req_abs * 0.05;
    } else {
      rtb_M_req_abs = -((std::sqrt(FrequencyConverterrtConstB.Square -
        rtb_M_req_abs * rtb_M_req_abs) + FrequencyConverterrtConstB.Product) /
                        rtb_M_req_abs * 0.05);
    }

    // End of Switch: '<S6>/Switch1'

    // DataStoreWrite: '<Root>/Data Store Write' incorporates:
    //   Constant: '<S6>/Constant'
    //   Constant: '<S6>/Constant2'
    //   Product: '<S6>/Divide1'
    //   Product: '<S6>/Product1'
    //   Sum: '<S6>/Minus1'
    //   Switch: '<S6>/Switch'

    localDW->omega_sync = 0.20943951023931953 * *rtu_n_rpm_rotor / (1.0 -
      rtb_M_req_abs);
  } else {
    // DataStoreWrite: '<Root>/Data Store Write' incorporates:
    //   Constant: '<S6>/Zero'
    //   Switch: '<S6>/Switch'

    localDW->omega_sync = 0.0;
  }

  // End of Switch: '<Root>/Switch'

  // DataStoreRead: '<S1>/Data Store Read'
  *rty_ballast_power = localDW->ballast_power;

  // If: '<S1>/If'
  if (*rtu_M_AD > 0.0) {
    // Outputs for IfAction SubSystem: '<S1>/If Action Subsystem' incorporates:
    //   ActionPort: '<S4>/Action Port'

    // DataStoreWrite: '<S4>/Data Store Write' incorporates:
    //   Constant: '<S4>/Zero'

    localDW->ballast_power = 0.0;

    // End of Outputs for SubSystem: '<S1>/If Action Subsystem'
  } else {
    // Outputs for IfAction SubSystem: '<S1>/If Action Subsystem1' incorporates:
    //   ActionPort: '<S5>/Action Port'

    // MinMax: '<S5>/Max' incorporates:
    //   Constant: '<S5>/Constant'
    //   Constant: '<S5>/Zero'
    //   DataStoreRead: '<Root>/Data Store Read'
    //   DataStoreWrite: '<S5>/Data Store Write'
    //   Product: '<S5>/Product'
    //   Product: '<S5>/Product1'
    //   Sum: '<S5>/Minus'

    localDW->ballast_power = std::fmax(0.0, (localDW->omega_sync -
      *rtu_n_rpm_rotor * 0.20943951023931953) * *rtu_M_AD);

    // End of Outputs for SubSystem: '<S1>/If Action Subsystem1'
  }

  // End of If: '<S1>/If'

  // DataStoreRead: '<Root>/Data Store Read1'
  *rty_omega_sync = localDW->omega_sync;
}

// Model initialize function
void FrequencyConverter_initialize(const char_T **rt_errorStatus,
  FrequencyConverter_RT_MODEL *const FrequencyConverterrtM,
  FrequencyConverter_DW_f *localDW)
{
  // Registration code

  // initialize error status
  FrequencyConverterrtM->setErrorStatusPointer(rt_errorStatus);

  // states (dwork)
  (void) std::memset(static_cast<void *>(localDW), 0,
                     sizeof(FrequencyConverter_DW_f));
}

const char_T** FrequencyConverter_RT_MODEL::getErrorStatusPointer() const
{
  return errorStatus;
}

void FrequencyConverter_RT_MODEL::setErrorStatusPointer(const char_T
  ** aErrorStatusPointer)
{
  (errorStatus = aErrorStatusPointer);
}

const char_T* FrequencyConverter_RT_MODEL::getErrorStatus() const
{
  return (*(errorStatus));
}

void FrequencyConverter_RT_MODEL::setErrorStatus(const char_T* const
  aErrorStatus) const
{
  (*(errorStatus) = aErrorStatus);
}

//
// File trailer for generated code.
//
// [EOF]
//
