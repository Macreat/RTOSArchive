idf_component_register(SRCS "wifi_control.c" "wifi_app.c" "uart_control.c" "tasks.c" "rgb_led.c" "http_server.c" "configure_peripherals.c" "main.c"
                    INCLUDE_DIRS "."
                    EMBED_FILES webpage/app.css webpage/app.js webpage/favicon.ico webpage/index.html webpage/jquery-3.3.1.min.js)
