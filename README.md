# motorcontrol_algorithm_qt_hmi_v0.0
This repo will be used as the codebase to push changes to the Qt project and Embedded C project for basic motor control using STM32 controller(STM32L433). The goal is to be able to move the motor and read the feedback signals such as current of phases , supply voltages , and backEmf calculations. Ofcourse , this will take time , but we will do it !!
I choose this controller because it has advanced timer capability which can give me a reliable motor control.
# here are the step by step goals we should try to achieve
1. create a basic running software for pwm generations on the gpio pins which support pwm : [ placeholder for pins which will be used ]
2. shall be able to control the pwm frequency and duty cycle of the signal : [ placeholder for upper lower limit of frequency ]
3. configure adc and other peripherals required to be able to read the feedback signals : [ feedback signals are current and voltages , motor driver IC i already have , compatible with stm32]
4. verify that code is running well and able to read the feedback data.
5. now write a basic algorithm to run everything in a closed loop control system with the feedback.

# problems we might face
1. the ic which i have is not supported for big motors , hence we will do it with whatever we have.
2. we need a supply and a setup , which i will make sure to arrange here with me.
3. for remote testing for collaborators , we need some solutions : [ this is only true when the basic things are working ]
4. rest problems ,i leave to time , we will figure however we move

# userinterface development using c++
1. the user interface shall have a graph which can showcase current values feedback from the motor
2. shall also show battery voltages and backemf generated
3. shall work via UART to communicate with the device : we will use FTDI in the inital phases.
4. raw values will be transmitted to the UI , UI should convert them to physical values and handle the processing of data


   _#### Note : _this does not need to be perfect , but it needs to be working
   _ #### Note : _this will take time , please do not expect this to be an easy feat
   _ #### Note : _this will be recorded and documented for reaching to people and showcasing our talent

   If you are here , thanks , let us start with whatever time we have on our hands and i will try to do it with whatever bangalore traffic allows me :) 

   
