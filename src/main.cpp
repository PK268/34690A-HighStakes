#include "main.h"
#include "vkrlogo_resize.c"
#include "liblvgl/lvgl.h"

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

#pragma region OKAPI
	double gearRatio = 36.0/48.0;

	chassis = ChassisControllerBuilder()
				  .withMotors(
					  {-1,-2,3}, // left Motors
					  {12,13,-14} // right Motors
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
	chassis->getOdometry()->setState({0_in, 0_in, 0_deg}); // zero the position of the robot
	chassis->setMaxVelocity(400);

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
	chassis->getModel()->tank(
		leftC,
		rightC
	);
}

inline void updateClamp()
{
	bool newL1 = controller.getDigital(ControllerDigital::L1); //clamp
	if(newL1 != l1)
	{
		l1 = newL1;
		clamp.set_value(newL1);
	}
}
inline void updateIntake()
{
	bool newR1 = controller.getDigital(ControllerDigital::R1); //intake
	bool newR2 = controller.getDigital(ControllerDigital::R2); //outtake
	
	intake.moveVoltage((
		(-12000 * newR1) + (12000 * newR2)));
	r1 = newR1;
	r2 = newR2;
}
void opcontrol() {
	chassis->setMaxVelocity(600);

	while (true)
	{
		updateDrive();
		updateClamp();
		updateIntake();
		pros::delay(1);
	}
}