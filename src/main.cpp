#include "main.h"
#include "vkrlogo_resize.c"
#include "liblvgl\lvgl.h"
#include "lemlib\api.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int autonNumber = -1;

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

pros::MotorGroup left_motors({-20, -17, 18});
pros::MotorGroup right_motors({19, 16, -15});

lemlib::Drivetrain drivetrain(&left_motors,				  // left motor group
							  &right_motors,			  // right motor group
							  11.875,					  // 11.875 inch track width
							  lemlib::Omniwheel::NEW_275, // using new 2.75" omnis
							  450,						  // drivetrain rpm is 450
							  2							  // horizontal drift is 2 (for now)
);

lemlib::OdomSensors sensors(nullptr, // vertical tracking wheel 1, set to null
							nullptr, // vertical tracking wheel 2, set to nullptr as we are using IMEs
							nullptr, // horizontal tracking wheel 1
							nullptr, // horizontal tracking wheel 2, set to nullptr as we don't have a second one
							&imu	 // inertial sensor
);

lemlib::ControllerSettings lateral_controller(12,  // proportional gain (kP) 12
											  0,   // integral gain (kI)
											  2.5, // derivative gain (kD) 3
											  0,   // anti windup
											  0,   // small error range, in inches
											  100, // small error range timeout, in milliseconds
											  0,   // large error range, in inches
											  500, // large error range timeout, in milliseconds
											  0	   // maximum acceleration (slew)
);
// 6.66 - 7.52 = 0.86
// ki = 2 * (60 / 0.86);
// kd = 0.125  * 60 * 	0.86
//  angular PID controller
// 100
lemlib::ControllerSettings angular_controller(1.25, // proportional gain (kP) 9
											  0,	// integral gain (kI)
											  4,	// derivative gain (kD)
											  0,	// anti windup
											  0,	// small error range, in inches
											  100,	// small error range timeout, in milliseconds
											  0,	// large error range, in inches
											  500,	// large error range timeout, in milliseconds
											  0		// maximum acceleration (slew)
);

// create the chassis
lemlib::Chassis llchassis(drivetrain,		  // drivetrain settings
						  lateral_controller, // lateral PID settings
						  angular_controller, // angular PID settings
						  sensors			  // odometry sensors
);

#pragma endregion

uint32_t ejectBlueTime = 0;
uint32_t waitReactivateTime = 0;
bool blue = false;
bool ejectBlue = true;
int colorFloor = 0;
int colorCeiling = 0;


#define STOP_VOLTAGE -12000
#define BLUE_TIME 250
#define RED_TIME 300
#define RED_CEILING 35
#define RED_FLOOR 0
#define BLUE_CEILING 220
#define BLUE_FLOOR 140
int delayTime = 0;

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
						  "Blue Solo WP\n"
						  "Red Solo WP\n"
						  "Slot 3\n"
						  "Slot 4");

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
	double gearRatio = 36.0 / 48.0;

	odomChassis = ChassisControllerBuilder()
					  .withMotors(
						  {-20, -17, 18}, // left Motors
						  {19, 16, -15}	  // right Motors
						  )
					  .withDimensions({okapi::AbstractMotor::gearset::blue, gearRatio}, {{2.75_in, 11.875_in}, 450})
					  .withOdometry()
					  .buildOdometry();

	l1 = false;
	r1 = false;
	r2 = false;

#pragma endregion

	ladyBrown.setBrakeMode(okapi::AbstractMotor::brakeMode::hold);
	LBRotation.reset_position();

	llchassis.calibrate();

	opticalSensor.setLedPWM(100);

	if (ejectBlue)
			{
				colorFloor = BLUE_FLOOR;
				colorCeiling = BLUE_CEILING;
				delayTime = BLUE_TIME;
			}
			else
			{
				colorFloor = RED_FLOOR;
				colorCeiling = RED_CEILING;
				delayTime = RED_TIME;
			}
}

void disabled() {}

void competition_initialize() {}


ASSET(redpos_txt);

void autonomous()
{
	odomChassis->getOdometry()->setState({0_in, 0_in, 0_deg}); // zero the position of the robot
	odomChassis->setMaxVelocity(400);
	autonNumber = 2;
	switch (autonNumber)
	{
	/////////////////////////////////////////////////////////////////////
	// BLUE SIDE WP///////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////
	case 0:
	{

		llchassis.setPose(0, 0, 180);

		// move to get first mogo
		llchassis.moveToPoint(0, 36, 1200, {.forwards = false, .maxSpeed = 70.0f});
		pros::delay(1200);

		// get mogo
		clamp.set_value(HIGH);
		pros::delay(700);

		llchassis.turnToHeading(90, 700, {.maxSpeed = 100});
		pros::delay(700);

		stage2.moveVoltage((
			0.9 * ((12000))));
		// Get first two stack
		llchassis.moveToPoint(20, 36, 2000, {.maxSpeed = 100.0f});
		pros::delay(2000);

		// turn to 4 stack
		llchassis.turnToHeading(0, 800, {.maxSpeed = 100});
		pros::delay(800);

		// get 4 stack
		clamp.set_value(LOW);
		llchassis.moveToPoint(20, 46, 850, {.maxSpeed = 100.0f});
		pros::delay(850);
		stage2.moveVoltage(0);

		// return to origin
		llchassis.moveToPoint(0, 14, 2000, {.forwards = false, .maxSpeed = 100.0f});
		pros::delay(2000);

		// move the allicance stake blockage
		llchassis.moveToPoint(-27, 14, 2000, {.forwards = false, .maxSpeed = 100.0f});
		pros::delay(2000);

		// allign to score alliance stake
		llchassis.turnToHeading(0, 1000, {.maxSpeed = 80});
		pros::delay(1000);

		llchassis.setPose(0, 0, 180);

		// score alliance stake
		llchassis.moveToPoint(1, 10, 1000, {.forwards = false, .maxSpeed = 100.0f});
		pros::delay(1000);

		stage2.moveVoltage((
			0.9 * ((12000))));
		pros::delay(1000);
		stage2.moveVoltage(0);
		// touch bar
		llchassis.moveToPoint(0, -24, 800, {.maxSpeed = 80.0f});
		pros::delay(800);
		doinker.set_value(HIGH);

		break;
	}

	/////////////////////////////////////////////////////////////////////
	// RED SIDE WP///////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////
	case 1:
	{
		llchassis.setPose(0, 0, 180);

		// move to get first mogo
		llchassis.moveToPoint(0, 36, 1200, {.forwards = false, .maxSpeed = 70.0f});
		pros::delay(1200);

		// get mogo
		clamp.set_value(HIGH);
		pros::delay(700);

		llchassis.turnToHeading(270, 700, {.maxSpeed = 100});
		pros::delay(700);

		stage2.moveVoltage((
			0.9 * ((12000))));
		// Get first two stack
		llchassis.moveToPoint(-20, 36, 2000, {.maxSpeed = 100.0f});
		pros::delay(2000);

		// turn to 4 stack
		llchassis.turnToHeading(0, 800, {.maxSpeed = 100});
		pros::delay(800);

		// get 4 stack
		clamp.set_value(LOW);
		llchassis.moveToPoint(-20, 46, 850, {.maxSpeed = 100.0f});
		pros::delay(850);
		stage2.moveVoltage(0);

		// return to origin
		llchassis.moveToPoint(0, 0, 2000, {.forwards = false, .maxSpeed = 100.0f});
		pros::delay(2000);

		// move the allicance stake blockage
		llchassis.moveToPoint(22, 6, 2000, {.forwards = false, .maxSpeed = 100.0f});
		pros::delay(2000);

		// allign to score alliance stake
		llchassis.turnToHeading(0, 1000, {.maxSpeed = 80});
		pros::delay(1000);

		llchassis.setPose(0, 0, 180);

		// score alliance stake
		llchassis.moveToPoint(0, 10, 1000, {.forwards = false, .maxSpeed = 100.0f});
		pros::delay(1000);

		stage2.moveVoltage((
			0.9 * ((12000))));
		pros::delay(1000);
		stage2.moveVoltage(0);
		break;
	}
	
	/////////////////////////////////////////////////////////////////////
	// RED POS SIDE///////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////
	case 2:
	{
		llchassis.setPose(0,0,0);
	
	
    llchassis.moveToPoint(0, -23, 1000,{.forwards = false,.maxSpeed = 80});
	pros::delay(1000);
	llchassis.moveToPoint(-9,-44,1000,{.forwards = false,.maxSpeed = 80});
	pros::delay(1400);
			clamp.set_value(HIGH);
			pros::delay(1000);
			stage2.moveVoltage((
			0.9 * ((12000))));
			llchassis.turnToHeading(-15,1000);
			pros::delay(1000);
			llchassis.moveToPoint(-12,-33,2000,{.maxSpeed = 80}); //orig -34
			pros::delay(1000);
			stage2.moveVoltage(0);
			llchassis.turnToHeading(270,1000,{.maxSpeed = 80});
			pros::delay(1000);
			
			stage2.moveVoltage((0));
						clamp.set_value(LOW);
						pros::delay(1000);
						llchassis.setPose(0,0,0);

						llchassis.turnToHeading(170,2000,{.maxSpeed = 100});
						pros::delay(1000);

						llchassis.moveToPoint(0,22,2000,{.forwards = false,.maxSpeed = 100});
						pros::delay(700);

						clamp.set_value(HIGH);
						pros::delay(1000);

			stage2.moveVoltage((
			0.9 * ((12000))));
pros::delay(2000);





		break;
	}
	// tune PID
	case 6:
	{
		llchassis.setPose(0, 0, 0);

		llchassis.moveToPoint(0, 36, 4000, {.maxSpeed = 100});
		// llchassis.turnToHeading(90,4000,{.maxSpeed = 100});
		pros::delay(4000);
		break;
	}
	default:
		break;
	}
}

inline void updateDrive()
{
	float leftC = controller.getAnalog(ControllerAnalog::leftY);
	float rightC = controller.getAnalog(ControllerAnalog::rightY);
	// chassis.tank(leftC, rightC);

	odomChassis->getModel()->tank(
		leftC,
		rightC);
}
bool activated = false;
inline void updateClamp()
{
	bool newL1 = controller.getDigital(ControllerDigital::L1); // clamp
	if (newL1 == true)
	{
		if (activated == false)
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
	bool newL2 = controller.getDigital(ControllerDigital::L2); // clamp
	if (newL2 == true)
	{
		if (activatedDoinker == false)
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
	bool newR1 = controller.getDigital(ControllerDigital::R1); // intake
	bool newR2 = controller.getDigital(ControllerDigital::R2); // outtake

	intake.moveVoltage((
		(12000 * newR1) + (-12000 * newR2)));

	stage2.moveVoltage((
		1 * ((12000 * newR1) + (-12000 * newR2))));
	r1 = newR1;
	r2 = newR2;
}
inline void updateForceOpen()
{
	if (controller.getDigital(ControllerDigital::Y))
	{
		clamp.set_value(LOW);
	}
}
bool activatedLadyBrown = false;
bool activatedVertLB = false;
bool activatedRestLB = false;
bool horizontal = false;
bool setZero = false;
void updateLadyBrown() // flat
{
	// down d pad makes it go to resting
	//  a makes it go to horizontal
	//  x makes it go to vert
	if (controller.getDigital(ControllerDigital::A))
	{
		activatedLadyBrown = true;
	}
	if (controller.getDigital(ControllerDigital::left) || controller.getDigital(ControllerDigital::right))
	{
		activatedLadyBrown = false;
	}
	float angle = (float)LBRotation.get_angle() / 100;
	float max = 230;
	if (activatedLadyBrown)
	{
		/*
		// ladyBrown.
		// 360 ticks
		//  36 : 1
		//  need 25 degrees or 0.07 * 360
		//  2.52 : 0.07
		// 2.52 * 360 = 907.2
		//ladyBrown.moveAbsolute(-907 / 2 + -907 / 13 + -907 / 100, 100);

*/
		// 345.5

		float target = 350;
		int voltageToMove = -500;
		int voltageToMoveNeg = -3000;

		if (angle > target || angle < 10)
		{
			ladyBrown.moveVoltage(voltageToMoveNeg);
		}
		else if (angle < target)
		{
			ladyBrown.moveVoltage(voltageToMove);
		}
	}
	else if (angle > max || angle < 10 || controller.getDigital(ControllerDigital::left))
	{
		setZero = false;
		ladyBrown.moveVoltage((12000 * controller.getDigital(ControllerDigital::left) + controller.getDigital(ControllerDigital::right) * -12000));
	}
	else
	{
		if (angle < max)
		{
			ladyBrown.moveVoltage(8000);
		}
		else
		{
			if (!setZero)
			{
				ladyBrown.moveVoltage(0);
				setZero = true;
			}
		}
	}
}
bool colorSortOn = true;
bool colorSortToggle = false;
bool colorSortSwitchToggle = false;



void UpdateColorSort()
{
	
	if (controller.getDigital(ControllerDigital::up))
	{
		if (!colorSortToggle)
		{
			colorSortOn = !colorSortOn;
			colorSortToggle = true;
		}
	}
	else
	{
		colorSortToggle = false;
	}

	if (colorSortOn)
	{
		if (!colorSortSwitchToggle && controller.getDigital(ControllerDigital::down))
		{
			ejectBlue = !ejectBlue;
			colorSortSwitchToggle = true;

			if (ejectBlue)
			{
				colorFloor = BLUE_FLOOR;
				colorCeiling = BLUE_CEILING;
				delayTime = BLUE_TIME;
			}
			else
			{
				colorFloor = RED_FLOOR;
				colorCeiling = RED_CEILING;
				delayTime = RED_TIME;
			}
		}
		else
		{
			colorSortSwitchToggle = false;
		}

		float hue = opticalSensor.getHue();
		float millis = pros::millis();

		if (millis < waitReactivateTime)
		{
			stage2.moveVoltage((STOP_VOLTAGE));
		}
		else
		{
			if (hue > colorFloor && hue < colorCeiling && !blue)
			{
				blue = true;
				ejectBlueTime = millis;
			}
			if (blue && millis >= ejectBlueTime + BLUE_TIME)
			{
				stage2.moveVoltage((STOP_VOLTAGE));
				blue = false;
				waitReactivateTime = millis + 235;
			}
		}
	}
}
void opcontrol()
{
	odomChassis->setMaxVelocity(600);

	while (true)
	{
		updateDrive();
		updateClamp();
		updateIntake();
		updateDoinker();
		updateForceOpen();
		updateLadyBrown();

		// THIS IS ALLWAYS LAST
		UpdateColorSort();

		pros::delay(1);
	}
}