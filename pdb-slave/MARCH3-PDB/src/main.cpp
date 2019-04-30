#include <mbed.h>
#include <LPC1768_pindefs_M3.h>
#include "StateMachine.h"

Serial pc(USBTX, USBRX, 9600); // Serial communication for debugging

// On/off-related inputs/outputs
DigitalOut buttonLed(LPC_BUTTON_LED, true); // Behaviour is logically inverted
DigitalOut mbedLed1(LPC_LED1, false); // Shows same as button led
DigitalIn button(LPC_BUTTON_PRESSED, PullDown);
DigitalOut keepPDBOn(LPC_KEEP_PDB_ON, PullDown);
DigitalOut mbedLed4(LPC_LED4, false); // Shows if in Shutdown state

// Low voltage related inputs/outputs
DigitalOut LVOn(LPC_LVON, PullDown);
DigitalIn LVOkay(LPC_LVOKAY, PullDown);

// Master communication related inputs/outputs
DigitalOut mbedLed2(LPC_LED2, false); // Shows if in MasterOk state
DigitalIn masterOk(p14, PullDown); // !!! Temporary to replace EtherCAT
DigitalOut masterShutdown(p15, PullDown); // !!! Temporary to replace EtherCAT
DigitalIn masterShutdownAllowed(p16, PullDown); // !!! Temporary to replace EtherCAT

// High voltage related inputs/outputs
DigitalOut mbedLed3(LPC_LED3, false); // Shows if any HV is on
// Todo: I2C bus to HV

StateMachine stateMachine; // State machine instance
Timer printTimer; // Timer to print debug statements only once per second

int main(){
    pc.printf("MBED gets power now!\r\n");

    // Set initial outputs
    buttonLed = true; // Behaviour is logically inverted
    mbedLed1 = false;
    mbedLed2 = false;
    mbedLed3 = false;
    mbedLed4 = false;
    keepPDBOn = false;
    LVOn = false;
    masterShutdown = false;

    printTimer.start();

    while(1){
        // Update EtherCAT variables
        // Todo: Implement EtherCAT

        // Get inputs
        bool buttonstate = button.read();
        bool LVOkayState = LVOkay.read();
        bool masterOkState = masterOk.read();
        bool masterShutdownAllowedState = masterShutdownAllowed.read();

        // Update system state
        stateMachine.updateState(buttonstate, masterOkState, masterShutdownAllowedState); // Todo: add HVon inputs

        // Debug prints
        if(printTimer.read_ms() > 1000){
            pc.printf("\r\nState: %s", stateMachine.getState().c_str());
            // pc.printf("\tKeepPDBOn: %d", stateMachine.getKeepPDBOn());
            // if((!LVOkayState) && (stateMachine.getState() == "LVOn_s")){
            //     pc.printf("LV not okay");
            // }
            // pc.printf("\tPin 16: %d", masterShutdownAllowedState);
            printTimer.reset();
        }

        // Control outputs
        // Set LEDs and digitalOuts
        buttonLed = !stateMachine.getOnOffButtonLedState(); // Behaviour is logically inverted
        mbedLed1 = stateMachine.getOnOffButtonLedState(); // Later change to getKeepPDBOn()?
        mbedLed2 = (stateMachine.getState() == "MasterOk_s"); // LED on if in MasterOk state
        mbedLed3 = (stateMachine.getHVOn() != 0); // LED on if any HV is on
        mbedLed4 = (stateMachine.getState() == "Shutdown_s"); // LED on if in Shutdown state
        keepPDBOn = stateMachine.getKeepPDBOn();
        LVOn = stateMachine.getLVOn();
        masterShutdown = stateMachine.getMasterShutdown();
        // Control HV
        // Todo: Do I2C communication to turn HV on/off based on getHVon() result;
        
        // Set logging stuff in EtherCAT buffers
        // Todo: Implement EtherCAT
        // Todo: Set error code based on xxOkay signals

    }
    
}