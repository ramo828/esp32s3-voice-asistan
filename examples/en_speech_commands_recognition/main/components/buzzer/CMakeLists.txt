idf_component_register(
    SRCS "src/buzzer.c"
    INCLUDE_DIRS "include"
    REQUIRES driver freertos esp_timer
)
