//
// File: FrequencyConverter.cpp
//
// Code generated for Simulink model 'FrequencyConverter'.
//
// Model version                  : 1.267
// Simulink Coder version         : 25.2 (R2025b) 28-Jul-2025
// C/C++ source code generated on : Mon May 11 04:13:52 2026
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
  const real_T *rtu_n_rpm_rotor, real_T *rty_ballast_power, real_T
  *rty_omega_sync, real_T *rty_torque, FrequencyConverter_DW_f *localDW)
{
  // DataStoreWrite: '<Root>/Data Store Write1  ' incorporates:
  //   DataStoreRead: '<S6>/Data Store Read'

  localDW->omega_sync_c = localDW->omega_sync;

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
  }

  // End of If: '<S1>/If'

  // MinMax: '<S3>/Max' incorporates:
  //   MinMax: '<S3>/Min'

  *rty_torque = std::fmax(std::fmin(*rtu_target_torque,
    FrequencyConverterrtConstB.Product), FrequencyConverterrtConstB.Gain);

  // If: '<S6>/If' incorporates:
  //   Abs: '<S6>/Abs'

  if (std::abs(*rty_torque) == 0.0) {
    // Outputs for IfAction SubSystem: '<S6>/If Action Subsystem' incorporates:
    //   ActionPort: '<S7>/Action Port'

    // DataStoreWrite: '<S7>/Data Store Write' incorporates:
    //   Constant: '<S7>/Zero'

    localDW->omega_sync = 0.0;

    // End of Outputs for SubSystem: '<S6>/If Action Subsystem'
  } else {
    real_T rtb_M_req_abs;

    // Outputs for IfAction SubSystem: '<S6>/If Action Subsystem1' incorporates:
    //   ActionPort: '<S8>/Action Port'

    // Abs: '<S8>/Abs1'
    rtb_M_req_abs = std::abs(*rty_torque);

    // Product: '<S8>/Product' incorporates:
    //   Constant: '<S8>/Constant1'
    //   Math: '<S8>/Square1'
    //   Product: '<S8>/Divide'
    //   Sqrt: '<S8>/Sqrt'
    //   Sum: '<S8>/Minus'
    //   Sum: '<S8>/Plus'

    rtb_M_req_abs = (std::sqrt(FrequencyConverterrtConstB.Square - rtb_M_req_abs
      * rtb_M_req_abs) + FrequencyConverterrtConstB.Product) / rtb_M_req_abs *
      0.05;

    // Switch: '<S8>/Switch' incorporates:
    //   Gain: '<S8>/Gain1'

    if (rtb_M_req_abs > 0.0) {
      rtb_M_req_abs = *rty_torque;
    } else {
      rtb_M_req_abs = -rtb_M_req_abs;
    }

    // Product: '<S8>/Divide1' incorporates:
    //   Constant: '<S8>/Constant'
    //   Constant: '<S8>/Constant2'
    //   DataStoreWrite: '<S8>/Data Store Write'
    //   Product: '<S8>/Product1'
    //   Sum: '<S8>/Minus1'
    //   Switch: '<S8>/Switch'

    localDW->omega_sync = 0.20943951023931953 * *rtu_n_rpm_rotor / (1.0 -
      rtb_M_req_abs);

    // End of Outputs for SubSystem: '<S6>/If Action Subsystem1'
  }

  // End of If: '<S6>/If'

  // DataStoreRead: '<Root>/Data Store Read1'
  *rty_omega_sync = localDW->omega_sync_c;
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
