#include <iostream>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
using namespace std;

const char* productos[] = {"Pata", "Respaldo", "Asiento", "Pata", "Pata"};
const int numProductos = 5;
const int MAX_BUFFER = 5;
const int MAX_SILLAS = 3;
int buffer[MAX_BUFFER];
int in = 0;
int out = 0;
int sillasProducidas = 0;
int piezasRestantes[3] = {0, 0, 0};  // Contadores de piezas sobrantes: [Patas, Respaldos, Asientos]
bool seguirProduciendo = true;  // Flag global para detener a los productores

// Semáforos y mutex
sem_t vacios;
sem_t llenos;
pthread_mutex_t mutex;

// Función simulación de un productor (fabrica una pieza de silla)
void* productor(void* arg) {
    int id = *(int*)arg;
    int piezaId;

    while (true) {
        // Verifica si se debe seguir produciendo
        pthread_mutex_lock(&mutex);
        if (!seguirProduciendo) {
            pthread_mutex_unlock(&mutex);
            break;  // Finaliza el hilo del productor si ya no se necesita más producción
        }
        pthread_mutex_unlock(&mutex);

        piezaId = rand() % numProductos;  // Seleccionar una pieza al azar
        sem_wait(&vacios);  // Espera hasta que haya espacio en el buffer

        pthread_mutex_lock(&mutex);  // Protege el acceso al buffer
        if (!seguirProduciendo) {  // Verificar nuevamente antes de colocar la pieza
            pthread_mutex_unlock(&mutex);
            sem_post(&vacios);  // Liberar semáforo si se sale del ciclo
            break;
        }

        buffer[in] = piezaId;  // Añade la pieza al buffer
        cout << "Productor " << id << " ha fabricado la pieza " << productos[piezaId]
             << " y la colocó en la posición " << in << endl;
        in = (in + 1) % MAX_BUFFER;  // Avanza el índice circular del buffer

        pthread_mutex_unlock(&mutex);
        sem_post(&llenos);  // Incrementa el número de productos disponibles
        sleep(1);  // Simula el tiempo de fabricación
    }
    return NULL;
}

// Función simulación de un consumidor (ensambla una silla)
void* consumidor(void* arg) {
    int id = *(int*)arg;
    int piezaId;
    int patas = 0, respaldos = 0, asientos = 0;

    while (true) {
        sem_wait(&llenos);  // Espera hasta que existan productos disponibles
        pthread_mutex_lock(&mutex);  // Protege el acceso al buffer

        if (sillasProducidas >= MAX_SILLAS) {  // Detiene a los consumidores cuando se alcanzan las sillas
            pthread_mutex_unlock(&mutex);
            sem_post(&llenos);  // Liberar semáforo si ya se alcanzó el límite
            break;
        }

        piezaId = buffer[out];  // Retirar una pieza del buffer
        cout << "Consumidor " << id << " ha retirado la pieza " << productos[piezaId]
             << " de la posición " << out << endl;
        out = (out + 1) % MAX_BUFFER;  // Avanza en el índice circular del buffer

        // Contabilizamos las piezas que el consumidor retira
        if (piezaId == 0 || piezaId == 3 || piezaId == 4) patas++;
        else if (piezaId == 1) respaldos++;
        else if (piezaId == 2) asientos++;

        // Si se completan las piezas para ensamblar una silla
        if (patas >= 4 && respaldos >= 1 && asientos >= 1) {
            patas -= 4;
            respaldos--;
            asientos--;
            sillasProducidas++;  // Incrementa el contador de sillas ensambladas
            cout << "Consumidor " << id << " ha ensamblado una silla completa. "
                 << "Sillas ensambladas: " << sillasProducidas << "/" << MAX_SILLAS << endl;
        }

        pthread_mutex_unlock(&mutex);
        sem_post(&vacios);  // Incrementa el número de espacios vacíos
        sleep(2);  // Simula el tiempo de ensamblaje

        if (sillasProducidas >= MAX_SILLAS) {  // Cuando se alcanzan las sillas, detener la producción
            pthread_mutex_lock(&mutex);
            seguirProduciendo = false;  // Indica a los productores que detengan la producción
            pthread_mutex_unlock(&mutex);

            // Libera a los productores que puedan estar bloqueados
            for (int i = 0; i < MAX_BUFFER; ++i) {
                sem_post(&vacios);
            }
            break;
        }
    }

    return NULL;
}

// Función para generar un reporte final de piezas sobrantes y sillas producidas
void generarReporte() {
    cout << "\n--- Reporte Final ---" << endl;
    cout << "Sillas fabricadas: " << sillasProducidas << endl;

    // Contar las piezas sobrantes en el buffer
    pthread_mutex_lock(&mutex);
    for (int i = 0; i < MAX_BUFFER; ++i) {
        if (buffer[i] == 0 || buffer[i] == 3 || buffer[i] == 4) piezasRestantes[0]++;  // Pata
        else if (buffer[i] == 1) piezasRestantes[1]++;  // Respaldo
        else if (buffer[i] == 2) piezasRestantes[2]++;  // Asiento
    }
    pthread_mutex_unlock(&mutex);

    // Mostrar el resultado final
    cout << "Piezas sobrantes en el almacén:" << endl;
    cout << "Patas: " << piezasRestantes[0] << endl;
    cout << "Respaldos: " << piezasRestantes[1] << endl;
    cout << "Asientos: " << piezasRestantes[2] << endl;
}

int main() {
    int numProductores, numConsumidores;

    // Solicitar la cantidad de productores y consumidores
    cout << "Ingrese el número de productores: ";
    cin >> numProductores;
    cout << "Ingrese el número de consumidores: ";
    cin >> numConsumidores;

    pthread_t productores[100], consumidores[100];
    int idProductores[100], idConsumidores[100];

    // Inicializa semáforos y mutex
    sem_init(&vacios, 0, MAX_BUFFER);
    sem_init(&llenos, 0, 0);
    pthread_mutex_init(&mutex, NULL);

    // Crea hilos productores
    for (int i = 0; i < numProductores; ++i) {
        idProductores[i] = i + 1;
        pthread_create(&productores[i], NULL, productor, &idProductores[i]);
    }

    // Crea hilos consumidores
    for (int i = 0; i < numConsumidores; ++i) {
        idConsumidores[i] = i + 1;
        pthread_create(&consumidores[i], NULL, consumidor, &idConsumidores[i]);
    }

    // Espera a que los hilos terminen
    for (int i = 0; i < numProductores; ++i) {
        pthread_join(productores[i], NULL);
    }
    for (int i = 0; i < numConsumidores; ++i) {
        pthread_join(consumidores[i], NULL);
    }

    // Genera el reporte final
    generarReporte();

    // Destruye semáforos y mutex
    sem_destroy(&vacios);
    sem_destroy(&llenos);
    pthread_mutex_destroy(&mutex);

    return 0;
}