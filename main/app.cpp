#include <iostream>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Clang-tidy forzará que esto sea PascalCase
class MiControlador {
public:
    void saludar() {
        std::cout << "¡Hola desde C++ en ESP32-S3 profesional!" << std::endl;
    }
};

extern "C" void app_main() {
    MiControlador controlador;

    while (true) {
        controlador.saludar();
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
