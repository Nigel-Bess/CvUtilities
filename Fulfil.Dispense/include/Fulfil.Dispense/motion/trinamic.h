/*
 * trinamic.h
 *  Created on: Jul 30, 2018, Mike. Modified Oct 2021, Steve.
 */

#ifndef FULFIL_DISPENSE_Trinamic_H_
#define FULFIL_DISPENSE_Trinamic_H_

#include <stdio.h>
#include <queue>
#include <memory>
#include "registers.h"
#include "serial.h"
#include "motion_timer.h"

#define TMCL_ROR 1
#define TMCL_ROL 2
#define TMCL_MST 3
#define TMCL_MVP 4
#define TMCL_SAP 5
#define TMCL_GAP 6
#define TMCL_STAP 7
#define TMCL_RSAP 8
#define TMCL_SGP 9
#define TMCL_GGP 10
#define TMCL_STGP 11
#define TMCL_RSGP 12
#define TMCL_RFS 13
#define TMCL_SIO 14
#define TMCL_GIO 15
#define TMCL_SCO 30
#define TMCL_GCO 31
#define TMCL_CCO 32

//Opcodes of TMCL control functions (to be used to run or abort a TMCL program in the module)
#define TMCL_APPL_STOP 128
#define TMCL_APPL_RUN 129
#define TMCL_APPL_RESET 131

//Options for MVP commands
#define MVP_ABS 0
#define MVP_REL 1
#define MVP_COORD 2

//Options for RFS command
#define RFS_START 0
#define RFS_STOP 1
#define RFS_STATUS 2

#define STATUS_SUCCESS 100
#define STATUS_COMMAND_SAVED 101
#define STATUS_BAD_CRC 1
#define STATUS_INVALID 2
#define STATUS_WRONG_TYPE 3
#define STATUS_INVALID_VALUE 4
#define STATUS_MEM_LOCK 5
#define STATUS_COMMAND_NA 6


namespace fulfil
{
namespace dispense {
namespace motion
{
/**
 * The purpose of this class is to provide a discrete
 * grid that a point cloud can be adapted to to enable
 * working in discrete coordinates instead of continuous
 * ones.
 */
class Trinamic
{
  private:

    int driverConnection = -1; // the descriptor of the serial connection to driver
    int timeout = 0; //for tracking timeout instances across serial writing / reading

    uint32_t baud;
    const char* com_port;

    StepperRegisters_t StepperReg;

    typedef struct
    {
      uint8_t Address;
      uint8_t Command;
      uint8_t RequestId;
      uint8_t MotorBank;
      uint32_t Value;
      uint8_t Crc;
    }TrinamicTxPacket_t;

    typedef struct
    {
      uint8_t ReplyAddress;
      uint8_t ModuleAddress;
      uint8_t Status;
      uint8_t Command;
      uint32_t Value;
      uint8_t Crc;
    }TrinamicRxPacket_t;

    typedef struct
    {
      union{
        uint8_t Payload[9];
        TrinamicTxPacket_t Tx;
        TrinamicRxPacket_t Rx;
      };
      static uint8_t Size() { return 9; }
    }Trinamic_t;

    Trinamic::Trinamic_t TrinamicRx;
    Trinamic::Trinamic_t TrinamicTx;

    void PackLeValue(Trinamic_t* packet, uint32_t data);
    void UnpackLeValue(Trinamic_t* packet);
    void CalcCrc(Trinamic_t* packet);
    bool CheckRxPacket(Trinamic_t* packet);
    bool ReadReply();
    //Returns true if successfully opened COM port connection to trinamic board
    bool ConnectMotor();

  public:

    Trinamic(uint32_t baud, const char* com_port);
    bool Motor_Homed;
    int GetAnalogSensorInput();
    int GetHomeSwitchState();
    float GetLoadState();
    void PrintTrinamicPacket(Trinamic_t* packet);
    void UpdateMovingRegisters();
    void StopMotor();
    bool TryConnectMotor();
    bool SendMotorCommand(uint8_t Address, uint8_t Command, uint8_t Type, uint8_t Motor, uint32_t Value);
    void update_and_print_motor_status();
    int get_current_position();
    int check_position_reached(); // -1 = error, 0 = no, 1 = yes

};
} // namespace motion
} // namespace dispense
} // namespace fulfil



#endif //FULFIL_DISPENSE_Trinamic_H_
