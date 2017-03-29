


static struct i2c_device_id foo_idtable[] = {
	{ "foo", my_id_for_foo },
	{ "bar", my_id_for_bar },
	{ }
};

MODULE_DEVICE_TABLE(i2c, foo_idtable);

static struct i2c_driver foo_driver = {
	.driver = {
		.name	= "foo",
	},

	.id_table	= foo_ids,
	.probe		= foo_probe,
	.remove		= foo_remove,
	/* if device autodetection is needed: */
	.class		= I2C_CLASS_SOMETHING,
	.detect		= foo_detect,
	.address_data	= &addr_data,

	.shutdown	= foo_shutdown,	/* optional */
	.suspend	= foo_suspend,	/* optional */
	.resume		= foo_resume,	/* optional */
	.command	= foo_command,	/* optional, deprecated */
}


static int __init foo_init(void)
{
	return i2c_add_driver(&foo_driver);
}

static void __exit foo_cleanup(void)
{
	i2c_del_driver(&foo_driver);
}

module_init(foo_init);
module_exit(foo_cleanup);

/* Substitute your own name and email address */
MODULE_AUTHOR("Frodo Looijaard <frodol@dds.nl>"
MODULE_DESCRIPTION("Driver for Barf Inc. Foo I2C devices");

/* a few non-GPL license types are also allowed */
MODULE_LICENSE("GPL");

