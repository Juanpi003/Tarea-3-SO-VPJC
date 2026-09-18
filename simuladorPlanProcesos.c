#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#define MAX_PROCESOS 200   // por si quieren simular mas de 100

typedef struct {
    int id;             // ID del proceso
    int arrival;                // Tiempo de llegada (t0, t1... tn)
    int burst;                  // Ráfaga de CPU original (tiempo cpu)
    int priority;               // Prioridad original (menor numero = mayor prioridad)
    int remaining;      // Para algoritmos con prioridades
    int start_time;     // Primer instante en que se ejecuta
    int finish_time;        // Momento en que termina
    int waiting_time;       // Tiempo total de espera
    int turnaround;     // finish_time - arrival
    int response_time;          // primer instante en que el proceso recibe CPU − arrival
} Process;

/*
 * Algorimto de planificacion por prioridades con ajuste dinamico 
 * no apropiativa. La prioridad sube (baja en uno su numero en prioridad) 
 * por cada X tiempo que el proceso lleva esperando 
 */
void simulate_priority_dynamic(Process *p, int n, int aging_step) {
    int completed = 0;
    int time = 0;
    // Arreglo que indica si cada proceso ya terminó.
    int *finished = calloc(n, sizeof(int));

    // Nos ayuda a evitar las divisiones entre cero
    if (aging_step <= 0) aging_step = 1; 

    printf("\n Inicio de la ejecucion \n");
    printf("Cada %d unidades esperando, se va a mejorar el proceso\n\n",
           aging_step, aging_step);
    printf("\n");

    // Ciclo que continúa hasta que todos los procesos hayan terminado.
    while (completed < n) {
        // Almacena la posición del proceso y se inicia en -1 para indicar que todavía no se ha encontrado ningún proceso listo
        int idx = -1;
        int best_dyn_priority = INT_MAX;

        /* 
        * Buscar entre todos los procesos que todavia no hayan terminado y los que
        * ya hayan llegado al sistema. Entre ellos se seleccionará el que tenga la 
        * mejor prioridad dinámica. 
        */
        for (int i = 0; i < n; i++) {
            if (!finished[i] && p[i].arrival <= time) {

                // Calcula cuánto tiempo lleva esperando el proceso
                int espera = time - p[i].arrival;

                // Calcula la prioridad dinámica en base al tiempo que se le asigno
                int dyn_priority = p[i].priority - (espera / aging_step);

                /* 
                * Determina si el proceso actual debe convertirse 
                * en el nuevo candidato  
                * Las condiciones de desempate son 
                * 1. Si todavía no existe candidato 
                * 2. Si tiene una mejor prioridad 
                * 3. Si tienen la misma prioridad, gana 
                * el que llegó primero.
                * 4. Si también llegaron al mismo tiempo, gana 
                * el proceso con el menor numero de proceso 
                */
                if (idx == -1 ||
                    dyn_priority < best_dyn_priority ||
                    (dyn_priority == best_dyn_priority && p[i].arrival < p[idx].arrival) ||
                    (dyn_priority == best_dyn_priority && p[i].arrival == p[idx].arrival && p[i].id < p[idx].id)) {
                    best_dyn_priority = dyn_priority;
                    idx = i;
                }
            }
        }

        /* 
        * Si idx continúa siendo -1 significa que no existe ningún 
        * proceso disponible para ejecutarse en este instante 
        * Esto puede llegar a ocurrir cuando todavía no ha 
        * llegado ningún proceso
        */
        if (idx == -1) {

            // Se busca el siguiente tiempo de llegada
            int next_arrival = INT_MAX;
            for (int i = 0; i < n; i++) {
                if (!finished[i] && p[i].arrival < next_arrival)
                    next_arrival = p[i].arrival;
            }
            time = next_arrival;
            continue;
        }

        if (p[idx].start_time == -1) {
            p[idx].start_time = time;
            p[idx].response_time = time - p[idx].arrival;
        }

        p[idx].waiting_time = time - p[idx].arrival; 
        int t_inicio = time;
        time += p[idx].burst;
        p[idx].finish_time = time;
        p[idx].turnaround = p[idx].finish_time - p[idx].arrival;
        p[idx].remaining = 0;

        finished[idx] = 1;
        completed++;

        printf("t=%3d -> P%-3d ejecuta %2d unidades (prioridad actual:%d) -> termina en t=%d\n",
               t_inicio, p[idx].id, p[idx].burst, best_dyn_priority, p[idx].finish_time);
    }

    free(finished);

    // Se calcula los  promedios promedios
    double avg_wait = 0, avg_turnaround = 0, avg_response = 0;
    for (int i = 0; i < n; i++) {
        avg_wait += p[i].waiting_time;
        avg_turnaround += p[i].turnaround;
        avg_response += p[i].response_time;
    }
    avg_wait /= n;
    avg_turnaround /= n;
    avg_response /= n;


    // ESTO NO ESTA BIEN, LOS ULTIMOS 3 PARAMETROS NO ME CUADRAN
    printf("\nResultados finales:\n");
    printf("%-4s %-8s %-6s %-6s %-6s %-8s %-6s %-6s %-6s\n",
           "Proceso", "Llegada", "Rafaga", "Prioridad", "Inicio", "Final", "T. de Espera", "T. de Retorno", "T. de Respuesta");
    for (int i = 0; i < n; i++) {
        printf("%-4d %-8d %-6d %-6d %-6d %-8d %-6d %-6d %-6d\n",
               p[i].id, p[i].arrival, p[i].burst, p[i].priority,
               p[i].start_time, p[i].finish_time, p[i].waiting_time,
               p[i].turnaround, p[i].response_time);
    }

    printf("\nPromedios:\n");
    printf("Tiempo de espera promedio:      %.2f\n", avg_wait);
    printf("Tiempo de retorno promedio:  %.2f\n", avg_turnaround);
    printf("Tiempo de respuesta promedio:   %.2f\n", avg_response);
}

int main(int argc, char *argv[]) {
    Process procesos[MAX_PROCESOS];
    int n = 0;

    /*
    * El programa recibe 2 argumentos de entrada:
    * 1. El .txt con el numero de procesos, llegada, rafaga y prioridad
    * 2. El tiempo que debe esperar un proceso para que mejore su prioridad
    */
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <archivo_procesos.txt> <aging_step (3-10)>\n", argv[0]);
        fprintf(stderr, "Ejemplo: %s procesos.txt 5\n", argv[0]);
        return 1;
    }

    const char *ruta_archivo = argv[1];
    char *endptr;
    long aging_step_long = strtol(argv[2], &endptr, 10);

    // Valida que el segundo argumento sea realmente un numero entero
    if (*endptr != '\0' || argv[2][0] == '\0') {
        fprintf(stderr, "Error: '%s' no es un numero entero valido para aging_step.\n", argv[2]);
        return 1;
    }

    // Valida que el nuemero este en el rango (entre 3 y 10)
    if (aging_step_long < 3 || aging_step_long > 10) {
        fprintf(stderr, "Error: aging_step debe estar entre 3 y 10 (recibido: %ld).\n", aging_step_long);
        return 1;
    }

    int aging_step = (int) aging_step_long;

    FILE *fp = fopen(ruta_archivo, "r");
    if (fp == NULL) {
        fprintf(stderr, "Error: no se pudo abrir el archivo '%s'.\n", ruta_archivo);
        return 1;
    }

    while (fscanf(fp, "%d %d %d %d",
                 &procesos[n].id,
                 &procesos[n].arrival,
                 &procesos[n].burst,
                 &procesos[n].priority) == 4) {

        procesos[n].remaining    = procesos[n].burst;
        procesos[n].start_time   = -1;
        procesos[n].finish_time  = 0;
        procesos[n].waiting_time = 0;
        procesos[n].turnaround   = 0;
        procesos[n].response_time= -1;
        n++;

        if (n >= MAX_PROCESOS) {
            fprintf(stderr, "Se alcanzó el límite MAX_PROCESOS=%d\n", MAX_PROCESOS);
            break;
        }
    }
    fclose(fp);

    if (n == 0) {
        fprintf(stderr, "Error: no se leyó ningún proceso valido desde '%s'.\n", ruta_archivo);
        return 1;
    }

    printf("Se recibieron %d procesos desde '%s':\n", n, ruta_archivo);
    for (int i = 0; i < n; i++) {
        printf("P%3d: Llegada=%3d Rafaga=%2d Prioridad=%d\n",
               procesos[i].id,
               procesos[i].arrival,
               procesos[i].burst,
               procesos[i].priority);
    }

    simulate_priority_dynamic(procesos, n, aging_step);

    return 0;
}