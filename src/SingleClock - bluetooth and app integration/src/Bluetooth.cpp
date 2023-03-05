#include <Bluetooth.hpp>

Bluetooth::Bluetooth()
{
}

Bluetooth::~Bluetooth()
{
}

void Bluetooth::InitBluetooth()
{
    ESP_bt_.begin(btName_, true)
}

void Bluetooth::GetDataFromClient()
{
    if (ESP_bt_.hasClient())
    {
        while (ESP_bt_.available())
        {
            incomingData_ = ESP_BT.readString();
            Serial.println("Received: " + incomingData_);
            if (incomingData_ == "stop") 
            {
                break;
            }
        }
    }
}

void Bluetooth::InitClock(StepperMotorMovement motor)
{
    
}