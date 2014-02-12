
#include <asm/mach/arch.h>

#include <linux/clk.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/regulator/fixed.h>
#include <linux/regulator/machine.h>

#include <mach/common.h>
#include <mach/devices-common.h>
#include <mach/gpio.h>
#include <mach/iomux-mx6dl.h>
#include <mach/iomux-v3.h>
#include <mach/mx6.h>

#include <linux/edm.h>

/****************************************************************************
 *                                                                          
 * SGTL5000 Audio Codec
 *                                                                          
 ****************************************************************************/

static struct regulator_consumer_supply wizard_sgtl5000_consumer_vdda = {
	.supply = "VDDA",
	.dev_name = "0-000a", /* Modified load time */
};

/* ------------------------------------------------------------------------ */

static struct regulator_consumer_supply wizard_sgtl5000_consumer_vddio = {
	.supply = "VDDIO",
	.dev_name = "0-000a", /* Modified load time */
};

/* ------------------------------------------------------------------------ */

static struct regulator_init_data wizard_sgtl5000_vdda_reg_initdata = {
	.num_consumer_supplies = 1,
	.consumer_supplies = &wizard_sgtl5000_consumer_vdda,
};

/* ------------------------------------------------------------------------ */

static struct regulator_init_data wizard_sgtl5000_vddio_reg_initdata = {
	.num_consumer_supplies = 1,
	.consumer_supplies = &wizard_sgtl5000_consumer_vddio,
};

/* ------------------------------------------------------------------------ */

static struct fixed_voltage_config wizard_sgtl5000_vdda_reg_config = {
	.supply_name		= "VDDA",
	.microvolts		= 2500000,
	.gpio			= -1,
	.init_data		= &wizard_sgtl5000_vdda_reg_initdata,
};

/* ------------------------------------------------------------------------ */

static struct fixed_voltage_config wizard_sgtl5000_vddio_reg_config = {
	.supply_name		= "VDDIO",
	.microvolts		= 3300000,
	.gpio			= -1,
	.init_data		= &wizard_sgtl5000_vddio_reg_initdata,
};

/* ------------------------------------------------------------------------ */

static struct platform_device wizard_sgtl5000_vdda_reg_devices = {
	.name	= "reg-fixed-voltage",
	.id	= 0,
	.dev	= {
		.platform_data = &wizard_sgtl5000_vdda_reg_config,
	},
};

/* ------------------------------------------------------------------------ */

static struct platform_device wizard_sgtl5000_vddio_reg_devices = {
	.name	= "reg-fixed-voltage",
	.id	= 1,
	.dev	= {
		.platform_data = &wizard_sgtl5000_vddio_reg_config,
	},
};

/* ------------------------------------------------------------------------ */

static struct platform_device wizard_audio_device = {
	.name = "imx-sgtl5000",
};

/* ------------------------------------------------------------------------ */

static const struct i2c_board_info wizard_sgtl5000_i2c_data __initdata = {
        I2C_BOARD_INFO("sgtl5000", 0x0a)
};

/* ------------------------------------------------------------------------ */

static char wizard_sgtl5000_dev_name[8] = "0-000a";

static __init int wizard_init_sgtl5000(void) {
	int i2c_bus = 1; /* TODO: get this from the module. */

        wizard_sgtl5000_dev_name[0] = '0' + i2c_bus;
	wizard_sgtl5000_consumer_vdda.dev_name = wizard_sgtl5000_dev_name;
	wizard_sgtl5000_consumer_vddio.dev_name = wizard_sgtl5000_dev_name;
        
        wizard_audio_device.dev.platform_data = 
        (struct mxc_audio_platform_data *)edm_analog_audio_platform_data;
        platform_device_register(&wizard_audio_device);
        
	i2c_register_board_info(i2c_bus, &wizard_sgtl5000_i2c_data, 1);
	platform_device_register(&wizard_sgtl5000_vdda_reg_devices);
	platform_device_register(&wizard_sgtl5000_vddio_reg_devices);
        return 0;
}


/****************************************************************************
 *                                                                          
 * ADS7846 Touchscreen
 *                                                                          
 ****************************************************************************/

#include <linux/spi/ads7846.h>
#include <linux/spi/spi.h>

static int wizard_tsc2046_gpio_index = 3;
static int wizard_ads7845_gpio_index = 2;

int wizard_get_tsc2046_pendown_state(void) {
	return !gpio_get_value(edm_external_gpio[wizard_tsc2046_gpio_index]);
}

static struct ads7846_platform_data wizard_tsc2046_config = {
        .x_max              	= 0x0fff,
        .y_max                  = 0x0fff,
        .pressure_max           = 255,
        .get_pendown_state      = wizard_get_tsc2046_pendown_state,
        .keep_vref_on           = 1,
        .wakeup			= true,
};

static struct spi_board_info wizard_tsc2046_spi_data  = {
        .modalias		= "ads7846",
        .bus_num		= 0,
        .chip_select		= 0,
        .max_speed_hz		= 1500000,
        .irq			= -EINVAL, /* Set programmatically */
        .platform_data		= &wizard_tsc2046_config,
};

static struct spi_board_info wizard_spidev_data  = {
        .modalias		= "spidev",
        .bus_num		= 0,
        .chip_select		= 0,
        .max_speed_hz		= 300000,
};


/* ------------------------------------------------------------------------ */

int wizard_get_ads7845_pendown_state(void) {
	return !gpio_get_value(edm_external_gpio[wizard_ads7845_gpio_index]);
}

static struct ads7846_platform_data wizard_ads7845_config = {
        .x_max              	= 0x0fff,
        .y_max                  = 0x0fff,
        .pressure_max           = 255,
        .get_pendown_state      = wizard_get_ads7845_pendown_state,
        .keep_vref_on           = 1,
        .wakeup			= true,
};

static struct spi_board_info wizard_ads7845_spi_data  = {
        .modalias		= "ads7846",
        .bus_num		= 1,
        .chip_select		= 0,
        .max_speed_hz		= 1500000,
        .irq			= -EINVAL, /* Set programmatically */
        .platform_data		= &wizard_ads7845_config,
};

/* ------------------------------------------------------------------------ */

void __init wizard_init_ts(void) {
        gpio_request(edm_external_gpio[wizard_tsc2046_gpio_index], "tsc2046 irq");
        gpio_request(edm_external_gpio[wizard_ads7845_gpio_index], "ads7845 irq");
        wizard_tsc2046_spi_data.irq = gpio_to_irq(edm_external_gpio[wizard_tsc2046_gpio_index]);
	spi_register_board_info(&wizard_tsc2046_spi_data, 1);
        wizard_ads7845_spi_data.irq = gpio_to_irq(edm_external_gpio[wizard_ads7845_gpio_index]);
	spi_register_board_info(&wizard_ads7845_spi_data, 2);
}


/****************************************************************************
 *                                                                          
 * main-function for wizardboard
 *                                                                          
 ****************************************************************************/

static __init int wizard_init(void) {
        wizard_init_sgtl5000();
        wizard_init_ts();
        return 0;
}
arch_initcall(wizard_init);

static __exit void wizard_exit(void) {
	/* Actually, this cannot be unloaded. Or loaded as a module..? */
} 
module_exit(wizard_exit);

MODULE_DESCRIPTION("Wizardboard driver");
MODULE_AUTHOR("Tapani <tapani@technexion.com>");
MODULE_LICENSE("GPL");
