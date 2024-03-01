/*
 ============================================================================
 Name        : move_motor routines, trinamic development
 Author      : Steve Burke
 Copyright   : Copyright FF
 ============================================================================
 */

#include <iostream>
#include <math.h>
#include <unistd.h>
#include "Fulfil.Dispense/motion/registers.h"
#include "Fulfil.Dispense/motion/motion_timer.h"
#include "Fulfil.Dispense/motion/trinamic.h"
#include <Fulfil.CPPUtils/logging.h>
#include <stdio.h>
#include <Fulfil.CPPUtils/inih/INIReader.h>

using namespace std;
using fulfil::utils::Logger;
using fulfil::dispense::motion::Trinamic;

int RunMotorRoutines(Trinamic motor_controller)
{
  int microstep_setting = 3;
  int steps_per_revolution = 200;
  float desired_rev_per_second = 1;
  int max_positioning_speed = 100; //in pps (= microsteps / second)

  int microsteps_per_step = pow(2.0, microstep_setting);
  int microsteps_per_rev = microsteps_per_step * steps_per_revolution;
  int velocity_setting = round(microsteps_per_rev * desired_rev_per_second);

  std::cout << "Current settings indicate " << microsteps_per_rev << " microsteps per single revolution" << std::endl;
  std::cout << "Desired revolution per second speed is: " << desired_rev_per_second << std::endl;
  std::cout << "Therefore velocity setting in pps (microstep/sec) is: " << velocity_setting << std::endl;

  sleep(1);

  motor_controller.SendMotorCommand(0x02, TMCL_SAP, MOT_MaxCurrent, 0, 8);
  motor_controller.SendMotorCommand(0x02, TMCL_SAP, MOT_StandbyCurrent, 0, 5);
  motor_controller.SendMotorCommand(0x02, TMCL_SAP, 140, 0, microstep_setting);//microstep resolution, value = 2^X = 2^3 = 8 microsteps / full step
  motor_controller.SendMotorCommand(0x02, TMCL_SAP, 202, 0, steps_per_revolution);//Motor fullstep resolution (default is 200). Full steps per revolution


  /**
   *  Motor movements and reading motor axis parameters (status)
   */

  motor_controller.update_and_print_motor_status();

  std::cout << "Starting motor movement now" << std::endl << std::endl;
  sleep (2);
  motor_controller.SendMotorCommand(0x02,TMCL_MST, 0, 0, 0);  //Stop motor command
  sleep(2);
  motor_controller.SendMotorCommand(0x02,TMCL_ROR, 0, 0, velocity_setting);  //Rotate right, value = pps (= microsteps/sec)

  motor_controller.update_and_print_motor_status();
  sleep(2);
  motor_controller.update_and_print_motor_status();

  /**
   *  Test Analog Input
   */
  int max_rotation_time_seconds = 5; //seconds
  float sleep_time_ms = 100; //milliseconds
  int remaining_loops = max_rotation_time_seconds * (1000/sleep_time_ms);
  int sensor_threshold = 4000;
  std::cout << "remaining_loops is: " << remaining_loops << std::endl;
  while(remaining_loops > 0)
  {
    std::cout << "Stopping motor as soon as inductive sensor analog input goes high" << std::endl;

    int analog_input = motor_controller.GetAnalogSensorInput();
    if(analog_input > sensor_threshold)
    {
      std::cout << "Analog sensor has been triggered, stopping motor now!" << std::endl;
      break;
    }
    std::cout << "Analog input reads: " << motor_controller.GetAnalogSensorInput() << std::endl;
    usleep(sleep_time_ms * 1000);
    remaining_loops--;
  }

  motor_controller.SendMotorCommand(0x02,TMCL_MST, 0, 0, 0);  //Stop motor command
  sleep(2);
  motor_controller.update_and_print_motor_status();

  /**
   *  Start moving to set positions after resetting absolute position to zero
   */
  std::cout << "Updating position of motor now!" << std::endl;
  motor_controller.SendMotorCommand(0x02, TMCL_SAP, MOT_ActualPosition, 0, 0);
  motor_controller.update_and_print_motor_status();

  //move motor to new position
  motor_controller.SendMotorCommand(0x02, TMCL_SAP, MOT_MaxPositionSpeed, 0, velocity_setting*10);
  //motor_controller.SendMotorCommand(0x02, TMCL_SAP, MOT_MaxAcceleration, 0, 0);
  motor_controller.SendMotorCommand(0x02, TMCL_MVP, MVP_ABS, 0, -5000);
  sleep(2);

  motor_controller.SendMotorCommand(0x02, TMCL_MVP, MVP_ABS, 0, -3000);
  sleep(2);

  motor_controller.SendMotorCommand(0x02, TMCL_MVP, MVP_ABS, 0, -10000);
  sleep(5);
  motor_controller.update_and_print_motor_status();
  motor_controller.SendMotorCommand(0x02,TMCL_MST, 0, 0, 0);  //Stop motor command


  /**

  /**
   *  Homing routine testing


  std::cout << std::endl;
  std::cout << "Testing homing routine now" << std::endl;
  // Homing routine testing
  motor_controller.SendMotorCommand(0x02, TMCL_SAP, 193, 0, 7); // search home switch in positive direction, ignore end switches
  motor_controller.SendMotorCommand(0x02, TMCL_SAP, 194, 0, round(velocity_setting * 0.3));//reference search rough speed (in pps)
  motor_controller.SendMotorCommand(0x02, TMCL_SAP, 195, 0, round(velocity_setting * 0.2));//reference search fine speed (in pps)
  motor_controller.SendMotorCommand(0x02, TMCL_SIO, 0, 0, 0); //set INO/HOME pull-up resistor to OFF. //TODO: experiment w/ this for new sensor. 3rd param = 1 for setting INO pull up ON

  std::cout << "Current state of home switch is: " << motor_controller.GetHomeSwitchState() << std::endl;
  sleep(5);
  std::cout << "Commencing reference search. Home state is:" << motor_controller.GetHomeSwitchState() << std::endl;
  motor_controller.SendMotorCommand(0x02, TMCL_RFS, 0, 0, 0); //start reference search
  sleep(5);
  std::cout << "Stopping reference search. Home state is:" << motor_controller.GetHomeSwitchState() << std::endl;
  motor_controller.SendMotorCommand(0x02, TMCL_RFS, 1, 0, 0); //Stop reference search

  /**
   *  Test stall routine

   motor_controller.SendMotorCommand(0x02, TMCL_SAP, 173, 0, 1); // Enable stallguard filter (necessary??)
   motor_controller.SendMotorCommand(0x02, TMCL_SAP, 174, 0, 63); // StallGuard2 threshold for detecting stall. Default 0. Lower is higher sensitivity
   motor_controller.SendMotorCommand(0x02, TMCL_SAP, 181, 0, 0); // Motor will stop upon stall detected for speed (pps) above value: 0
   motor_controller.SendMotorCommand(0x02,TMCL_ROR, 0, 0, velocity_setting);  //Rotate right, value = pps (= microsteps/sec)

   sleep(2); //settle before start stall detection
   int stall_threshold = 63; //63 is maximum
   while(1)
   {
     int current_load_state = motor_controller.GetLoadState();
     std::cout << "Current load state is: " << current_load_state << std::endl;

     if(current_load_state == 0)
     {
       std::cout << "load state is unexpectedly zero...." << std::endl;
       break;
     }
     if(current_load_state > 300)
     {
       stall_threshold--;
       std::cout << "lowering stall threshold to: " << stall_threshold << std::endl;
       motor_controller.SendMotorCommand(0x02, TMCL_SAP, 174, 0, stall_threshold); // StallGuard2 threshold for detecting stall. Default 0. Lower is higher sensitivity
     }
     usleep(250000);
   }
   **/

  return 0;
}

int main(void)
{
  Logger* logger = Logger::Instance(Logger::default_logging_dir,"dispense_logs",Logger::Level::Debug,Logger::Level::Debug);

  INIReader motion_config_reader = INIReader("motion_config.ini", true);
  uint32_t baud = motion_config_reader.GetInteger("motion_parameters", "baud", -1);
  string com_port = motion_config_reader.Get("motion_parameters", "com_port", "error");
  Trinamic motor_controller = Trinamic(baud, com_port.c_str());

  /**
  *  Serial Connection Testing
  */

  if(!motor_controller.TryConnectMotor()) exit(1);

  RunMotorRoutines(motor_controller);

  return 0;
}

/**
bool Trinamic1241::initialize_drive() {

    enqueue_command(CanWriteSDO(address_, 0x2005, 0, 3, 4, false)); //Disable limit switches
    enqueue_command(CanWriteSDO(address_, 0x6067, 0, 0, 4, false)); //Set position window to 0
    enqueue_command(CanWriteSDO(address_, 0x60f2, 0, 1, 2, false)); //Set relative moves to be relative to current position
    enqueue_command(CanWriteSDO(address_, 0x2089, 0, 50, 2, false)); // Set .5s delay before going to standby
    enqueue_command(CanWriteSDO(address_, 0x605A, 0, 5, 2, false)); // profile ramp with switch on disabled.  Quick stop ramp was having issues
    enqueue_command(CanWriteSDO(address_, TCO_QUICKSTOPDECELERATION, 0, get_pulses(T1241_EMCY_DECEL), 4, false));
    enqueue_command(CanWriteSDO(address_, TCO_PROFILEACCELERATION, 0, get_pulses(T1241_DEFAULT_ACCEL), 4, false));
    enqueue_command(CanWriteSDO(address_, TCO_PROFILEDECELERATION, 0, get_pulses(T1241_DEFAULT_ACCEL), 4, false));
}
 **/
