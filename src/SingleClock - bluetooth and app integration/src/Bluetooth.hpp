#include <BluetoothSerial.h>
#include <StepperMotorMovement.hpp>

class Bluetooth
{
private:
    BluetoothSerial ESP_bt_;
    String btName_ = "Clock" 
public : 
    Bluetooth();
    ~Bluetooth();
    void InitBluetooth();
    void GetDataFromClient();
    void InitClock(StepperMotorMovement);

    String incomingData_;
};
