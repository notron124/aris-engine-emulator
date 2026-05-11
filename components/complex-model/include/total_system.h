//
// File: total_system.h
//
// Code generated for Simulink model 'total_system'.
//
// Model version                  : 1.247
// Simulink Coder version         : 25.2 (R2025b) 28-Jul-2025
// C/C++ source code generated on : Mon May 11 04:27:18 2026
//
// Target selection: ert.tlc
// Embedded hardware selection: Intel->x86-64 (Linux 64)
// Code generation objectives: Unspecified
// Validation result: Not run
//
#ifndef total_system_h_
#define total_system_h_
#include <cmath>
#include "rtwtypes.h"
#include "rtw_continuous.h"
#include "rtw_solver.h"
#include "total_system_types.h"
#include "FrequencyConverter.h"
#include "AsyncMotor.h"
#include "BallastResistor.h"
#include "ICE.h"
#include "model_reference_types.h"

// Class declaration for model total_system
namespace Model
{
  class System final
  {
    // public data and function members
   public:
    // Block signals (default storage)
    struct B {
      real_T Switch;                   // '<Root>/Switch'
      real_T Switch1;                  // '<Root>/Switch1'
      real_T Model2_o1;                // '<Root>/Model2'
      real_T M_electromagnetic;        // '<Root>/Model2'
      real_T Model2_o3;                // '<Root>/Model2'
      real_T Model2_o4;                // '<Root>/Model2'
      real_T Model1_o1;                // '<Root>/Model1'
      real_T Switch2;                  // '<Root>/Switch2'
      real_T Model3_o1;                // '<Root>/Model3'
      boolean_T Model2_o5;             // '<Root>/Model2'
      boolean_T Model1_o2;             // '<Root>/Model1'
    };

    // Block states (default storage) for system '<Root>'
    struct DW {
      real_T UnitDelay1_DSTATE;        // '<Root>/Unit Delay1'
      real_T UnitDelay_DSTATE;         // '<Root>/Unit Delay'
      real_T UnitDelay2_DSTATE;        // '<Root>/Unit Delay2'
      FrequencyConverter_MdlrefDW Model_InstanceData;// '<Root>/Model'
      AsyncMotor_MdlrefDW Model2_InstanceData;// '<Root>/Model2'
      BallastResistor_MdlrefDW Model1_InstanceData;// '<Root>/Model1'
      ICE_MdlrefDW Model3_InstanceData;// '<Root>/Model3'
    };

    // External inputs (root inport signals with default storage)
    struct ExtU {
      real_T T_ballast_max;            // '<Root>/T_ballast_max'
      real_T ballast_fan_enabled;      // '<Root>/ballast_fan_enabled'
      real_T ad_fan_enabled;           // '<Root>/ad_fan_enabled'
      real_T T_AD_max;                 // '<Root>/T_AD_max'
      real_T M_AD_target;              // '<Root>/M_AD_target'
      real_T P_oil_max;                // '<Root>/P_oil_max'
      real_T omega_ICE_max_run;        // '<Root>/omega_ICE_max_run'
      real_T ICE_target_n_rpm;         // '<Root>/ICE_target_n_rpm'
      real_T P_oil_min;                // '<Root>/P_oil_min'
      real_T test_ICE_set_omega;       // '<Root>/test_ICE_set_omega'
      real_T test_FC_set_omega_sync;   // '<Root>/test_FC_set_omega_sync'
      real_T test_AD_omega_ICE;        // '<Root>/test_AD_omega_ICE'
      real_T test_AD_omega_sync;       // '<Root>/test_AD_omega_sync'
      real_T ICE_fan_enabled;          // '<Root>/ICE_fan_enabled'
      real_T test_FC_n_rpm_rotor;      // '<Root>/test_FC_n_rpm_rotor'
      real_T test_FC_M_AD;             // '<Root>/test_FC_M_AD'
    };

    // External outputs (root outports fed by signals with default storage)
    struct ExtY {
      real_T T_AD;                     // '<Root>/T_AD'
      real_T T_ballast;                // '<Root>/T_ballast'
      real_T M_AD;                     // '<Root>/M_AD'
      boolean_T AD_limits_exceeded;    // '<Root>/AD_limits_exceeded'
      real_T T_cool;                   // '<Root>/T_cool'
      real_T P_oil;                    // '<Root>/P_oil'
      real_T n_rpm_AD;                 // '<Root>/n_rpm_AD'
      boolean_T ballast_limit_exceeded;// '<Root>/ballast_limit_exceeded'
      real_T FC_poles;                 // '<Root>/FC_poles'
      real_T FC_omega_sync;            // '<Root>/FC_omega_sync'
      real_T target_torque;            // '<Root>/target_torque'
      real_T ballast_power;            // '<Root>/ballast_power'
    };

    // Real-time Model Data Structure
    struct RT_MODEL {
      const char_T *errorStatus;
      RTWSolverInfo solverInfo;
      rtTimingBridge timingBridge;

      //
      //  Timing:
      //  The following substructure contains information regarding
      //  the timing information for the model.

      struct {
        uint32_T clockTick0;
        time_T stepSize0;
        uint32_T clockTick1;
        SimTimeStep simTimeStep;
        time_T *t;
        time_T tArray[2];
      } Timing;

      time_T** getTPtrPtr();
      time_T* getTPtr() const;
      void setTPtr(time_T* aTPtr);
      const char_T** getErrorStatusPointer();
      boolean_T isMinorTimeStep() const;
      boolean_T isMajorTimeStep() const;
      const char_T** getErrorStatusPtr();
      const char_T* getErrorStatus() const;
      void setErrorStatus(const char_T* const aErrorStatus);
    };

    // Copy Constructor
    System(System const&) = delete;

    // Assignment Operator
    System& operator= (System const&) & = delete;

    // Move Constructor
    System(System &&) = delete;

    // Move Assignment Operator
    System& operator= (System &&) = delete;

    // Real-Time Model get method
    System::RT_MODEL * getRTM();

    // External inputs
    ExtU rtU;

    // External outputs
    ExtY rtY;

    // model initialize function
    void initialize();

    // model step function
    void step();

    // model terminate function
    static void terminate();

    // Constructor
    System();

    // Destructor
    ~System();

    // private data and function members
   private:
    // Block signals
    B rtB;

    // Block states
    DW rtDW;

    // Real-Time Model
    RT_MODEL rtM;
  };
}

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
//  '<Root>' : 'total_system'

#endif                                 // total_system_h_

//
// File trailer for generated code.
//
// [EOF]
//
