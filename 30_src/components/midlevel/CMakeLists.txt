idf_component_register(
    SRCS "drv_i2c.c" "drv_spi.c" "drv_ina237.c" "drv_ina237_convert.c" "drv_ihm.c"
    INCLUDE_DIRS "include"
    REQUIRES driver common
)
