add_executable(main
        hc06.c
        hc05.c
        main.c
)

target_link_libraries(main pico_stdlib hardware_adc oled1_lib freertos pico_multicore hardware_pio )
pico_add_extra_outputs(main)
