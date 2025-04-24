set(srcs
    main.c
    speech_commands_action.c
    )

set(requires
    esp-sr
    led_strip
    hardware_driver
    buzzer          # ← buzzer bileşenini burada ekledik
    )

idf_component_register(SRCS ${srcs}
                       INCLUDE_DIRS include
                       REQUIRES ${requires}
                       )

component_compile_options(-w)