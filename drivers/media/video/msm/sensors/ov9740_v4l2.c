/* Copyright (c) 2012, Code Aurora Forum. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include "msm_sensor.h"
#include "msm.h"
#define SENSOR_NAME "ov9740"

DEFINE_MUTEX(ov9740_mut);
static struct msm_sensor_ctrl_t ov9740_s_ctrl;

static int8_t  zte_force_write=0;
static int  zte_iso=0xff;
static int  zte_brightness=0xff;
static int  zte_awb=0xff;
static int  zte_antibanding=0xff;

static int is_init=0;


struct ov9740_i2c_reg_conf {
    uint16_t  waddr;
    uint16_t wdata;
};

static int csi_config=0;


static int32_t ov9740_i2c_write_table(struct msm_sensor_ctrl_t *s_ctrl, struct ov9740_i2c_reg_conf const *reg_conf_tbl,int len)
{
	uint32_t i;
	int32_t rc = 0;
	
	for (i = 0; i < len; i++)
	{
		    rc=msm_camera_i2c_write(s_ctrl->sensor_i2c_client, reg_conf_tbl[i].waddr, reg_conf_tbl[i].wdata, MSM_CAMERA_I2C_BYTE_DATA);
		if (rc < 0)
		{
		    break;
		}
     }
    return 0;
}


static struct msm_camera_i2c_reg_conf ov9740_recommend_settings[] = {
};


/* Preview and Snapshot Setup */
static struct ov9740_i2c_reg_conf const prev_snap_tbl[] = {
	//Orientation
	#if 1	
	{0x0101 ,0x02},//yuansheng
	{0x0347 ,0x05},//Y start	 893U10
	{0x034b ,0xd9},// Y end	893U10	
	#else	
	{0x0101 ,0x01},//bit1 flip bit0 mirror 893U10 893A20 893A21
	{0x0347 ,0x04},//Y start top left pixel data
	{0x034b ,0xd8},// Y end  bottom right pixel data	
	#endif		
	//PLL setting
	{0x3104 ,0x20},// PLL mode control
	{0x0305 ,0x03},// PLL control
	{0x0307 ,0x4c},// PLL control
	{0x0303 ,0x01},// PLL control
	{0x0301 ,0x08},// PLL control
	{0x3010 ,0x01},// PLL control
	//Timing setting
	{0x0340 ,0x03},//VTS
	{0x0341 ,0x07},//VTS
	{0x0342 ,0x06},//HTS
	{0x0343 ,0x62},//HTS
	{0x0344 ,0x00},// X start
	{0x0345 ,0x08},//X start
	{0x0346 ,0x00},//Y start
	//{0x0347 ,0x04},//Y start top left pixel data
	{0x0348 ,0x05},//X end
	{0x0349 ,0x0c},//X end
	{0x034a ,0x02},//Y end
	//{0x034b ,0xd8},// Y end  bottom right pixel data
	{0x034c ,0x05},// H output size
	{0x034d ,0x00},//H output size
	{0x034e ,0x02},//V output size
	{0x034f ,0xd0},//V output size
	//Output select 
	{0x3002 ,0x00},// IO control
	{0x3004 ,0x00},// IO control
	{0x3005 ,0x00},// IO control
	{0x3012 ,0x70},// MIPI control
	{0x3013 ,0x60},//MIPI control
	{0x3014 ,0x01},//MIPI control
	{0x301f ,0x03},// MIPI control
	{0x3026 ,0x00},// Output select
	{0x3027 ,0x00},// Output select
	//Analog control
	{0x3601 ,0x40},// Analog control
	{0x3602 ,0x16},// Analog control
	{0x3603 ,0xaa},// Analog control
	{0x3604 ,0x0c},// Analog control
	{0x3610 ,0xa1},// Analog control
	{0x3612 ,0x24},// Analog control
	{0x3620 ,0x66},// Analog control
	{0x3621 ,0xc0},// Analog control
	{0x3622 ,0x9f},// Analog control
	{0x3630 ,0xd2},// Analog control
	{0x3631 ,0x5e},// Analog control
	{0x3632 ,0x27},// Analog control
	{0x3633 ,0x50},// Analog control
	// Sensor control
	{0x3703 ,0x42},// Sensor control 
	{0x3704 ,0x10},// Sensor control 
	{0x3705 ,0x45},// Sensor control 
	{0x3707 ,0x11},// Sensor control 
	//Timing control
	{0x3817 ,0x94},// Internal timing control
	{0x3819 ,0x6e},// Internal timing control
	{0x3831 ,0x40},// Digital gain enable
	{0x3833 ,0x04},// Internal timing control
	{0x3835 ,0x04},// Internal timing control
	{0x3837 ,0x01},
	//Internal timing control;; AEC/AGC control
	{0x3503 ,0x10},//AEC/AGC control
	{0x3a18 ,0x01},//Gain ceiling
	{0x3a19 ,0xB5},//Gain ceiling
	{0x3a1a ,0x05},//Max diff
	{0x3a11 ,0x90},// High threshold
	{0x3a1b ,0x4a},// WPT 2 
	{0x3a0f ,0x48},// WPT  
	{0x3a10 ,0x44},// BPT  
	{0x3a1e ,0x42},// BPT 2 
	{0x3a1f ,0x22},//; Low threshold 
	//Banding filter
	{0x3a08 ,0x00},// 50Hz banding step
	{0x3a09 ,0xe8},// 50Hz banding step	
	{0x3a0e ,0x03},// 50Hz banding Max
	{0x3a14 ,0x15},// 50Hz Max exposure
	{0x3a15 ,0xc6},// 50Hz Max exposure
	{0x3a0a ,0x00},// 60Hz banding step
	{0x3a0b ,0xc0},// 60Hz banding step
	{0x3a0d ,0x04},// 60Hz banding Max
	{0x3a02 ,0x18},// 60Hz Max exposure
	{0x3a03 ,0x20},// 60Hz Max exposure

	//50/60 detection
	{0x3c0a ,0x9c},// Number of samples
	{0x3c0b ,0x3f},// Number of samples

	// BLC control
	{0x4002 ,0x45},// BLC auto enable
	{0x4005 ,0x18},// BLC mode

	// VFIFO control
	{0x4601 ,0x16},// VFIFO control
	{0x460e ,0x82},	
	// VFIFO control

	// DVP control
	{0x4702 ,0x04},// Vsync control
	{0x4704 ,0x00},// Vsync mode 
	{0x4706 ,0x08},// Vsync control

	//MIPI control
	{0x4800 ,0x44},// MIPI control
	{0x4801 ,0x0f},//
	// MIPI control
	{0x4803 ,0x05},
	// MIPI control
	{0x4805 ,0x10},
	// MIPI control
	{0x4837 ,0x20},////niujing modify mipi before 0x30
	{0x4831 ,0x07},////niujing modify mipi HS_Prepare	
	{0x482a ,0x15},//HS_Prepare+HS_zero niujing modify mipi before 
	{0x482b ,0x15},//HS-trail niujing modify mipi before 
	// MIPI control;; ISP control
	{0x5000 ,0xff},// [7] LC [6] Gamma [3] DNS [2] BPC [1] WPC [0] CIP
	{0x5001 ,0xff},// [7] SDE [6] UV adjust [4] scale [3] contrast [2] UV average [1] CMX [0] AWB
	{0x5003 ,0xff},// [7] PAD [5] Buffer [3] Vario [1] BLC [0] AWB gain
    //;;AWB
	{0x5180 ,0xf0}, //AWB setting
	{0x5181 ,0x00}, //;AWB setting
	{0x5182 ,0x41}, //AWB setting 
	{0x5183 ,0x42}, //AWB setting
	{0x5184 ,0x80}, //AWB setting
	{0x5185 ,0x68}, //AWB setting
	{0x5186 ,0x93}, //AWB setting 
	{0x5187 ,0xa8}, //AWB setting
	{0x5188 ,0x17}, //AWB setting
	{0x5189 ,0x45}, //AWB setting
	{0x518a ,0x27}, //AWB setting
	{0x518b ,0x41}, //AWB setting
	{0x518c ,0x2d}, //AWB setting
	{0x518d ,0xf0}, //AWB setting
	{0x518e ,0x10}, //;AWB setting
	{0x518f ,0xff}, //AWB setting
	{0x5190 ,0x00}, //AWB setting
	{0x5191 ,0xff}, //AWB setting 
	{0x5192 ,0x00}, //;AWB setting
	{0x5193 ,0xff}, //AWB setting 
	{0x5194 ,0x00}, //AWB setting 

	// DNS
	{0x529a ,0x02}, //DNS setting
	{0x529b ,0x08}, //DNS setting
	{0x529c ,0x0a}, //DNS setting
	{0x529d ,0x10}, //DNS setting
	{0x529e ,0x10}, //;DNS setting
	{0x529f ,0x28}, //	;;DNS setting
	{0x52a0 ,0x32}, //;DNS setting
	{0x52a2 ,0x00}, //DNS setting 
	{0x52a3 ,0x02}, //DNS setting 
	{0x52a4 ,0x00}, //DNS setting 
	{0x52a5 ,0x04}, //DNS setting  
	{0x52a6 ,0x00}, //DNS setting  
	{0x52a7 ,0x08}, //DNS setting  
	{0x52a8 ,0x00}, //DNS setting  
	{0x52a9 ,0x10}, //;;DNS setting
	{0x52aa ,0x00}, //DNS setting  
	{0x52ab ,0x38}, //;;DNS setting
	{0x52ac ,0x00}, //DNS setting  
	{0x52ad ,0x3c}, //;DNS setting 
	{0x52ae ,0x00}, //DNS setting   
	{0x52af ,0x4c}, //;DNS setting

	//CIP
	{0x530d ,0x06}, //CIP setting

	//CMX
	{0x5380 ,0x01},  //CMX setting  
	{0x5381 ,0x00},  //CMX setting  
	{0x5382 ,0x00},  //CMX setting   
	{0x5383 ,0x0d},  //CMX setting  
	{0x5384 ,0x00},  //CMX setting  
	{0x5385 ,0x2f},  //CMX setting   
	{0x5386 ,0x00},  //CMX setting  
	{0x5387 ,0x00},  //CMX setting   
	{0x5388 ,0x00},  //CMX setting   
	{0x5389 ,0xd3},  //CMX setting  
	{0x538a ,0x00},  //CMX setting   
	{0x538b ,0x0f},  //CMX setting   
	{0x538c ,0x00},  //CMX setting   
	{0x538d ,0x00},  //CMX setting  
	{0x538e ,0x00},  //CMX setting  
	{0x538f ,0x32},  //CMX setting   
	{0x5390 ,0x00},  //CMX setting  
	{0x5391 ,0x94},  //CMX setting    
	{0x5392 ,0x00},  //CMX setting  
	{0x5393 ,0xa4},  //CMX setting  
	{0x5394 ,0x18},  //CMX setting 
	//Contrast
	{0x5401 ,0x2c}, // Contrast setting
	{0x5403 ,0x28}, // Contrast setting
	{0x5404 ,0x06}, // Contrast setting	
	{0x5405 ,0xe0}, // Contrast setting

	//Y Gamma
	{0x5480 ,0x04}, //Y Gamma setting  
	{0x5481 ,0x12}, //Y Gamma setting 
	{0x5482 ,0x27}, //Y Gamma setting  
	{0x5483 ,0x49}, //Y Gamma setting  
	{0x5484 ,0x57}, //Y Gamma setting  
	{0x5485 ,0x66}, //Y Gamma setting  
	{0x5486 ,0x75}, //Y Gamma setting  
	{0x5487 ,0x81}, //Y Gamma setting 
	{0x5488 ,0x8c}, //Y Gamma setting 
	{0x5489 ,0x95}, //Y Gamma setting 
	{0x548a ,0xa5}, //Y Gamma setting 
	{0x548b ,0xb2}, //Y Gamma setting 
	{0x548c ,0xc8}, //Y Gamma setting 
	{0x548d ,0xd9}, //Y Gamma setting 
	{0x548e ,0xec}, //Y Gamma setting 

	//UV Gamma
	{0x5490 ,0x01}, //UV Gamma setting 
	{0x5491 ,0xc0}, //UV Gamma setting 
	{0x5492 ,0x03}, //UV Gamma setting 
	{0x5493 ,0x00}, //UV Gamma setting 
	{0x5494 ,0x03}, //UV Gamma setting 
	{0x5495 ,0xe0}, //UV Gamma setting 
	{0x5496 ,0x03}, //UV Gamma setting 
	{0x5497 ,0x10}, //UV Gamma setting 
	{0x5498 ,0x02}, //UV Gamma setting 
	{0x5499 ,0xac}, //UV Gamma setting 
	{0x549a ,0x02}, //UV Gamma setting 
	{0x549b ,0x75}, //UV Gamma setting 
	{0x549c ,0x02}, //;UV Gamma setting 
	{0x549d ,0x44}, //UV Gamma setting 
	{0x549e ,0x02}, //UV Gamma setting 
	{0x549f ,0x20}, //UV Gamma setting 
	{0x54a0 ,0x02}, //UV Gamma setting 
	{0x54a1 ,0x07}, //UV Gamma setting 
	{0x54a2 ,0x01}, //UV Gamma setting 
	{0x54a3 ,0xec}, //UV Gamma setting 
	{0x54a4 ,0x01}, //UV Gamma setting 
	{0x54a5 ,0xc0}, //UV Gamma setting 
	{0x54a6 ,0x01}, //UV Gamma setting 
	{0x54a7 ,0x9b}, //UV Gamma setting 
	{0x54a8 ,0x01}, //UV Gamma setting 
	{0x54a9 ,0x63}, //UV Gamma setting 
	{0x54aa ,0x01}, //UV Gamma setting 
	{0x54ab ,0x2b}, //UV Gamma setting 
	{0x54ac ,0x01}, //UV Gamma setting 
	{0x54ad ,0x22}, //UV Gamma setting 
	// UV adjust
	{0x5501 ,0x1c}, //UV adjust setting 
	{0x5502 ,0x00}, //;UV adjust setting 
	{0x5503 ,0x40}, //UV adjust setting 
	{0x5504 ,0x00}, //UV adjust setting 
	{0x5505 ,0x80}, //UV adjust setting 
	//Lens correction
	{0x5800 ,0x1c}, // Lens correction setting
	{0x5801 ,0x16}, // Lens correction setting
	{0x5802 ,0x15}, // Lens correction setting
	{0x5803 ,0x16}, // Lens correction setting
	{0x5804 ,0x18}, // Lens correction setting
	{0x5805 ,0x1a}, // Lens correction setting
	{0x5806 ,0x0c}, // Lens correction setting
	{0x5807 ,0x0a}, // Lens correction setting
	{0x5808 ,0x08}, // Lens correction setting
	{0x5809 ,0x08}, // Lens correction setting
	{0x580a ,0x0a}, // Lens correction setting
	{0x580b ,0x0b}, // Lens correction setting
	{0x580c ,0x05}, // Lens correction setting
	{0x580d ,0x02}, // Lens correction setting
	{0x580e ,0x00}, // Lens correction setting
	{0x580f ,0x00}, // Lens correction setting
	{0x5810 ,0x02}, // Lens correction setting
	{0x5811 ,0x05}, // Lens correction setting
	{0x5812 ,0x04}, // Lens correction setting
    {0x5813 ,0x01}, // Lens correction setting
	{0x5814 ,0x00}, // Lens correction setting
	{0x5815 ,0x00}, // Lens correction setting
	{0x5816 ,0x02}, // Lens correction setting
	{0x5817 ,0x03}, // Lens correction setting
	{0x5818 ,0x0a}, // Lens correction setting
	{0x5819 ,0x07}, // Lens correction setting
	{0x581a ,0x05}, // Lens correction setting
	{0x581b ,0x05}, // Lens correction setting
	{0x581c ,0x08}, // Lens correction setting
	{0x581d ,0x0b}, // Lens correction setting
	{0x581e ,0x15}, // Lens correction setting
	{0x581f ,0x14}, // Lens correction setting
	{0x5820 ,0x14}, // Lens correction setting
	{0x5821 ,0x13}, // Lens correction setting
	{0x5822 ,0x17}, // Lens correction setting
	{0x5823 ,0x16}, // Lens correction setting
	{0x5824 ,0x46}, // Lens correction setting
	{0x5825 ,0x4c}, // Lens correction setting
	{0x5826 ,0x6c}, // Lens correction setting
	{0x5827 ,0x4c}, // Lens correction setting
	{0x5828 ,0x80}, // Lens correction setting
	{0x5829 ,0x2e}, // Lens correction setting
	{0x582a ,0x48}, // Lens correction setting
	{0x582b ,0x46}, // Lens correction setting
	{0x582c ,0x2a}, // Lens correction setting
	{0x582d ,0x68}, // Lens correction setting
	{0x582e ,0x08}, // Lens correction setting
	{0x582f ,0x26}, // Lens correction setting
	{0x5830 ,0x44}, // Lens correction setting
	{0x5831 ,0x46}, // Lens correction setting
	{0x5832 ,0x62}, // Lens correction setting
	{0x5833 ,0x0c}, // Lens correction setting
	{0x5834 ,0x28}, // Lens correction setting
	{0x5835 ,0x46}, // Lens correction setting
	{0x5836 ,0x28}, // Lens correction setting
	{0x5837 ,0x88}, // Lens correction setting
	{0x5838 ,0x0e}, // Lens correction setting
	{0x5839 ,0x0e}, // Lens correction setting
	{0x583a ,0x2c}, // Lens correction setting
	{0x583b ,0x2e}, // Lens correction setting
	{0x583c ,0x46}, // Lens correction setting
	{0x583d ,0xca}, // Lens correction setting
	{0x583e ,0xf0}, // Lens correction setting
	{0x5842 ,0x02}, // Lens correction setting
	{0x5843 ,0x5e}, // Lens correction setting
	{0x5844 ,0x04}, // Lens correction setting
	{0x5845 ,0x32}, // Lens correction setting
	{0x5846 ,0x03}, // Lens correction setting
	{0x5847 ,0x29}, // Lens correction setting
	{0x5848 ,0x02}, // Lens correction setting
	{0x5849 ,0xcc}, // Lens correction setting
	//{0x501a ,0xe0}, //colorbar0x80
	{0x4300 ,0x32}, // YUV shunxu
	{0x0340 ,0x03},
	{0x0341 ,0xc4}, //vts to 0x3c4
	{0x3c01 ,0x80}, //band manual mode enable
	{0x3a0d ,0x05}, //60Hz banding Max
	{0x3a18 ,0x00}, //Gain Ceiling 8x
	{0x3a19 ,0x80},
};


static struct v4l2_subdev_info ov9740_subdev_info[] = {
	{
		.code   = V4L2_MBUS_FMT_YUYV8_2X8,
		.colorspace = V4L2_COLORSPACE_JPEG,
		.fmt    = 1,
		.order    = 0,
	},
	/* more can be supported, to be added later */
};

static struct msm_camera_i2c_conf_array ov9740_init_conf[] = {
	{ov9740_recommend_settings,
	ARRAY_SIZE(ov9740_recommend_settings), 0, MSM_CAMERA_I2C_BYTE_DATA},
};

static struct msm_camera_i2c_conf_array ov9740_confs[] = {
	{ov9740_recommend_settings,
	ARRAY_SIZE(ov9740_recommend_settings), 0, MSM_CAMERA_I2C_BYTE_DATA},
};
static struct msm_sensor_output_info_t ov9740_dimensions[] = {
	{
		.x_output = 0x500,
		.y_output = 0x2d0,
		.line_length_pclk = 0x500,
		.frame_length_lines = 0x2d0,
		.vt_pixel_clk = 76000000,
		.op_pixel_clk = 304000000,
		.binning_factor = 1,
	},

};

static struct msm_camera_csi_params ov9740_csi_params = {
	.data_format = CSI_8BIT,
	.lane_cnt    = 1,
	.lane_assign = 0xe4,
	.dpcm_scheme = 0,
	.settle_cnt  = 7
};

static struct msm_camera_csi_params *ov9740_csi_params_array[] = {
	&ov9740_csi_params,
	&ov9740_csi_params,
};

static struct msm_sensor_output_reg_addr_t ov9740_reg_addr = {
	.x_output = 0xcc,	
	.y_output = 0xce,	
	.line_length_pclk = 0xcc,	
	.frame_length_lines = 0xce,
};

static struct msm_sensor_id_info_t ov9740_id_info = {
	.sensor_id_reg_addr = 0x0,
	.sensor_id = 0x9740,
};

static const struct i2c_device_id ov9740_i2c_id[] = {
	{SENSOR_NAME, (kernel_ulong_t)&ov9740_s_ctrl},
	{ }
};

static struct msm_cam_clk_info cam_clk_info[] = {
	{"cam_clk", MSM_SENSOR_MCLK_24HZ},
};


int32_t ov9740_sensor_i2c_probe_for_init(struct i2c_client *client,
	const struct i2c_device_id *id)
{

	int rc = 0;
	uint8_t cam_front = 0;
	uint8_t cam_main = 0;
		
	struct msm_sensor_ctrl_t *s_ctrl;
	struct msm_camera_sensor_info *data;

	msm_cam_get_camera_num( &cam_front, &cam_main);
	pr_err("%s: cam_front = %d,cam_main = %d\n",__func__,cam_front,cam_main);
	
	pr_err("%s %s_i2c_probe called\n", __func__, client->name);
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		pr_err("%s %s i2c_check_functionality failed\n",
			__func__, client->name);
		rc = -EFAULT;
		return rc;
	}

	s_ctrl = (struct msm_sensor_ctrl_t *)(id->driver_data);
	if (s_ctrl->sensor_i2c_client != NULL) {
		s_ctrl->sensor_i2c_client->client = client;
		if (s_ctrl->sensor_i2c_addr != 0)
			s_ctrl->sensor_i2c_client->client->addr =
				s_ctrl->sensor_i2c_addr;
	} else {
		pr_err("%s %s sensor_i2c_client NULL\n",
			__func__, client->name);
		rc = -EFAULT;
		return rc;
	}

	s_ctrl->sensordata = client->dev.platform_data;
	if (s_ctrl->sensordata == NULL) {
		pr_err("%s %s NULL sensor data\n", __func__, client->name);
		return -EFAULT;
	}

      data = s_ctrl->sensordata;
      s_ctrl->reg_ptr = kzalloc(sizeof(struct regulator *)
			* data->sensor_platform_info->num_vreg, GFP_KERNEL);
	if (!s_ctrl->reg_ptr) {
		pr_err("%s: could not allocate mem for regulators\n",
			__func__);
		return -ENOMEM;
	}

    //if main camera has been up,donnot config_vreg again
    if( cam_main == 0)
    {
	rc = msm_camera_config_vreg(&s_ctrl->sensor_i2c_client->client->dev,
			s_ctrl->sensordata->sensor_platform_info->cam_vreg,
			s_ctrl->sensordata->sensor_platform_info->num_vreg,
			s_ctrl->reg_ptr, 1);
	if (rc < 0) {
		pr_err("%s: regulator on failed\n", __func__);
		goto config_vreg_failed;
	}

	rc = msm_camera_enable_vreg(&s_ctrl->sensor_i2c_client->client->dev,
			s_ctrl->sensordata->sensor_platform_info->cam_vreg,
			s_ctrl->sensordata->sensor_platform_info->num_vreg,
			s_ctrl->reg_ptr, 1);
	if (rc < 0) {
		pr_err("%s: enable regulator failed\n", __func__);
		goto enable_vreg_failed;
	}
	}

	gpio_direction_output(data->sensor_pwd, 0);
	mdelay(1);
	
	gpio_direction_output(data->sensor_reset, 1);
	mdelay(5);
	
	gpio_direction_output(data->sensor_reset, 0);
	mdelay(5);
	
	gpio_direction_output(data->sensor_reset, 1);
	mdelay(5);

	rc = msm_camera_request_gpio_table(data, 1);
	if (rc < 0) {
		pr_err("%s: request gpio failed\n", __func__);
		goto request_gpio_failed;
	}

	rc = msm_camera_config_gpio_table(data, 1);
	if (rc < 0) {
		pr_err("%s: config gpio failed\n", __func__);
		goto config_gpio_failed;
	}

	if (s_ctrl->clk_rate != 0)
		cam_clk_info->clk_rate = s_ctrl->clk_rate;

	rc = msm_cam_clk_enable(&s_ctrl->sensor_i2c_client->client->dev,
		cam_clk_info, &s_ctrl->cam_clk, ARRAY_SIZE(cam_clk_info), 1);
	if (rc < 0) {
		pr_err("%s: clk enable failed\n", __func__);
		goto enable_clk_failed;
	}
	mdelay(5);

	if (s_ctrl->func_tbl->sensor_match_id)
		rc = s_ctrl->func_tbl->sensor_match_id(s_ctrl);
	else
		rc = msm_sensor_match_id(s_ctrl);
	if (rc < 0)
		goto probe_fail;

#ifdef CONFIG_SENSOR_INFO
	msm_sensorinfo_set_front_sensor_id( s_ctrl->sensor_id_info->sensor_id);
#endif

	if (s_ctrl->func_tbl->sensor_csi_setting(s_ctrl, MSM_SENSOR_REG_INIT, 0) < 0)
	{
		pr_err("%s  sensor_setting init  failed\n",__func__);
		return rc;
	}

	snprintf(s_ctrl->sensor_v4l2_subdev.name,
		sizeof(s_ctrl->sensor_v4l2_subdev.name), "%s", id->name);
	v4l2_i2c_subdev_init(&s_ctrl->sensor_v4l2_subdev, client,
		s_ctrl->sensor_v4l2_subdev_ops);

	rc = msm_sensor_register(&s_ctrl->sensor_v4l2_subdev);
	pr_err("%s  rc=%d\n", __func__, rc);
	goto power_down;
probe_fail:
	pr_err("%s %s_i2c_probe failed\n", __func__, client->name);
	msm_cam_clk_enable(&s_ctrl->sensor_i2c_client->client->dev,
		cam_clk_info, &s_ctrl->cam_clk, ARRAY_SIZE(cam_clk_info), 0);
enable_clk_failed:
		msm_camera_config_gpio_table(data, 0);
config_gpio_failed:
	if( cam_main == 0)
	msm_camera_enable_vreg(&s_ctrl->sensor_i2c_client->client->dev,
			s_ctrl->sensordata->sensor_platform_info->cam_vreg,
			s_ctrl->sensordata->sensor_platform_info->num_vreg,
			s_ctrl->reg_ptr, 0);

enable_vreg_failed:
	if( cam_main == 0)
	msm_camera_config_vreg(&s_ctrl->sensor_i2c_client->client->dev,
		s_ctrl->sensordata->sensor_platform_info->cam_vreg,
		s_ctrl->sensordata->sensor_platform_info->num_vreg,
		s_ctrl->reg_ptr, 0);
config_vreg_failed:
	msm_camera_request_gpio_table(data, 0);
request_gpio_failed:
	kfree(s_ctrl->reg_ptr);
	return rc;
power_down:
	if (rc > 0)
		rc = 0;
	s_ctrl->func_tbl->sensor_power_down(s_ctrl);
	return rc;

}


static struct i2c_driver ov9740_i2c_driver = {
	.id_table = ov9740_i2c_id,
	.probe  = ov9740_sensor_i2c_probe_for_init,
	.driver = {
		.name = SENSOR_NAME,
	},
};

static struct msm_camera_i2c_client ov9740_sensor_i2c_client = {
	.addr_type = MSM_CAMERA_I2C_WORD_ADDR,
};

static int __init msm_sensor_init_module(void)
{
	int rc = 0;
	CDBG("ov9740\n");

	rc = i2c_add_driver(&ov9740_i2c_driver);

	return rc;
}

static struct v4l2_subdev_core_ops ov9740_subdev_core_ops = {
	.s_ctrl = msm_sensor_v4l2_s_ctrl,
	.queryctrl = msm_sensor_v4l2_query_ctrl,
	.ioctl = msm_sensor_subdev_ioctl,
	.s_power = msm_sensor_power,
};

static struct v4l2_subdev_video_ops ov9740_subdev_video_ops = {
	.enum_mbus_fmt = msm_sensor_v4l2_enum_fmt,
};

static struct v4l2_subdev_ops ov9740_subdev_ops = {
	.core = &ov9740_subdev_core_ops,
	.video  = &ov9740_subdev_video_ops,
};

int32_t msm_ov9740_sensor_power_up(struct msm_sensor_ctrl_t *s_ctrl)
{

	int32_t rc = 0;
	struct msm_camera_sensor_info *data = s_ctrl->sensordata;
	pr_err("%s: %d\n", __func__, __LINE__);
	s_ctrl->reg_ptr = kzalloc(sizeof(struct regulator *)
			* data->sensor_platform_info->num_vreg, GFP_KERNEL);
	if (!s_ctrl->reg_ptr) {
		pr_err("%s: could not allocate mem for regulators\n",
			__func__);
		return -ENOMEM;
	}

	rc = msm_camera_request_gpio_table(data, 1);
	if (rc < 0) {
		pr_err("%s: request gpio failed\n", __func__);
		goto request_gpio_failed;
	}

	rc = msm_camera_config_gpio_table(data, 1);
	if (rc < 0) {
		pr_err("%s: config gpio failed\n", __func__);
		goto config_gpio_failed;
	}

	if (s_ctrl->clk_rate != 0)
		cam_clk_info->clk_rate = s_ctrl->clk_rate;

	rc = msm_cam_clk_enable(&s_ctrl->sensor_i2c_client->client->dev,
		cam_clk_info, &s_ctrl->cam_clk, ARRAY_SIZE(cam_clk_info), 1);
	if (rc < 0) {
		pr_err("%s: clk enable failed\n", __func__);
		goto enable_clk_failed;
	}

	usleep_range(1000, 2000);

	gpio_direction_output(data->sensor_pwd, 0);
    msleep(10);

	rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,   0x3014, 0x05, MSM_CAMERA_I2C_BYTE_DATA);
	if (rc < 0) 
	pr_err("%s: i2c write failed\n", __func__);	
	//rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x0100, 0x00, MSM_CAMERA_I2C_BYTE_DATA);
	//if (rc < 0) {
	//pr_err("%s: OV9740_MODE_SELECT=standby failed\n", __func__);
	//} 

	if (data->sensor_platform_info->ext_power_ctrl != NULL)
		data->sensor_platform_info->ext_power_ctrl(1);

	if (data->sensor_platform_info->i2c_conf &&
		data->sensor_platform_info->i2c_conf->use_i2c_mux)
		msm_sensor_enable_i2c_mux(data->sensor_platform_info->i2c_conf);

	return rc;

enable_clk_failed:
		msm_camera_config_gpio_table(data, 0);
config_gpio_failed:
	msm_camera_enable_vreg(&s_ctrl->sensor_i2c_client->client->dev,
			s_ctrl->sensordata->sensor_platform_info->cam_vreg,
			s_ctrl->sensordata->sensor_platform_info->num_vreg,
			s_ctrl->reg_ptr, 0);
request_gpio_failed:
	kfree(s_ctrl->reg_ptr);
	return rc;
		
}

int32_t msm_ov9740_sensor_power_down(struct msm_sensor_ctrl_t *s_ctrl)
{
	struct msm_camera_sensor_info *data = s_ctrl->sensordata;
	int rc = 0;
	pr_err("%s\n", __func__);
	
	csi_config = 0;
	if (data->sensor_platform_info->i2c_conf &&
		data->sensor_platform_info->i2c_conf->use_i2c_mux)
		msm_sensor_disable_i2c_mux(
			data->sensor_platform_info->i2c_conf);

	if (data->sensor_platform_info->ext_power_ctrl != NULL)
		data->sensor_platform_info->ext_power_ctrl(0);
	#if 0
	rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,  0x3002, 0xe8, MSM_CAMERA_I2C_BYTE_DATA);
	if (rc < 0) {
	pr_err("%s: i2c write failed\n", __func__);		
	return rc;
	}
	rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,  0x3004, 0x03, MSM_CAMERA_I2C_BYTE_DATA);
	if (rc < 0) {
	pr_err("%s: i2c write failed\n", __func__);		
	return rc;
	}
	rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,   0x3005, 0xff, MSM_CAMERA_I2C_BYTE_DATA);
	if (rc < 0) {
	pr_err("%s: i2c write failed\n", __func__);		
	return rc;
	}
	#endif
	rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,   0x3014, 0x1d, MSM_CAMERA_I2C_BYTE_DATA);
	if (rc < 0) {
	pr_err("%s: i2c write failed\n", __func__);		
	return rc;
	}
		
    gpio_direction_output(data->sensor_pwd, 1);
	msleep(10);

	msm_cam_clk_enable(&s_ctrl->sensor_i2c_client->client->dev,
		cam_clk_info, &s_ctrl->cam_clk, ARRAY_SIZE(cam_clk_info), 0);
	msm_camera_config_gpio_table(data, 0);

	msm_camera_request_gpio_table(data, 0);
	if( s_ctrl->reg_ptr)
	{
	  kfree(s_ctrl->reg_ptr);
	  s_ctrl->reg_ptr = NULL;
	}

    zte_iso=0xff;
    zte_brightness=0xff;
    zte_awb=0xff;
	zte_antibanding=0xff;
	
	return 0;
}


//add params setting
int ov9740_set_iso(struct msm_sensor_ctrl_t *s_ctrl,
		struct msm_sensor_v4l2_ctrl_info_t *ctrl_info, int value)
{
	int rc = 0;
	pr_err("%s   value=%d\n", __func__, value);

	if(zte_force_write)
	{
	}
	else if( value == zte_iso)
	{
	   	pr_err("%s: return quickly\n", __func__);
	    return rc;
	}

	switch (value)
	{
	case MSM_V4L2_ISO_AUTO:
	{		
	    rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x3A18 ,0x00, MSM_CAMERA_I2C_BYTE_DATA);
        if (rc < 0)
        {
           return rc;
        }
	    rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x3A19 ,0x60, MSM_CAMERA_I2C_BYTE_DATA);
        if (rc < 0)
        {
           return rc;
        }
	}
	break;

	case MSM_V4L2_ISO_100:
	{
	    rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x3A18 ,0x00, MSM_CAMERA_I2C_BYTE_DATA);
        if (rc < 0)
        {
           return rc;
        }
	    rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x3A19 ,0x40, MSM_CAMERA_I2C_BYTE_DATA);
        if (rc < 0)
        {
           return rc;
        }
	}
	break;

	case MSM_V4L2_ISO_200:
	{		
	    rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x3A18 ,0x00, MSM_CAMERA_I2C_BYTE_DATA);
        if (rc < 0)
        {
           return rc;
        }
	    rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x3A19 ,0x60, MSM_CAMERA_I2C_BYTE_DATA);
        if (rc < 0)
        {
           return rc;
        }
	}
	break;

	case MSM_V4L2_ISO_400:
	{
	    rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x3A18 ,0x00, MSM_CAMERA_I2C_BYTE_DATA);
        if (rc < 0)
        {
           return rc;
        }
	    rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x3A19 ,0x80, MSM_CAMERA_I2C_BYTE_DATA);
        if (rc < 0)
        {
           return rc;
        }
	}
	break;

	case MSM_V4L2_ISO_800:
	{
		rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x3A18 ,0x00, MSM_CAMERA_I2C_BYTE_DATA);
        if (rc < 0)
        {
           return rc;
        }
	    rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x3A19 ,0xC0, MSM_CAMERA_I2C_BYTE_DATA);
        if (rc < 0)
        {
           return rc;
        }
		
	}
	break;
	default:
	{
	    pr_err("%s: parameter error!\n", __func__);
	    rc = -EFAULT;
	}     
	}

	/*
	* Attention
	*
	* Time delay of 100ms or more is required by sensor,
	*
	* ISO config will have no effect after setting 
	* without time delay of 100ms or more
	*/
	msleep(10);

	if( rc >=0 )
		zte_iso = value;
	return rc;

}


int ov9740_set_effect(struct msm_sensor_ctrl_t *s_ctrl,
		struct msm_sensor_v4l2_ctrl_info_t *ctrl_info, int value)
{

	long rc = 0;

      return rc;
}



int ov9740_set_brightness(struct msm_sensor_ctrl_t *s_ctrl,
		struct msm_sensor_v4l2_ctrl_info_t *ctrl_info, int value)
{

	long rc = 0;
	int16_t  tmp_reg = 0;
	pr_err("%s   value=%d\n", __func__, value);

    if(zte_force_write)
	{
	}
	else if( value == zte_brightness)
	{
	   	pr_err("%s: return quickly\n", __func__);
	    return rc;
	}

	switch (value)
	{
	  case CAMERA_BRIGHTNESS_0:
	  {
	     rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x5587, 0x40, MSM_CAMERA_I2C_BYTE_DATA);
		 if (rc < 0) return rc;
		 rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client,0x5588, &tmp_reg, MSM_CAMERA_I2C_BYTE_DATA);
		 if (rc < 0) return rc;
		 tmp_reg &= 0xf7; // set bit[3] = 0
		 rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x5588, tmp_reg, MSM_CAMERA_I2C_BYTE_DATA);
		 if (rc < 0) return rc;
	  	break;
	  }
	  case CAMERA_BRIGHTNESS_1:
	  {
		rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x5587, 0x30, MSM_CAMERA_I2C_BYTE_DATA);
		if (rc < 0) return rc;
		rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client,0x5588, &tmp_reg, MSM_CAMERA_I2C_BYTE_DATA);
		if (rc < 0) return rc;
		tmp_reg &= 0xf7; // set bit[3] = 0
		rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x5588, tmp_reg, MSM_CAMERA_I2C_BYTE_DATA);
		if (rc < 0) return rc;
	  	break;
	  }
	  case CAMERA_BRIGHTNESS_2:
	  {
		rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x5587, 0x20, MSM_CAMERA_I2C_BYTE_DATA);
		if (rc < 0) return rc;
		rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client,0x5588, &tmp_reg, MSM_CAMERA_I2C_BYTE_DATA);
		if (rc < 0) return rc;
		tmp_reg &= 0xf7; // set bit[3] = 0
		rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x5588, tmp_reg, MSM_CAMERA_I2C_BYTE_DATA);
		if (rc < 0) return rc;
	  	break;
	  }
	  case CAMERA_BRIGHTNESS_3:
	  {
		rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x5587, 0x10, MSM_CAMERA_I2C_BYTE_DATA);
		if (rc < 0) return rc;
		rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client,0x5588, &tmp_reg, MSM_CAMERA_I2C_BYTE_DATA);
		if (rc < 0) return rc;
		tmp_reg &= 0xf7; // set bit[3] = 0
		rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x5588, tmp_reg, MSM_CAMERA_I2C_BYTE_DATA);
		if (rc < 0) return rc;
	  	break;
	  }
	  case CAMERA_BRIGHTNESS_4:
	  {
		rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x5587, 0x00, MSM_CAMERA_I2C_BYTE_DATA);
		if (rc < 0) return rc;
		rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client,0x5588, &tmp_reg, MSM_CAMERA_I2C_BYTE_DATA);
		if (rc < 0) return rc;
		tmp_reg &= 0xf7; // set bit[3] = 0
		rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x5588, tmp_reg, MSM_CAMERA_I2C_BYTE_DATA);
		if (rc < 0) return rc;
	  	break;
	  }
	  case CAMERA_BRIGHTNESS_5:
	  {
	  	rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x5587, 0x10, MSM_CAMERA_I2C_BYTE_DATA);
		if (rc < 0) return rc;
		rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client,0x5588, &tmp_reg, MSM_CAMERA_I2C_BYTE_DATA);
		if (rc < 0) return rc;
		tmp_reg |= 0x08; // set bit[3] = 1
		rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x5588, tmp_reg, MSM_CAMERA_I2C_BYTE_DATA);
		if (rc < 0) return rc;
	  	break;
	  }
	  case CAMERA_BRIGHTNESS_6:
	  {
	  	rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x5587, 0x20, MSM_CAMERA_I2C_BYTE_DATA);
		if (rc < 0) return rc;
		rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client,0x5588, &tmp_reg, MSM_CAMERA_I2C_BYTE_DATA);
		if (rc < 0) return rc;
		tmp_reg |= 0x08; // set bit[3] = 1
		rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x5588, tmp_reg, MSM_CAMERA_I2C_BYTE_DATA);
		if (rc < 0) return rc;
	  	break;
	  }
	}
	
	if( rc >=0 )
		zte_brightness = value;
      return rc;
}


int ov9740_set_wb(struct msm_sensor_ctrl_t *s_ctrl,
		struct msm_sensor_v4l2_ctrl_info_t *ctrl_info, int value)
{
	int rc = 0;
	int16_t  tmp_reg = 0;
	pr_err("%s   value=%d\n", __func__, value);
 
	if(zte_force_write)
	{
	}
	else if( value == zte_awb)
	{
	   	pr_err("%s: return quickly\n", __func__);
	    return rc;
	}
	
    switch(value)
    {
        case MSM_V4L2_WB_AUTO:
        {
           	rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x3406, 0x00, MSM_CAMERA_I2C_BYTE_DATA);
			if (rc < 0) return rc;
        }
        break;
        case MSM_V4L2_WB_INCANDESCENT:
        {
			rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client,0x3406, &tmp_reg, MSM_CAMERA_I2C_BYTE_DATA);
			if (rc < 0) return rc;
			rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x3406, tmp_reg|0x01, MSM_CAMERA_I2C_BYTE_DATA);
			if (rc < 0) return rc;
			rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x3400, 0x04, MSM_CAMERA_I2C_BYTE_DATA);	
			if (rc < 0) return rc;
			rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x3401, 0x0a9, MSM_CAMERA_I2C_BYTE_DATA);
			if (rc < 0) return rc;
			rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x3402, 0x04, MSM_CAMERA_I2C_BYTE_DATA);
			if (rc < 0) return rc;
			rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x3403, 0x00, MSM_CAMERA_I2C_BYTE_DATA);
			if (rc < 0) return rc;
			rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x3404, 0x09, MSM_CAMERA_I2C_BYTE_DATA);
			if (rc < 0) return rc;
			rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x3405, 0x0e, MSM_CAMERA_I2C_BYTE_DATA); 
			if (rc < 0) return rc;
        }
        break;
        
        case MSM_V4L2_WB_FLUORESCENT:
        {
	     	rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x3406, tmp_reg|0x01, MSM_CAMERA_I2C_BYTE_DATA);
			if (rc < 0) return rc;
			rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x3400, 0x04, MSM_CAMERA_I2C_BYTE_DATA);	 
			if (rc < 0) return rc;
			rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x3401, 0x058, MSM_CAMERA_I2C_BYTE_DATA);	
			if (rc < 0) return rc;
			rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x3402, 0x04, MSM_CAMERA_I2C_BYTE_DATA);
			if (rc < 0) return rc;
			rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x3403, 0x00, MSM_CAMERA_I2C_BYTE_DATA);  
			if (rc < 0) return rc;
			rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x3404, 0x08, MSM_CAMERA_I2C_BYTE_DATA);
			if (rc < 0) return rc;
			rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x3405, 0x40, MSM_CAMERA_I2C_BYTE_DATA);
			if (rc < 0) return rc;
        }
        break; 

	    case MSM_V4L2_WB_DAYLIGHT:
        {
	     	rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x3406, tmp_reg|0x01, MSM_CAMERA_I2C_BYTE_DATA);
			if (rc < 0) return rc;
			rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x3400, 0x07, MSM_CAMERA_I2C_BYTE_DATA);	
			if (rc < 0) return rc;
			rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x3401, 0x02, MSM_CAMERA_I2C_BYTE_DATA);	
			if (rc < 0) return rc;
			rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x3402, 0x04, MSM_CAMERA_I2C_BYTE_DATA);
			if (rc < 0) return rc;
			rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x3403, 0x00, MSM_CAMERA_I2C_BYTE_DATA); 
			if (rc < 0) return rc;
			rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x3404, 0x05, MSM_CAMERA_I2C_BYTE_DATA);
			if (rc < 0) return rc;
			rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x3405, 0x15, MSM_CAMERA_I2C_BYTE_DATA); 
			if (rc < 0) return rc;
        }
        break; 

        case MSM_V4L2_WB_CLOUDY_DAYLIGHT:
        {
            rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x3406, tmp_reg|0x01, MSM_CAMERA_I2C_BYTE_DATA);
			if (rc < 0) return rc;
			rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x3400, 0x07, MSM_CAMERA_I2C_BYTE_DATA);	
			if (rc < 0) return rc;
			rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x3401, 0x88, MSM_CAMERA_I2C_BYTE_DATA);	
			if (rc < 0) return rc;
			rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x3402, 0x04, MSM_CAMERA_I2C_BYTE_DATA);
			if (rc < 0) return rc;
			rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x3403, 0x00, MSM_CAMERA_I2C_BYTE_DATA);
			if (rc < 0) return rc;
			rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x3404, 0x05, MSM_CAMERA_I2C_BYTE_DATA);
			if (rc < 0) return rc;
			rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x3405, 0x00, MSM_CAMERA_I2C_BYTE_DATA);
			if (rc < 0) return rc;
        }
        break;
        default:
        {
            pr_err("%s: parameter error!\n", __func__);
            return -EFAULT;
        }     
    }

	if( rc >=0 )
		zte_awb = value;

	return rc;
}


int ov9740_set_antibanding(struct msm_sensor_ctrl_t *s_ctrl,
		struct msm_sensor_v4l2_ctrl_info_t *ctrl_info, int value)
{
	long rc = 0;
    uint16_t tmp_reg = 0;
	pr_err("%s E,value=%d\n",__func__,value);

    if(zte_force_write)
	{
	}
	else if( zte_antibanding== value )
	{
	  	CDBG("%s: return quickly\n", __func__);
	    return rc;
	}

    switch( value)
    {
      case CAMERA_ANTIBANDING_60HZ:
	  {
		tmp_reg=0;
		rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client,0x3c00, &tmp_reg, MSM_CAMERA_I2C_BYTE_DATA);
		tmp_reg &= 0xfb; // 60Hz, bit[2] = 0
		rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x3c00, tmp_reg, MSM_CAMERA_I2C_BYTE_DATA);
		
		tmp_reg=0;
		rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client,0x3c01, &tmp_reg, MSM_CAMERA_I2C_BYTE_DATA);
		tmp_reg |= 0x80; // manual enable, bit[7] = 1
		rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x3c01, tmp_reg, MSM_CAMERA_I2C_BYTE_DATA);
		
		tmp_reg=0;
		rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client,0x3a00, &tmp_reg, MSM_CAMERA_I2C_BYTE_DATA);
		tmp_reg |= 0x20; // turn off band filter, bit[5] = 1
		rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x3a00, tmp_reg, MSM_CAMERA_I2C_BYTE_DATA);
	  	break;
	  }
	  case CAMERA_ANTIBANDING_50HZ:
	  {
		tmp_reg=0;
		rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client,0x3c00, &tmp_reg, MSM_CAMERA_I2C_BYTE_DATA);
		tmp_reg |= 0x04; // 50Hz, bit[2] = 1
		rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x3c00, tmp_reg, MSM_CAMERA_I2C_BYTE_DATA);

		tmp_reg=0;
		rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client,0x3c01, &tmp_reg, MSM_CAMERA_I2C_BYTE_DATA);
		tmp_reg |= 0x80; // manual enable, bit[7] = 1
		rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x3c01, tmp_reg, MSM_CAMERA_I2C_BYTE_DATA);
		
		tmp_reg=0;
		rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client,0x3a00, &tmp_reg, MSM_CAMERA_I2C_BYTE_DATA);
		tmp_reg |= 0x20; // turn off band filter, bit[5] = 1
		rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x3a00, tmp_reg, MSM_CAMERA_I2C_BYTE_DATA);
	  	break;
	  }
	  case CAMERA_ANTIBANDING_OFF:
	  case CAMERA_ANTIBANDING_AUTO:
	  {
		tmp_reg=0;	 
		rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client,0x3c01, &tmp_reg, MSM_CAMERA_I2C_BYTE_DATA);
		tmp_reg &= 0x7f; // auto enable, bit[7] = 0
		rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x3c01, tmp_reg, MSM_CAMERA_I2C_BYTE_DATA);

		tmp_reg=0;
		rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client,0x3a00, &tmp_reg, MSM_CAMERA_I2C_BYTE_DATA);
		tmp_reg |= 0x20; // turn on band filter, bit[5] = 1
		rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x3a00, tmp_reg, MSM_CAMERA_I2C_BYTE_DATA); 
	  	break;
	  }
	  default:
	  	pr_err("%s: invalid value\n", __func__);
	  	break;
    }


    if( rc == 0)
	  zte_antibanding= value;
	
	pr_err("%s X\n",__func__);
	return rc;
}


struct msm_sensor_v4l2_ctrl_info_t ov9740_v4l2_ctrl_info[] = {
	{
		.ctrl_id = V4L2_CID_SPECIAL_EFFECT,
		.min = MSM_V4L2_EFFECT_OFF,
		.max = MSM_V4L2_EFFECT_NEGATIVE,
		.step = 1,
		.s_v4l2_ctrl = ov9740_set_effect,
	},
	{
		.ctrl_id = V4L2_CID_ISO,
		.min = MSM_V4L2_ISO_AUTO,
		.max = MSM_V4L2_ISO_800,
		.step = 1,
		.s_v4l2_ctrl = ov9740_set_iso,
	},
	{
		.ctrl_id = V4L2_CID_BRIGHTNESS,
		.min = MSM_V4L2_BRIGHTNESS_L0,
		.max = MSM_V4L2_BRIGHTNESS_L4,
		.step = 1,
		.s_v4l2_ctrl = ov9740_set_brightness,
	},
	{
		.ctrl_id = V4L2_CID_AUTO_WHITE_BALANCE,
		.min = MSM_V4L2_WB_AUTO,
		.max = MSM_V4L2_WB_CLOUDY_DAYLIGHT,
		.step = 1,
		.s_v4l2_ctrl = ov9740_set_wb,
	},
	{
		.ctrl_id = V4L2_CID_POWER_LINE_FREQUENCY,
		.min = MSM_V4L2_POWER_LINE_OFF,
		.max = MSM_V4L2_POWER_LINE_AUTO,
		.step = 1,
		.s_v4l2_ctrl = ov9740_set_antibanding,
	},
};


int32_t msm_ov9740_sensor_setting(struct msm_sensor_ctrl_t *s_ctrl,
			int update_type, int res)
{
	int32_t rc = 0;

	pr_err("%s: E,update_type:%d,res:%d\n", __func__,update_type,res);
	//s_ctrl->func_tbl->sensor_stop_stream(s_ctrl);
	//msleep(30);
	if (update_type == MSM_SENSOR_REG_INIT) {

		s_ctrl->curr_csi_params = NULL;

	    if( is_init == 0)
	    {
	        pr_err("%s: MSM_SENSOR_REG_INIT\n", __func__);
	        rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x0103, 0x01, MSM_CAMERA_I2C_BYTE_DATA);
		    if (rc < 0) {
		    pr_err("%s: i2c write failed\n", __func__);		
		    return rc;
		    }
		    msleep(10);	
			
			rc=ov9740_i2c_write_table( s_ctrl, prev_snap_tbl, ARRAY_SIZE(prev_snap_tbl));
			if (rc < 0)
				return rc;
			is_init = 1;
			msleep(10);
	    }
		csi_config = 0;
		
	} 
	else if (update_type == MSM_SENSOR_UPDATE_PERIODIC) 
	{
		pr_err("%s: UPDATE_PERIODIC:%d\n", __func__,res);

		if( csi_config == 0)
		{
		    s_ctrl->func_tbl->sensor_stop_stream(s_ctrl);
	        msleep(30);
			
			s_ctrl->curr_csic_params = s_ctrl->csic_params[res];
			v4l2_subdev_notify(&s_ctrl->sensor_v4l2_subdev,
				NOTIFY_CSIC_CFG,
					s_ctrl->curr_csic_params);
			pr_err("msm_ov9740_sensor_setting CSI config is done\n");
			mb();

			v4l2_subdev_notify(&s_ctrl->sensor_v4l2_subdev,
			                   NOTIFY_PCLK_CHANGE,
			                   &s_ctrl->sensordata->pdata->ioclk.vfe_clk_rate);

			msleep(20);

			s_ctrl->func_tbl->sensor_start_stream(s_ctrl);

			zte_force_write = 1;
			ov9740_set_iso(s_ctrl,0,zte_iso);
			ov9740_set_brightness(s_ctrl,0,zte_brightness);
			ov9740_set_wb(s_ctrl,0,zte_awb);
			ov9740_set_antibanding(s_ctrl,0,zte_antibanding);
			zte_force_write = 0;
		}

	}
	 pr_err("%s: X\n", __func__);
	return rc;
}


static void ov9740_stop_stream(struct msm_sensor_ctrl_t *s_ctrl) 
{
    int rc = 0;
	rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x0100, 0x00, MSM_CAMERA_I2C_BYTE_DATA);
	if (rc < 0) 
	{
	   pr_err("%s: OV9740_MODE_SELECT=streaming failed\n", __func__);
	} 
}
static void ov9740_start_stream(struct msm_sensor_ctrl_t *s_ctrl) 
{

    int rc = 0;
	rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x0100, 0x01, MSM_CAMERA_I2C_BYTE_DATA);
	if (rc < 0) 
	{
	   pr_err("%s: OV9740_MODE_SELECT=streaming failed\n", __func__);
	} 
}

static struct msm_sensor_fn_t ov9740_func_tbl = {
	.sensor_start_stream = ov9740_start_stream,
	.sensor_stop_stream = ov9740_stop_stream,
	.sensor_csi_setting = msm_ov9740_sensor_setting,
	.sensor_set_sensor_mode = msm_sensor_set_sensor_mode,
	.sensor_mode_init = msm_sensor_mode_init,
	.sensor_get_output_info = msm_sensor_get_output_info,
	.sensor_config = msm_sensor_config,
	.sensor_power_up = msm_ov9740_sensor_power_up,
	.sensor_power_down = msm_ov9740_sensor_power_down,
	.sensor_get_csi_params = msm_sensor_get_csi_params,
};

static struct msm_sensor_reg_t ov9740_regs = {
	.default_data_type = MSM_CAMERA_I2C_BYTE_DATA,
	.init_settings = &ov9740_init_conf[0],
	.init_size = ARRAY_SIZE(ov9740_init_conf),
	.mode_settings = &ov9740_confs[0],
	.output_settings = &ov9740_dimensions[0],
	.num_conf = ARRAY_SIZE(ov9740_confs),
};

static struct msm_sensor_ctrl_t ov9740_s_ctrl = {
	.msm_sensor_reg = &ov9740_regs,
	.sensor_i2c_client = &ov9740_sensor_i2c_client,
	.sensor_i2c_addr = 0x20,
	.msm_sensor_v4l2_ctrl_info = ov9740_v4l2_ctrl_info,
	.num_v4l2_ctrl = ARRAY_SIZE(ov9740_v4l2_ctrl_info),
	.sensor_output_reg_addr = &ov9740_reg_addr,
	.sensor_id_info = &ov9740_id_info,
	.cam_mode = MSM_SENSOR_MODE_INVALID,
	.csic_params = &ov9740_csi_params_array[0],
	.msm_sensor_mutex = &ov9740_mut,
	.sensor_i2c_driver = &ov9740_i2c_driver,
	.sensor_v4l2_subdev_info = ov9740_subdev_info,
	.sensor_v4l2_subdev_info_size = ARRAY_SIZE(ov9740_subdev_info),
	.sensor_v4l2_subdev_ops = &ov9740_subdev_ops,
	.func_tbl = &ov9740_func_tbl,
	.clk_rate = MSM_SENSOR_MCLK_24HZ,
};

module_init(msm_sensor_init_module);
MODULE_DESCRIPTION("Aptina 1.26MP YUV sensor driver");
MODULE_LICENSE("GPL v2");
