#include <utils/Log.h>
#include "sensor.h"
#include "sensor_drv_u.h"
#include "sensor_raw.h"

#define OTP_AUTO_LOAD_LSC_ov5648_DARLING

#define RG_TYPICAL_ov5648_DARLING    	0x017E
#define BG_TYPICAL_ov5648_DARLING		0x015E
#define R_TYPICAL_ov5648_DARLING		0x0000
#define G_TYPICAL_ov5648_DARLING		0x0000
#define B_TYPICAL_ov5648_DARLING		0x0000


static uint8_t light_rg=0;
static uint8_t light_bg=0;

// index: index of otp group. (1, 2, 3)
// return: 0, group index is empty
// 1, group index has invalid data
// 2, group index has valid data
LOCAL uint32_t ov5648_check_otp(int index)
{
	uint32_t flag = 0;
	uint32_t i = 0;
	uint32_t rg = 0;
	uint32_t bg = 0;

	if (index == 1)
	{
		// read otp --Bank 0
		Sensor_WriteReg(0x3d84, 0xc0);
		Sensor_WriteReg(0x3d85, 0x00);
		Sensor_WriteReg(0x3d86, 0x0f);
		Sensor_WriteReg(0x3d81, 0x01);
		usleep(5 * 1000);
		flag = Sensor_ReadReg(0x3d05);
		rg = Sensor_ReadReg(0x3d07);
		bg = Sensor_ReadReg(0x3d08);
	}
	else if (index == 2)
	{
		// read otp --Bank 0
		Sensor_WriteReg(0x3d84, 0xc0);
		Sensor_WriteReg(0x3d85, 0x00);
		Sensor_WriteReg(0x3d86, 0x0f);
		Sensor_WriteReg(0x3d81, 0x01);
		usleep(5 * 1000);
		flag = Sensor_ReadReg(0x3d0e);
		// read otp --Bank 1
		Sensor_WriteReg(0x3d84, 0xc0);
		Sensor_WriteReg(0x3d85, 0x10);
		Sensor_WriteReg(0x3d86, 0x1f);
		Sensor_WriteReg(0x3d81, 0x01);
		usleep(5 * 1000);
		rg = Sensor_ReadReg(0x3d00);
		bg = Sensor_ReadReg(0x3d01);
	}
	else if (index == 3)
	{
		// read otp --Bank 1
		Sensor_WriteReg(0x3d84, 0xc0);
		Sensor_WriteReg(0x3d85, 0x10);
		Sensor_WriteReg(0x3d86, 0x1f);
		Sensor_WriteReg(0x3d81, 0x01);
		usleep(5 * 1000);
		flag = Sensor_ReadReg(0x3d07);
		rg = Sensor_ReadReg(0x3d09);
		bg = Sensor_ReadReg(0x3d0a);
	}
	SENSOR_PRINT("ov5648 check_otp: flag = 0x%d----index = %d---\n", flag, index);
	flag = flag & 0x80;
	// clear otp buffer
	for (i=0; i<16; i++) {
		Sensor_WriteReg(0x3d00 + i, 0x00);
	}
	SENSOR_PRINT("ov5648 check_otp: flag = 0x%d  rg = 0x%x, bg = 0x%x,-------\n", flag, rg, bg);
	if (flag) {
		return 1;
	}
	else
	{
		if (rg == 0 && bg == 0)
		{
			return 0;
		}
		else
		{
			return 2;
		}
	}
}

// index: index of otp group. (1, 2, 3)
// return: 0,
static int ov5648_read_otp(int index, struct otp_info_t *otp_ptr)
{
	int i = 0;
	int temp = 0;
	// read otp into buffer
	if (index == 1)
	{
		// read otp --Bank 0
		Sensor_WriteReg(0x3d84, 0xc0);
		Sensor_WriteReg(0x3d85, 0x00);
		Sensor_WriteReg(0x3d86, 0x0f);
		Sensor_WriteReg(0x3d81, 0x01);
		usleep(5 * 1000);
		otp_ptr->module_id = (Sensor_ReadReg(0x3d05) & 0x7f);
		otp_ptr->lens_id = Sensor_ReadReg(0x3d06);
		temp = Sensor_ReadReg(0x3d0b);
		otp_ptr->rg_ratio_current = (Sensor_ReadReg(0x3d07)<<2) + ((temp>>6) & 0x03);
		otp_ptr->bg_ratio_current = (Sensor_ReadReg(0x3d08)<<2) + ((temp>>4) & 0x03);
		light_rg = ((Sensor_ReadReg(0x3d0c)<<2) + (temp>>2)) & 0x03;
		light_bg = ((Sensor_ReadReg(0x3d0d)<<2) + temp) & 0x03;

		//(*otp_ptr).user_data[0] = Sensor_ReadReg(0x3d09);
		//(*otp_ptr).user_data[1] = Sensor_ReadReg(0x3d0a);
	}
	else if (index == 2)
	{
		// read otp --Bank 0
		Sensor_WriteReg(0x3d84, 0xc0);
		Sensor_WriteReg(0x3d85, 0x00);
		Sensor_WriteReg(0x3d86, 0x0f);
		Sensor_WriteReg(0x3d81, 0x01);
		usleep(5 * 1000);
		otp_ptr->module_id = (Sensor_ReadReg(0x3d0e) & 0x7f);
		otp_ptr->lens_id  = Sensor_ReadReg(0x3d0f);
		// read otp --Bank 1
		Sensor_WriteReg(0x3d84, 0xc0);
		Sensor_WriteReg(0x3d85, 0x10);
		Sensor_WriteReg(0x3d86, 0x1f);
		Sensor_WriteReg(0x3d81, 0x01);
		usleep(5 * 1000);
		temp = Sensor_ReadReg(0x3d04);
		otp_ptr->rg_ratio_current = (Sensor_ReadReg(0x3d00)<<2) + ((temp>>6) & 0x03);
		otp_ptr->bg_ratio_current = (Sensor_ReadReg(0x3d01)<<2) + ((temp>>4) & 0x03);
		light_rg = ((Sensor_ReadReg(0x3d05)<<2) + (temp>>2)) & 0x03;
		light_bg = ((Sensor_ReadReg(0x3d06)<<2) + temp) & 0x03;
		//(*otp_ptr).user_data[0] = Sensor_ReadReg(0x3d02);
		//(*otp_ptr).user_data[1] = Sensor_ReadReg(0x3d03);
	} 
	else if (index == 3)
	{
		// read otp --Bank 1
		Sensor_WriteReg(0x3d84, 0xc0);
		Sensor_WriteReg(0x3d85, 0x10);
		Sensor_WriteReg(0x3d86, 0x1f);
		Sensor_WriteReg(0x3d81, 0x01);
		usleep(5 * 1000);
		otp_ptr->module_id = (Sensor_ReadReg(0x3d07) & 0x7f);
		otp_ptr->lens_id = Sensor_ReadReg(0x3d08);
		temp = Sensor_ReadReg(0x3d0d);
		otp_ptr->rg_ratio_current = (Sensor_ReadReg(0x3d09)<<2) + ((temp>>6) & 0x03);
		otp_ptr->bg_ratio_current = (Sensor_ReadReg(0x3d0a)<<2) + ((temp>>4) & 0x03);
		light_rg = ((Sensor_ReadReg(0x3d0e)<<2) + (temp>>2)) & 0x03;
		light_bg = ((Sensor_ReadReg(0x3d0f)<<2) + temp) & 0x03;
		//(*otp_ptr).user_data[0] = Sensor_ReadReg(0x3d0b);
		//(*otp_ptr).user_data[1] = Sensor_ReadReg(0x3d0c);
	}

	// clear otp buffer
	for (i=0;i<16;i++) {
		Sensor_WriteReg(0x3d00 + i, 0x00);
	}

	return 0;
	}

static uint32_t ov5648_DARLING_read_otp_info(void *param_ptr)
{
	uint32_t rtn = SENSOR_SUCCESS;
	struct otp_info_t *otp_info=(struct otp_info_t *)param_ptr;
	otp_info->rg_ratio_typical=RG_TYPICAL_ov5648_DARLING;
	otp_info->bg_ratio_typical=BG_TYPICAL_ov5648_DARLING;
	otp_info->r_typical=R_TYPICAL_ov5648_DARLING;
	otp_info->g_typical=G_TYPICAL_ov5648_DARLING;
	otp_info->b_typical=B_TYPICAL_ov5648_DARLING;
	/*TODO*/
	
	int i = 0;
	int otp_index = 0;
	int temp = 0;
	uint16_t stream_value = 0;
	
	/*get otp availabe id*/
	stream_value = Sensor_ReadReg(0x0100);
	SENSOR_PRINT("ov5648_check_otp_module_id:stream_value = 0x%x\n", stream_value);
	if(1 != (stream_value & 0x01))
	{
		Sensor_WriteReg(0x0100, 0x01);
		usleep(50 * 1000);
	}
	/* R/G and B/G of current camera module is read out from sensor OTP
	    check first OTP with valid data*/
	
	for(i=1;i<=3;i++) {
		temp = ov5648_check_otp(i);
		SENSOR_PRINT("ov5648_check_otp_module_id i=%d temp = %d \n",i,temp);
		if (temp == 2) {
			otp_index = i;
			break;
		}
	}
	if (i > 3) {
		/* no valid wb OTP data*/
		SENSOR_PRINT("ov5648_check_otp_module_id no valid wb OTP data\n");
		return SENSOR_FAIL;
	}
	
	ov5648_read_otp(otp_index, otp_info);
 
	/*print otp information*/
	SENSOR_PRINT("flag=0x%x",otp_info->flag);
	SENSOR_PRINT("module_id=0x%x",otp_info->module_id);
	SENSOR_PRINT("lens_id=0x%x",otp_info->lens_id);
	SENSOR_PRINT("vcm_id=0x%x",otp_info->vcm_id);
	SENSOR_PRINT("vcm_id=0x%x",otp_info->vcm_id);
	SENSOR_PRINT("vcm_driver_id=0x%x",otp_info->vcm_driver_id);
	SENSOR_PRINT("data=%d-%d-%d",otp_info->year,otp_info->month,otp_info->day);
	SENSOR_PRINT("rg_ratio_current=0x%x",otp_info->rg_ratio_current);
 	SENSOR_PRINT("bg_ratio_current=0x%x",otp_info->bg_ratio_current);
	SENSOR_PRINT("rg_ratio_typical=0x%x",otp_info->rg_ratio_typical);
	SENSOR_PRINT("bg_ratio_typical=0x%x",otp_info->bg_ratio_typical);
	SENSOR_PRINT("r_current=0x%x",otp_info->r_current);
	SENSOR_PRINT("g_current=0x%x",otp_info->g_current);
	SENSOR_PRINT("b_current=0x%x",otp_info->b_current);
	SENSOR_PRINT("r_typical=0x%x",otp_info->r_typical);
	SENSOR_PRINT("g_typical=0x%x",otp_info->g_typical);
	SENSOR_PRINT("b_typical=0x%x",otp_info->b_typical);
	SENSOR_PRINT("vcm_dac_start=0x%x",otp_info->vcm_dac_start);
	SENSOR_PRINT("vcm_dac_inifity=0x%x",otp_info->vcm_dac_inifity);
	SENSOR_PRINT("vcm_dac_macro=0x%x",otp_info->vcm_dac_macro);
	
	return rtn;
}


// R_gain, sensor red gain of AWB, 0x400 =1
// G_gain, sensor green gain of AWB, 0x400 =1
// B_gain, sensor blue gain of AWB, 0x400 =1
// return 0;
LOCAL int ov5648_update_awb_gain(int R_gain, int G_gain, int B_gain)
{
	if (R_gain>0x400) {
		Sensor_WriteReg(0x5186, R_gain>>8);
		Sensor_WriteReg(0x5187, R_gain & 0x00ff);
	}
	if (G_gain>0x400) {
		Sensor_WriteReg(0x5188, G_gain>>8);
		Sensor_WriteReg(0x5189, G_gain & 0x00ff);
	}
	if (B_gain>0x400) {
		Sensor_WriteReg(0x518a, B_gain>>8);
		Sensor_WriteReg(0x518b, B_gain & 0x00ff);
	}
	return 0;
}
// call this function after OV5648 initialization
// return: 0 update success
// 1, no OTP

// call this function after OV5648 initialization
// return: 0 update success
// 1, no OTP
LOCAL uint32_t ov5648_otp_update_otp (void *param_ptr)
{
	struct otp_info_t *otp_info=(struct otp_info_t *)param_ptr;
	int i = 0;
	int otp_index = 0;
	int temp = 0;
	int R_gain, G_gain, B_gain, G_gain_R, G_gain_B;
	int rg = 0;
	int bg = 0;
	uint16_t stream_value = 0;
	uint16_t reg_value = 0;
	uint16_t Base_gain=0x400;

	stream_value = Sensor_ReadReg(0x0100);
	SENSOR_PRINT("ov5648 update_otp:stream_value = 0x%x\n", stream_value);
	
	if(1 != (stream_value & 0x01)){
		Sensor_WriteReg(0x0100, 0x01);
		usleep(50 * 1000);
	}

	if(light_rg==0) {
		// no light source information in OTP
		rg = otp_info->rg_ratio_current;
	}
	else {
		// light source information found in OTP
		rg = otp_info->rg_ratio_current* (light_rg +512) / 1024;
	}
	if(light_bg==0) {
		// no light source information in OTP
		bg = otp_info->bg_ratio_current;
	}
	else {
		// light source information found in OTP
		bg = otp_info->bg_ratio_current* (light_bg +512) / 1024;
	}
	//calculate G gain
	//0x400 = 1x gain
	
	//calculate sensor WB gain, 0x400 = 1x gain
	R_gain = 0x400 * (otp_info->rg_ratio_typical)/ rg;
	G_gain = 0x400;
	B_gain = 0x400 * (otp_info->bg_ratio_typical)/ bg;

	// find gain<0x400
	if ((R_gain < 0x400) || (B_gain < 0x400)) {
		if (R_gain < B_gain)
			Base_gain = R_gain;
		else
			Base_gain = B_gain;

		// set min gain to 0x400
		R_gain = 0x400 * R_gain / Base_gain;
		G_gain = 0x400 * G_gain / Base_gain;
		B_gain = 0x400 * B_gain / Base_gain;
	}

	ov5648_update_awb_gain(R_gain, G_gain, B_gain);

	if(1 != (stream_value & 0x01))
		Sensor_WriteReg(0x0100, stream_value);

	SENSOR_PRINT("ov5648_otp: R_gain:0x%x, G_gain:0x%x, B_gain:0x%x------\n",R_gain, G_gain, B_gain);

	return 0;
}

static uint32_t ov5648_DARLING_update_awb(void *param_ptr)
{
	uint32_t rtn = SENSOR_SUCCESS;
	struct otp_info_t *otp_info=(struct otp_info_t *)param_ptr;

	/*TODO*/

	rtn=ov5648_otp_update_otp(otp_info);
	
	return rtn;
} 

#ifndef OTP_AUTO_LOAD_LSC_ov5648_DARLING

static uint32_t ov5648_DARLING_update_lsc(void *param_ptr)
{
	uint32_t rtn = SENSOR_SUCCESS;
	struct otp_info_t *otp_info=(struct otp_info_t *)param_ptr;

	/*TODO*/


	return rtn;
}

#endif
static uint32_t ov5648_DARLING_update_otp(void *param_ptr)
{
	uint32_t rtn = SENSOR_SUCCESS;
	struct otp_info_t *otp_info=(struct otp_info_t *)param_ptr;

	rtn=ov5648_DARLING_update_awb(param_ptr);
	if(rtn!=SENSOR_SUCCESS)
	{
		SENSOR_PRINT_ERR("OTP awb appliy error!");
		return rtn;
	}

	#ifndef OTP_AUTO_LOAD_LSC_ov5648_DARLING
	
	rtn=ov5648_DARLING_update_lsc(param_ptr);
	if(rtn!=SENSOR_SUCCESS)
	{
		SENSOR_PRINT_ERR("OTP lsc appliy error!");
		return rtn;
	}
	#endif

	return rtn;
}

static uint32_t ov5648_DARLING_identify_otp(void *param_ptr)
{
	uint32_t rtn = SENSOR_SUCCESS;

	rtn=ov5648_DARLING_read_otp_info(param_ptr);
	SENSOR_PRINT("rtn=%d",rtn);

	return rtn;
}

