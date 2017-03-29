

static int iic_xxx_xfer(struct i2c_adapter *i2c_adap, struct i2c_msg msgs[], int num)
{
	/* routine for controlling I2C bus */
	return ret;
}

static struct i2c_algorithm iic_xxx_algo = {
	.name = "Sample IIC algorithm",
	.id = I2C_ALGO_SAMPLE,
	.master_xfer = iic_xxx_xfer,
};

static struct i2c_adapter iic_xxx_adapter = {
	.owner = THIS_MODULE,
	.name = "Sample IIC adapter",
	.algo = &iic_xxx_algo,
};

static int __init iic_xxx_init(void)
{
	/* initialize hardware, allocate memory, register interrupt */
	iic_xxx_hw_init();
	/* register adapter */
	i2c_add_adapter( &iic_xxx_adapter );
	return 0;
}

static void iic_xxx_exit(void)
{
	/* remove adapter */
	i2c_del_adapter(&iic_xxx_adapter);
	/* remove memory and interrupt */
	iic_xxx_hw_release ();
}

module_init(iic_xxx_init);
module_exit(iic_xxx_exit);

MODULE_AUTHOR("Andy");
MODULE_DESCRIPTION("I2C-Bus adapter sample routines");
MODULE_LICENSE("GPL");


