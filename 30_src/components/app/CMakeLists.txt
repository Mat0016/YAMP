idf_component_register(
    SRCS "app_main.c" "app_power_monitor.c" "app_power_monitor_logic.c" "app_ihm.c"
    INCLUDE_DIRS "include"
    REQUIRES midlevel common
)
