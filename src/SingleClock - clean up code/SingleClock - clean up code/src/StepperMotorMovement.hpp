#include <HelperFunctions.hpp>
#include <Time.hpp>
#include <AccelStepper.h>
#include <MultiStepper.h>

class StepperMotorMovement
{
private:
    //Stepper motor A
    const int STEP_A_ = 23;           // Driver p28 for f(scx)A
    const int DIR_A_ = 19;            // Driver p27 for CW/CCW
    //Stepper motor D
    const int STEP_D_ = 18;           // Driver p17 for f(scx)A
    const int DIR_D_ = 5;             // Driver p16 for CW/CCW

    const int TOTALSTEPS_ = 360 * 12; // of 360 available

    int motorsFinished_ = 0;          // Used to break motors while loop
    int move_finished_hour_ = 1;      // Used to check if hour pointer is completed
    int move_finished_min_ = 1;       // Used to check if minute pointer is completed
    long initial_homing_ = -1;        // Used to Home Stepper at startup
    int minuteDegrees_;
    int hourDegrees_;

    Time time_;
    HelperFunction helper_;
    AccelStepper motorHours_;
    AccelStepper motorMinutes_;
public:
    StepperMotorMovement(Time, HelperFunction);
    ~StepperMotorMovement();
    void MoveMinuteMotorInDeg(long minuteMotorPos);
    void MoveHourMotorInDeg(long hourMotorPos);
    void MoveBothMotorsInDeg(long minuteMotorPos, long hourMotorPos);
    void Homing();
    long ConvMinToRotation(int);
    long ConvHourToRotation(int);
    void Step(String);
    
    long minuteStepInDegrees_ = 0;
    long hourStepInDegrees_ = 0;
    const int RESET_ = 15;            // pin for RESET
};
