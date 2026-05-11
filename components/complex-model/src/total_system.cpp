//
// File: total_system.cpp
//
// Code generated for Simulink model 'total_system'.
//
// Model version                  : 1.257
// Simulink Coder version         : 25.2 (R2025b) 28-Jul-2025
// C/C++ source code generated on : Mon May 11 19:17:10 2026
//
// Target selection: ert.tlc
// Embedded hardware selection: Intel->x86-64 (Linux 64)
// Code generation objectives: Unspecified
// Validation result: Not run
//
#include "total_system.h"
#include "rtwtypes.h"
#include "FrequencyConverter.h"
#include "BallastResistor.h"
#include "AsyncMotor.h"
#include "ICE.h"

namespace Model
{
  // Model step function
  void System::step()
  {
    // local block i/o variables
    real_T rtb_Model3_o3;
    real_T rtb_Switch3;
    real_T rtb_Switch4;

    // Switch: '<Root>/Switch4' incorporates:
    //   Inport: '<Root>/test_FC_M_AD'
    //   UnitDelay: '<Root>/Unit Delay1'

    if (rtU.test_FC_M_AD != 0.0) {
      rtb_Switch4 = rtU.test_FC_M_AD;
    } else {
      rtb_Switch4 = rtDW.UnitDelay1_DSTATE;
    }

    // End of Switch: '<Root>/Switch4'

    // Switch: '<Root>/Switch3' incorporates:
    //   Inport: '<Root>/test_FC_n_rpm_rotor'
    //   UnitDelay: '<Root>/Unit Delay'

    if (rtU.test_FC_n_rpm_rotor != 0.0) {
      rtb_Switch3 = rtU.test_FC_n_rpm_rotor;
    } else {
      rtb_Switch3 = rtDW.UnitDelay_DSTATE;
    }

    // End of Switch: '<Root>/Switch3'

    // ModelReference: '<Root>/Model' incorporates:
    //   Inport: '<Root>/M_AD_target'
    //   Inport: '<Root>/test_FC_set_omega_sync'
    //   Outport: '<Root>/FC_omega_sync'
    //   Outport: '<Root>/ballast_power'
    //   Outport: '<Root>/target_torque'

    FrequencyConverter(&rtU.M_AD_target, &rtb_Switch4, &rtb_Switch3,
                       &rtU.test_FC_set_omega_sync, &rtY.ballast_power,
                       &rtY.FC_omega_sync, &rtY.target_torque,
                       &(rtDW.Model_InstanceData.rtdw));

    // Switch: '<Root>/Switch' incorporates:
    //   Inport: '<Root>/test_AD_omega_sync'

    if (rtU.test_AD_omega_sync != 0.0) {
      // Switch: '<Root>/Switch'
      rtB.Switch = rtU.test_AD_omega_sync;
    } else {
      // Switch: '<Root>/Switch' incorporates:
      //   Outport: '<Root>/FC_omega_sync'

      rtB.Switch = rtY.FC_omega_sync;
    }

    // End of Switch: '<Root>/Switch'

    // Switch: '<Root>/Switch1' incorporates:
    //   Inport: '<Root>/test_AD_omega_ICE'

    if (rtU.test_AD_omega_ICE != 0.0) {
      // Switch: '<Root>/Switch1'
      rtB.Switch1 = rtU.test_AD_omega_ICE;
    } else {
      // Switch: '<Root>/Switch1' incorporates:
      //   UnitDelay: '<Root>/Unit Delay2'

      rtB.Switch1 = rtDW.UnitDelay2_DSTATE;
    }

    // End of Switch: '<Root>/Switch1'

    // ModelReference: '<Root>/Model2' incorporates:
    //   Inport: '<Root>/T_AD_max'
    //   Inport: '<Root>/ad_fan_enabled'

    AsyncMotor(&(rtDW.Model2_InstanceData.rtm), &rtB.Switch, &rtB.Switch1,
               &rtU.ad_fan_enabled, &rtU.T_AD_max, &rtB.Model2_o1,
               &rtB.M_electromagnetic, &rtB.Model2_o3, &rtB.Model2_o4,
               &rtB.Model2_o5, &(rtDW.Model2_InstanceData.rtb),
               &(rtDW.Model2_InstanceData.rtdw));

    // ModelReference: '<Root>/Model1' incorporates:
    //   Inport: '<Root>/T_ballast_max'
    //   Inport: '<Root>/ballast_fan_enabled'
    //   Outport: '<Root>/ballast_power'

    BallastResistor(&(rtDW.Model1_InstanceData.rtm), &rtU.T_ballast_max,
                    &rtY.ballast_power, &rtU.ballast_fan_enabled, &rtB.Model1_o1,
                    &rtB.Model1_o2, &(rtDW.Model1_InstanceData.rtb),
                    &(rtDW.Model1_InstanceData.rtdw));

    // Switch: '<Root>/Switch2' incorporates:
    //   Inport: '<Root>/test_ICE_set_omega'

    if (rtU.test_ICE_set_omega != 0.0) {
      // Switch: '<Root>/Switch2'
      rtB.Switch2 = rtU.test_ICE_set_omega;
    } else {
      // Switch: '<Root>/Switch2'
      rtB.Switch2 = rtB.Model2_o1;
    }

    // End of Switch: '<Root>/Switch2'

    // Outport: '<Root>/P_oil' incorporates:
    //   Inport: '<Root>/ICE_fan_enabled'
    //   Inport: '<Root>/ICE_target_n_rpm'
    //   Inport: '<Root>/P_oil_max'
    //   Inport: '<Root>/P_oil_min'
    //   Inport: '<Root>/omega_ICE_max_run'
    //   ModelReference: '<Root>/Model3'

    ICE(&(rtDW.Model3_InstanceData.rtm), &rtU.ICE_target_n_rpm,
        &rtU.omega_ICE_max_run, &rtB.M_electromagnetic, &rtU.P_oil_max,
        &rtU.P_oil_min, &rtB.Switch2, &rtU.ICE_fan_enabled, &rtB.Model3_o1,
        &rtY.P_oil, &rtb_Model3_o3, &(rtDW.Model3_InstanceData.rtb),
        &(rtDW.Model3_InstanceData.rtdw));

    // Outport: '<Root>/T_cool'
    rtY.T_cool = rtB.Model3_o1;

    // Outport: '<Root>/T_ballast'
    rtY.T_ballast = rtB.Model1_o1;

    // Outport: '<Root>/ballast_limit_exceeded'
    rtY.ballast_limit_exceeded = rtB.Model1_o2;

    // Outport: '<Root>/T_AD'
    rtY.T_AD = rtB.Model2_o4;

    // Outport: '<Root>/M_AD'
    rtY.M_AD = rtB.M_electromagnetic;

    // Outport: '<Root>/AD_limits_exceeded'
    rtY.AD_limits_exceeded = rtB.Model2_o5;

    // Outport: '<Root>/n_rpm_AD'
    rtY.n_rpm_AD = rtB.Model2_o1;

    // Update for UnitDelay: '<Root>/Unit Delay1'
    rtDW.UnitDelay1_DSTATE = rtB.M_electromagnetic;

    // Update for UnitDelay: '<Root>/Unit Delay'
    rtDW.UnitDelay_DSTATE = rtB.Model2_o1;

    // Update for UnitDelay: '<Root>/Unit Delay2'
    rtDW.UnitDelay2_DSTATE = rtb_Model3_o3;

    // Update for ModelReference: '<Root>/Model2'
    AsyncMotor_Update(&(rtDW.Model2_InstanceData.rtm), &rtB.Model2_o4,
                      &(rtDW.Model2_InstanceData.rtb),
                      &(rtDW.Model2_InstanceData.rtdw));

    // Update for ModelReference: '<Root>/Model1'
    BallastResistor_Update(&(rtDW.Model1_InstanceData.rtm), &rtB.Model1_o1,
      &(rtDW.Model1_InstanceData.rtb), &(rtDW.Model1_InstanceData.rtdw));

    // Update for ModelReference: '<Root>/Model3'
    ICE_Update(&(rtDW.Model3_InstanceData.rtm), &rtB.Model3_o1,
               &(rtDW.Model3_InstanceData.rtb), &(rtDW.Model3_InstanceData.rtdw));

    // Update absolute time for base rate
    // The "clockTick0" counts the number of times the code of this task has
    //  been executed. The absolute time is the multiplication of "clockTick0"
    //  and "Timing.stepSize0". Size of "clockTick0" ensures timer will not
    //  overflow during the application lifespan selected.

    (&rtM)->Timing.t[0] =
      ((time_T)(++(&rtM)->Timing.clockTick0)) * (&rtM)->Timing.stepSize0;

    {
      // Update absolute timer for sample time: [0.001s, 0.0s]
      // The "clockTick1" counts the number of times the code of this task has
      //  been executed. The resolution of this integer timer is 0.001, which is the step size
      //  of the task. Size of "clockTick1" ensures timer will not overflow during the
      //  application lifespan selected.

      (&rtM)->Timing.clockTick1++;
    }
  }

  // Model initialize function
  void System::initialize()
  {
    // Registration code
    {
      // Setup solver object
      rtsiSetSimTimeStepPtr(&(&rtM)->solverInfo, &(&rtM)->Timing.simTimeStep);
      rtsiSetTPtr(&(&rtM)->solverInfo, (&rtM)->getTPtrPtr());
      rtsiSetStepSizePtr(&(&rtM)->solverInfo, &(&rtM)->Timing.stepSize0);
      rtsiSetErrorStatusPtr(&(&rtM)->solverInfo, (&rtM)->getErrorStatusPtr());
      rtsiSetRTModelPtr(&(&rtM)->solverInfo, (&rtM));
    }

    rtsiSetSimTimeStep(&(&rtM)->solverInfo, MAJOR_TIME_STEP);
    rtsiSetIsMinorTimeStepWithModeChange(&(&rtM)->solverInfo, false);
    rtsiSetIsContModeFrozen(&(&rtM)->solverInfo, false);
    rtsiSetSolverName(&(&rtM)->solverInfo,"FixedStepDiscrete");
    (&rtM)->setTPtr(&(&rtM)->Timing.tArray[0]);
    (&rtM)->Timing.stepSize0 = 0.001;

    {
      static uint32_T *clockTickPtrs[2];
      static real_T *taskTimePtrs[2];
      (&rtM)->timingBridge.nTasks = 2;
      clockTickPtrs[0] = &((&rtM)->Timing.clockTick0);
      clockTickPtrs[1] = &((&rtM)->Timing.clockTick1);
      (&rtM)->timingBridge.clockTick = clockTickPtrs;
      (&rtM)->timingBridge.clockTickH = (nullptr);
      taskTimePtrs[0] = &((&rtM)->Timing.t[0]);
      taskTimePtrs[1] = (nullptr);
      (&rtM)->timingBridge.taskTime = taskTimePtrs;
    }

    // Model Initialize function for ModelReference Block: '<Root>/Model'
    FrequencyConverter_initialize((&rtM)->getErrorStatusPointer(),
      &(rtDW.Model_InstanceData.rtm), &(rtDW.Model_InstanceData.rtdw));

    // Model Initialize function for ModelReference Block: '<Root>/Model1'
    BallastResistor_initialize((&rtM)->getErrorStatusPointer(), &((&rtM)
      ->solverInfo), &(&rtM)->timingBridge, 0, 1, &(rtDW.Model1_InstanceData.rtm),
      &(rtDW.Model1_InstanceData.rtb), &(rtDW.Model1_InstanceData.rtdw));

    // Model Initialize function for ModelReference Block: '<Root>/Model2'
    AsyncMotor_initialize((&rtM)->getErrorStatusPointer(), &((&rtM)->solverInfo),
                          &(&rtM)->timingBridge, 0, 1,
                          &(rtDW.Model2_InstanceData.rtm),
                          &(rtDW.Model2_InstanceData.rtb),
                          &(rtDW.Model2_InstanceData.rtdw));

    // Model Initialize function for ModelReference Block: '<Root>/Model3'
    ICE_initialize((&rtM)->getErrorStatusPointer(), &((&rtM)->solverInfo),
                   &(&rtM)->timingBridge, 0, 1, &(rtDW.Model3_InstanceData.rtm),
                   &(rtDW.Model3_InstanceData.rtb),
                   &(rtDW.Model3_InstanceData.rtdw));

    // SystemInitialize for ModelReference: '<Root>/Model2'
    AsyncMotor_Init(&rtB.Model2_o4, &(rtDW.Model2_InstanceData.rtb),
                    &(rtDW.Model2_InstanceData.rtdw));

    // SystemInitialize for ModelReference: '<Root>/Model1'
    BallastResistor_Init(&(rtDW.Model1_InstanceData.rtdw));

    // SystemInitialize for ModelReference: '<Root>/Model3'
    ICE_Init(&rtB.Model3_o1, &(rtDW.Model3_InstanceData.rtdw));

    // ConstCode for Outport: '<Root>/FC_poles' incorporates:
    //   Constant: '<Root>/Constant'

    rtY.FC_poles = 2.0;
  }

  // Model terminate function
  void System::terminate()
  {
    // (no terminate code required)
  }

  time_T** System::RT_MODEL::getTPtrPtr()
  {
    return &(Timing.t);
  }

  time_T* System::RT_MODEL::getTPtr() const
  {
    return (Timing.t);
  }

  void System::RT_MODEL::setTPtr(time_T* aTPtr)
  {
    (Timing.t = aTPtr);
  }

  const char_T** System::RT_MODEL::getErrorStatusPointer()
  {
    return &errorStatus;
  }

  boolean_T System::RT_MODEL::isMinorTimeStep() const
  {
    return ((Timing.simTimeStep) == MINOR_TIME_STEP);
  }

  boolean_T System::RT_MODEL::isMajorTimeStep() const
  {
    return ((Timing.simTimeStep) == MAJOR_TIME_STEP);
  }

  const char_T** System::RT_MODEL::getErrorStatusPtr()
  {
    return &errorStatus;
  }

  const char_T* System::RT_MODEL::getErrorStatus() const
  {
    return (errorStatus);
  }

  void System::RT_MODEL::setErrorStatus(const char_T* const aErrorStatus)
  {
    (errorStatus = aErrorStatus);
  }

  // Constructor
  System::System() :
    rtU(),
    rtY(),
    rtB(),
    rtDW(),
    rtM()
  {
    // Currently there is no constructor body generated.
  }

  // Destructor
  // Currently there is no destructor body generated.
  System::~System() = default;

  // Real-Time Model get method
  System::RT_MODEL * System::getRTM()
  {
    return (&rtM);
  }
}

//
// File trailer for generated code.
//
// [EOF]
//
