#include "main.h"
#include "vkrlogo_resize.c" 
#include "liblvgl/lvgl.h" 
#include "lemlib/api.hpp" 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#pragma region lvgl

lv_obj_t *myButton;
lv_obj_t *myButtonLabel;
lv_obj_t *roller1;
lv_obj_t *logo;

lv_style_t selectBtnREL; // relesed style
lv_style_t selectBtnPR;	 // pressed style

lv_style_t list;
lv_style_t rollerSelected;

lv_style_t bg;

int autonNumber = -1;

static lv_res_t btn_click_action(lv_obj_t *btn)
{
	uint8_t id = lv_obj_get_free_num(btn); // id usefull when there are multiple buttons

	if (id == 0)
	{
		autonNumber = lv_roller_get_selected(roller1);
	}

	return LV_RES_OK;
}

#pragma endregion

bool l1;
bool r1;
bool r2;

#pragma region lemlib

// imu
pros::Imu imu(10);

pros::MotorGroup left_motors({-20,-17,18},pros::MotorGearset::blue); 
pros::MotorGroup right_motors({19,16,-15},pros::MotorGearset::blue); 

lemlib::Drivetrain drivetrain(&left_motors, // left motor group
                              &right_motors, // right motor group
                              11.875, // 11.875 inch track width
                              lemlib::Omniwheel::NEW_275, // using new 2.75" omnis
                              450, // drivetrain rpm is 360
                              2 // horizontal drift is 2 (for now)
);

lemlib::OdomSensors sensors(nullptr, // vertical tracking wheel 1, set to null
                            nullptr, // vertical tracking wheel 2, set to nullptr as we are using IMEs
                            nullptr, // horizontal tracking wheel 1
                            nullptr, // horizontal tracking wheel 2, set to nullptr as we don't have a second one
                            nullptr // inertial sensor
);

// lateral PID controller
lemlib::ControllerSettings lateral_controller(10, // proportional gain (kP)
                                              0, // integral gain (kI)
                                              3, // derivative gain (kD)
                                              3, // anti windup
                                              1, // small error range, in inches
                                              100, // small error range timeout, in milliseconds
                                              3, // large error range, in inches
                                              500, // large error range timeout, in milliseconds
                                              20 // maximum acceleration (slew)
);

// angular PID controller
lemlib::ControllerSettings angular_controller(2, // proportional gain (kP)
                                              0, // integral gain (kI)
                                              10, // derivative gain (kD)
                                              3, // anti windup
                                              1, // small error range, in degrees
                                              100, // small error range timeout, in milliseconds
                                              3, // large error range, in degrees
                                              500, // large error range timeout, in milliseconds
                                              0 // maximum acceleration (slew)
);

// create the chassis
lemlib::Chassis llchassis(drivetrain, // drivetrain settings
                        lateral_controller, // lateral PID settings
                        angular_controller, // angular PID settings
                        sensors // odometry sensors
);

#pragma endregion

void initialize()
{
#pragma region lvgl
#pragma region styles
	// Screen BG
	lv_style_copy(&bg, &lv_style_plain);
	bg.body.main_color = LV_COLOR_BLACK;
	bg.body.grad_color = LV_COLOR_BLACK;

	// Roller Body
	lv_style_copy(&list, &lv_style_plain);
	list.body.main_color = LV_COLOR_BLACK;
	list.body.grad_color = LV_COLOR_BLACK;
	list.body.shadow.width = 10;
	list.text.color = LV_COLOR_RED;

	// Roller Selected Box
	lv_style_copy(&rollerSelected, &lv_style_plain);
	rollerSelected.body.main_color = LV_COLOR_BLACK;
	rollerSelected.body.grad_color = LV_COLOR_BLACK;
	rollerSelected.body.shadow.width = 12;
	rollerSelected.body.shadow.color = LV_COLOR_WHITE;
	rollerSelected.text.color = LV_COLOR_RED;

	// Select Button
	lv_style_copy(&selectBtnREL, &lv_style_pretty);
	selectBtnREL.body.main_color = LV_COLOR_MAKE(150, 0, 0);
	selectBtnREL.body.grad_color = LV_COLOR_MAKE(150, 0, 0);
	selectBtnREL.body.radius = 0;
	selectBtnREL.body.shadow.width = 12;
	selectBtnREL.body.shadow.color = LV_COLOR_WHITE;
	selectBtnREL.text.color = LV_COLOR_MAKE(255, 255, 255);

	// Select Button Pressed
	lv_style_copy(&selectBtnPR, &lv_style_pretty);
	selectBtnPR.body.main_color = LV_COLOR_MAKE(255, 0, 0);
	selectBtnPR.body.grad_color = LV_COLOR_MAKE(0, 0, 0);
	selectBtnPR.body.radius = 0;
	selectBtnPR.body.shadow.width = 12;
	selectBtnPR.body.shadow.color = LV_COLOR_WHITE;
	selectBtnPR.text.color = LV_COLOR_MAKE(255, 255, 255);

#pragma endregion
#pragma region initilization
	// Screen bg
	lv_obj_set_style(lv_scr_act(), &bg);

	// Logo
	logo = lv_img_create(lv_scr_act(), NULL);
	lv_img_set_src(logo, &vkrlogo_resize);
	lv_obj_align(logo, NULL, LV_ALIGN_IN_RIGHT_MID, -50, -20);

	// Roller
	roller1 = lv_roller_create(lv_scr_act(), NULL);
	lv_roller_set_options(roller1,
						  "Close Side\n"
						  "Disrupt\n"
						  "Push Forward\n"
						  "5 ball (don't use)");

	lv_roller_set_visible_row_count(roller1, 6);
	lv_obj_align(roller1, NULL, LV_ALIGN_IN_LEFT_MID, 0, 0);
	lv_roller_set_style(roller1, LV_ROLLER_STYLE_BG, &list);
	lv_roller_set_style(roller1, LV_ROLLER_STYLE_SEL, &rollerSelected);
	// lv_obj_add_event_cb(roller1, event_handler, LV_EVENT_ALL, NULL);

	// Button
	myButton = lv_btn_create(lv_scr_act(), NULL);						// create button, lv_scr_act() is deafult screen object
	lv_obj_set_free_num(myButton, 0);									// set button is to 0
	lv_btn_set_action(myButton, LV_BTN_ACTION_CLICK, btn_click_action); // set function to be called on button click
	lv_btn_set_style(myButton, LV_BTN_STYLE_REL, &selectBtnREL);		// set the relesed style
	lv_btn_set_style(myButton, LV_BTN_STYLE_PR, &selectBtnPR);			// set the pressed style
	lv_obj_set_size(myButton, 250, 40);									// set the button size
	lv_obj_align(myButton, NULL, LV_ALIGN_IN_BOTTOM_RIGHT, -20, -10);	// set the position to top mid

	// Button Label
	myButtonLabel = lv_label_create(myButton, NULL); // create label and puts it inside of the button
	lv_label_set_text(myButtonLabel, "SELECT");		 // sets label text
#pragma endregion
#pragma endregion
#pragma region lemlib


#pragma endregion
#pragma region OKAPI
	double gearRatio = 36.0/48.0;

	odomChassis = ChassisControllerBuilder()
				  .withMotors(
					  {-20,-17,18}, // left Motors
					  {19,16,-15} // right Motors
					  )
				  .withDimensions({okapi::AbstractMotor::gearset::blue, gearRatio}, {{4_in, 10.85_in * 3.333333}, imev5BlueTPR / gearRatio})
				  .withOdometry()
				  .buildOdometry();
				  
l1 = false;
r1 = false;
r2 = false;

#pragma endregion
}

void disabled() {}

void competition_initialize() {}

void autonomous() 
{
	//llchassis.setPose(0,0,0);
	odomChassis->getOdometry()->setState({0_in, 0_in, 0_deg}); // zero the position of the robot
	odomChassis->setMaxVelocity(400);

	switch (autonNumber)
	{
	case 0:
		break;
	case 1:
		break;
	case 2: 
		break;
	case 3:
		break;
	case 4:
		break;
	default:
		break;
	}
}

inline void updateDrive()
{
	float leftC = controller.getAnalog(ControllerAnalog::leftY);
	float rightC = controller.getAnalog(ControllerAnalog::rightY);
	chassis.tank(leftC, rightC);
	/*
	odomChassis->getModel()->tank(
		leftC,
		rightC
	);
	*/
}
bool activated = false;
inline void updateClamp()
{
	bool newL1 = controller.getDigital(ControllerDigital::L1); //clamp
	if(newL1 == true)
	{
		if(activated == false)
		{
			l1 = !l1;
			clamp.set_value(l1);
			activated = true;
		}
	}
	else
	{
		activated = false;
	}
}
bool l2 = false;
bool activatedDoinker = false;
inline void updateDoinker()
{
	bool newL2 = controller.getDigital(ControllerDigital::L2); //clamp
	if(newL2 == true)
	{
		if(activatedDoinker == false)
		{
			l2 = !l2;
			doinker.set_value(l2);
			activatedDoinker = true;
		}
	}
	else
	{
		activatedDoinker = false;
	}
}
inline void updateIntake()
{
	bool newR1 = controller.getDigital(ControllerDigital::R1); //intake
	bool newR2 = controller.getDigital(ControllerDigital::R2); //outtake
	
	intake.moveVoltage((
		(12000 * newR1) + (-12000 * newR2)));
		
	stage2.moveVoltage((
		1 * ((12000 * newR1) + (-12000 * newR2))));
	r1 = newR1;
	r2 = newR2;
}
void opcontrol() {
	odomChassis->setMaxVelocity(600);

	while (true)
	{
		updateDrive();
		updateClamp();
		updateIntake();
		updateDoinker();
		pros::delay(1);
	}
}