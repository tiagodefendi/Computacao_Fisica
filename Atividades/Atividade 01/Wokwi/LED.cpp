void setup()
{
    REG_SET_BIT(GPIO_ENABLE_REG, BIT14);
    REG_SET_BIT(GPIO_ENABLE_REG, BIT15);
}

void loop()
{
    REG_SET_BIT(GPIO_OUT_W1TS_REG, BIT14);
    delay(50);
    REG_SET_BIT(GPIO_OUT_W1TC_REG, BIT14);
    REG_SET_BIT(GPIO_OUT_W1TS_REG, BIT15);
    delay(50);
    REG_SET_BIT(GPIO_OUT_W1TC_REG, BIT15);
    delay(450);
}

// github.com/tiagodefendi
