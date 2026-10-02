Para abordar un Trabajo Práctico de esta naturaleza, lo ideal es dividir el proyecto en tres grandes fases: la arquitectura de portabilidad (Porting), el diseño lógico (Modelado/Statecharts) y finalmente la implementación (C Coding y System Setup).

Aquí tienes una guía estructurada paso a paso para desarrollar tu Trabajo Práctico.

## 1\. Porting C Code: Capa de Abstracción de Hardware (HAL)

El objetivo de hacer "porting" es que el código del LCD no dependa de un microcontrolador específico (como STM32, PIC o Arduino). Para lograrlo, debes separar la lógica del display de las instrucciones directas del hardware.

Debes dividir tu código en dos capas:

| **Capa** | **Responsabilidad** | **Ejemplos de Funciones** |
| --- | --- | --- |
| **Hardware-Dependent (HAL)** | Controla los pines físicos, timers y buses (I2C/SPI). Es lo único que cambia si cambias de placa. | `HAL_GPIO_WritePin()`, `System_Delay_ms()`, `SPI_Transmit()` |
| **Hardware-Independent (Core)** | Conoce los comandos del LCD (ej. HD44780 o ILI9341) pero no sabe qué microcontrolador se usa. | `LCD_Init()`, `LCD_SetCursor()`, `LCD_PrintString()` |

**Ejemplo de abstracción en C:**

En lugar de escribir `PORTB |= (1<<2)` dentro de tu código del LCD, debes crear una función envoltorio (wrapper):

```c
// Lógica dependiente (Se implementa según el microcontrolador actual)
void LCD_IO_WritePin(uint8_t pin, uint8_t state) {
    // Ejemplo para STM32:
    // HAL_GPIO_WritePin(GPIOA, pin, state ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

// Lógica independiente (El código que portas)
void LCD_SendCommand(uint8_t cmd) {
    LCD_IO_WritePin(PIN_RS, 0); // RS = 0 para comandos
    // ... lógica para enviar los bits ...
}
```

## 2\. Modelado de Estados (Statechart)

En sistemas embebidos, no es recomendable usar retardos bloqueantes (como `delay(1000)`) para actualizar el LCD, ya que el sistema dejaría de atender otras tareas. Para evitarlo, modelamos el sistema con un diagrama de estados (Statechart).

Un modelo clásico para un sistema con display suele tener los siguientes estados:

1.  **`STATE_HW_SETUP`**: Inicializa los relojes del sistema (System Clock) y los periféricos (GPIO, SPI/I2C, Timers).
    
2.  **`STATE_LCD_INIT`**: Ejecuta la secuencia de inicialización propia del controlador del LCD.
    
3.  **`STATE_IDLE`**: El sistema está en reposo, esperando que ocurra un evento (presionar un botón, llegada de un dato por puerto serie, o el desbordamiento de un timer).
    
4.  **`STATE_UPDATE_DISPLAY`**: Se procesa la nueva información y se envía a la pantalla.
    
5.  **`STATE_ERROR`**: Si falla la comunicación I2C/SPI con el display o un sensor.
    

## 3\. C Coding y System Setup

La implementación en C de un Statechart se realiza típicamente utilizando una máquina de estados basada en la estructura `switch-case` dentro de un bucle infinito (Super-loop).

```c
#include <stdint.h>
#include <stdbool.h>

// 1. Definición de Estados
typedef enum {
    STATE_HW_SETUP,
    STATE_LCD_INIT,
    STATE_IDLE,
    STATE_UPDATE_DISPLAY,
    STATE_ERROR
} SystemState_t;

SystemState_t currentState = STATE_HW_SETUP;

int main(void) {
    // Super-loop
    while(1) {
        switch (currentState) {
            
            case STATE_HW_SETUP:
                // Configuración de relojes y pines (System Setup)
                MCU_Clock_Init();
                GPIO_Init();
                currentState = STATE_LCD_INIT;
                break;
                
            case STATE_LCD_INIT:
                if (LCD_Init() == SUCCESS) {
                    currentState = STATE_IDLE;
                } else {
                    currentState = STATE_ERROR;
                }
                break;
                
            case STATE_IDLE:
                // Esperar eventos de forma no bloqueante
                if (Timer_Expired()) {
                    currentState = STATE_UPDATE_DISPLAY;
                }
                if (Button_Pressed()) {
                    // Cambiar de menú, por ejemplo
                    currentState = STATE_UPDATE_DISPLAY;
                }
                break;
                
            case STATE_UPDATE_DISPLAY:
                LCD_Clear();
                LCD_PrintString("Menu Principal");
                currentState = STATE_IDLE; // Retornar a reposo
                break;
                
            case STATE_ERROR:
                // Manejo de fallos (ej. parpadear un LED de error)
                Error_Handler();
                break;
        }
    }
}
```





El conjunto de archivos proporcionado conforma un sistema embebido basado en arquitectura "Bare Metal" (sin sistema operativo). El programa implementa un **planificador cooperativo (scheduler) disparado por eventos de tiempo (Time-Triggered)**, utilizando el temporizador de hardware SysTick para ejecutar tareas periódicas sin bloquear la ejecución.

A continuación se detalla el análisis y funcionamiento de los módulos, seguido del comportamiento de las máquinas de estados.

### 1. Análisis y función de los archivos

*   **`app.c`**: Es el núcleo del planificador de tareas. Contiene el bucle principal dentro de `app_update()`, que se apoya en el contador `g_app_tick_cnt` (incrementado por el SysTick cada 1 ms) para decidir si es momento de ejecutar las rutinas de "update" de las tareas suscritas (en este caso, la tarea `test` y la tarea `display`). Además, incluye funciones de diagnóstico para perfilar el sistema, calculando el tiempo en el mejor y peor caso de ejecución (BCET, WCET) de cada tarea.
*   **`app_it.c`**: Maneja las interrupciones del microcontrolador. Su función principal es `HAL_SYSTICK_Callback()`, la cual incrementa la variable `g_app_tick_cnt` cada vez que el temporizador de hardware (SysTick) lanza una interrupción, generando así la base de tiempos del sistema.
*   **`systick.c`**: Implementa una función de retardo bloqueante precisa en microsegundos (`systick_delay_us()`). Lo logra leyendo los registros a bajo nivel del contador del SysTick directamente, lo que es útil en rutinas que necesitan pausas muy cortas y exactas.
*   **`task_test_attribute.h` / `task_test.c`**: Definen e implementan una "Tarea de Prueba". El archivo de cabecera (`.h`) define su estructura de datos con variables como el contador global y los "ticks" restantes. En el archivo `.c`, la tarea se encarga de temporizar eventos periódicos y de enviarle actualizaciones de texto a la tarea que maneja el Display.
*   **`task_display_attribute.h` / `task_display_interface.c` / `task_display.c`**: Componen la lógica de la "Tarea de Display".
    *   El archivo **`.h`** estructura una memoria RAM virtual (`ddram`) donde se almacenan las líneas a mostrar, además de declarar las banderas y eventos.
    *   El **`_interface.c`** provee la función `put_event_task_display()`, un punto de acceso (API) para que cualquier otra tarea escriba en la memoria de pantalla virtual de forma segura y envíe una señal o bandera (`flag=true` y `event=EV_DSP_UPDATE`) de que hay datos nuevos por mostrar.
    *   El **`.c`** contiene la tarea periódica que revisa esa memoria y, si hay cambios (flag activada), vuelca la información hacia el driver de la pantalla física.
*   **`display.h` / `display.c`**: Son el controlador (driver) de bajo nivel del módulo de pantalla LCD físico (típicamente controlador HD44780). Maneja la manipulación directa de pines (GPIO) para señales de Control (RS, EN) y Datos (D4-D7). Provee funciones básicas como la inicialización en modos de 4 bits, mover el cursor de la pantalla (`displayCharPositionWrite`) y enviar un string (`displayStringWrite`).

---

### 2. Comportamiento de las Máquinas de Estado (Statecharts)

Las dos tareas principales están diseñadas internamente como Máquinas de Estados (Statecharts) para evitar ejecutar código bloqueante y permitir que el planificador principal siga llamando a otras tareas.

#### `void task_test_statechart(void)`
**Comportamiento:** Funciona como un temporizador o cronómetro que ejecuta una acción cada 1 segundo.
1.  **Conteo constante:** Cada vez que el planificador llama a esta función (1 vez por milisegundo), la tarea incrementa su contador absoluto de ciclos (`p_task_test_dta->counter++`).
2.  **Temporización:** Posee una cuenta regresiva (`tick`), que inicia en 1000. Si este `tick` es mayor a 0, simplemente lo decrementa y la función termina su ejecución.
3.  **Disparo:** Cuando `tick` llega a 0 (lo que significa que pasó 1 segundo entero), reinicia el temporizador a 1000. Acto seguido, calcula cuántos segundos han pasado desde el inicio del programa (dividiendo `counter / 1000`), convierte este número a cadena de texto, y utiliza la interfaz `put_event_task_display()` para ordenarle al Display que imprima un cartel en la fila 1: "Test Nro: X".

#### `void task_display_statechart(void)`
**Comportamiento:** Máquina de estados finitos que funciona como un consumidor pasivo. Espera a que alguna otra tarea (como `task_test`) le pida dibujar algo. Se mueve principalmente entre dos estados:
1.  **Estado `ST_DSP_IDLE` (Reposo):** Es el estado por defecto. La máquina no hace nada y no gasta tiempo de CPU a menos que se cumplan las condiciones para actualizar. Verifica si la bandera `flag` está activada (`true`) y si el evento presente es `EV_DSP_UPDATE`. Si se cumplen, cambia su estado futuro a `ST_DSP_UPDATE`.
2.  **Estado `ST_DSP_UPDATE` (Actualización):** Al entrar a este estado en el siguiente ciclo, la máquina valida de nuevo el evento. Al validar, hace lo siguiente:
    *   Baja la bandera para indicar que el evento está siendo procesado (`flag = false`).
    *   Llama al driver del hardware físico para reposicionar el cursor en la Fila 0, Columna 0 y escupe el primer búfer de la memoria RAM virtual (`ddram[0]`).
    *   Mueve el cursor a la Fila 1, Columna 0 y vuelca el segundo renglón (`ddram[1]`).
    *   Finalmente, devuelve su estado a `ST_DSP_IDLE` para quedar lista ante futuros avisos.

Este paradigma de arquitectura separa limpiamente a quien produce la información de quién consume el tiempo necesario para comunicarse mediante los GPIO con el LCD.
