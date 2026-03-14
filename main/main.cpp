#include <iostream>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Clang-tidy forzará que esto sea PascalCase
class MiControlador {
   public:
    void saludar() {
        std::cout << "¡♡ Te quiero infinito mi amor ♡!" << std::endl;
        //printf("¡♡ Te quiero infinito mi amor ♡!");
    }
};

extern "C" void app_main() {
    MiControlador controlador;

    while (true) {
        controlador.saludar();
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
