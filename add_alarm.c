//Inclui as bibliotecas necessárias
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/timer.h"
#include "hardware/gpio.h"

// Definição dos pinos dos LEDs e do botão
#define LED_RED 13
#define LED_BLUE 12
#define LED_GREEN 11
#define BUTTON 5

// Flag para evitar que os LEDs sejam reativados antes do ciclo finalizar
volatile bool leds_active = false;

// Variável para armazenar o tempo da última ativação do botão (para debounce)
static volatile uint32_t last_time = 0;
uint cont = 0;

// Callback para desligar o terceiro LED e liberar a ativação do botão
uint64_t turn_off_Callback3(alarm_id_t id, void *user_data) {
    gpio_put(LED_GREEN, false); // Desliga o LED verde
    leds_active = false; // Permite que o botão possa ativar os LEDs novamente
    return 0;
}

// Callback para desligar o segundo LED e iniciar o temporizador para o próximo
uint64_t turn_off_Callback2(alarm_id_t id, void *user_data) {
    gpio_put(LED_RED, false); // Desliga o LED vermelho
    add_alarm_in_ms(3000, turn_off_Callback3, NULL, false); // Chama o próximo callback após 3 segundos
    return 0;
}

// Callback para desligar o primeiro LED e iniciar o temporizador para o próximo
uint64_t turn_off_Callback1(alarm_id_t id, void *user_data) {
    gpio_put(LED_BLUE, false); // Desliga o LED azul
    add_alarm_in_ms(3000, turn_off_Callback2, NULL, false); // Chama o próximo callback após 3 segundos
    return 0;
}

// Interrupção do botão
void button_callback(uint gpio, uint32_t events) {
    uint32_t current_time = to_us_since_boot(get_absolute_time()); // Obtém o tempo atual em microssegundos

    // Verifica se os LEDs estão desligados e se passou tempo suficiente desde o último acionamento (evita debounce)
    if (!leds_active && (current_time - last_time > 200000)) {
        leds_active = true; // Define a flag para impedir novas ativações enquanto os LEDs estiverem ligados
        gpio_put(LED_RED, true); // Liga o LED vermelho
        gpio_put(LED_BLUE, true); // Liga o LED azul
        gpio_put(LED_GREEN, true); // Liga o LED verde
        add_alarm_in_ms(3000, turn_off_Callback1, NULL, false); // Inicia o temporizador para desligar o primeiro LED após 3 segundos
    }
}

int main() {
    stdio_init_all(); // Inicializa a comunicação padrão

    // Configuração dos LEDs
    gpio_init(LED_RED);
    gpio_set_dir(LED_RED, GPIO_OUT);
    gpio_put(LED_RED, false);

    gpio_init(LED_BLUE);
    gpio_set_dir(LED_BLUE, GPIO_OUT);
    gpio_put(LED_BLUE, false);

    gpio_init(LED_GREEN);
    gpio_set_dir(LED_GREEN, GPIO_OUT);
    gpio_put(LED_GREEN, false);

    // Configuração do botão
    gpio_init(BUTTON);
    gpio_set_dir(BUTTON, GPIO_IN);
    gpio_pull_up(BUTTON); // Habilita pull-up interno para o botão

    // Configuração da interrupção do botão para detecção de borda de descida (quando pressionado)
    gpio_set_irq_enabled_with_callback(BUTTON, GPIO_IRQ_EDGE_FALL, true, &button_callback);

    // Loop infinito aguardando interrupções e temporizadores
    while (1) {
        tight_loop_contents(); // Mantém o processador em estado de espera eficiente
    }
}
