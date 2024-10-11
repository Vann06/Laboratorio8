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

//Se debe de agregar el manejo de piezas realizadas
int piezasProducidas[numProductos] ={0}; 
//Booleano para verificar que ya se terminaron las 3 sillas
bool produccion_terminada = false;

// Semáforos y mutex
sem_t vacios;   
sem_t llenos;   
pthread_mutex_t mutex;

// Función simulación de un productor (fabrica una pieza de silla)
void* productor(void* arg) {
    int id = *(int*)arg;
    int piezaId;
    
    while (true) {
        piezaId = rand() % numProductos;  	// Seleccionar una pieza  al azar

        sem_wait(&vacios);  				// Espera hasta que hay espacio en el buffer
        pthread_mutex_lock(&mutex);  		// Protege el acceso al buffer

        buffer[in] = piezaId;				// Añade la pieza al buffer
        cout << "Productor " << id << " ha fabricado la pieza " << productos[piezaId]
             << " y la coloco en la posicion " << in << endl;
        in = (in + 1) % MAX_BUFFER;  		// Avanza el índice circular del buffer

        pthread_mutex_unlock(&mutex);
        sem_post(&llenos);  				// Incrementa el número de productos disponibles
        
        sleep(1);  							// Simula el tiempo de fabricación
    }
    
    return NULL;
}

// Función simulación de un consumidor (ensambla una silla)
void* consumidor(void* arg) {
    int id = *(int*)arg;
    int piezaId;

    while (sillasProducidas < MAX_SILLAS) {
        sem_wait(&llenos);  				// Espera hasta que existan productos disponibles
        pthread_mutex_lock(&mutex);  		// Protege el acceso al buffer

        // Retirar una pieza del buffer
        piezaId = buffer[out];
        cout << "++Consumidor " << id << " ha retirado la pieza " << productos[piezaId]
             << " de la posicion " << out << endl;
        out = (out + 1) % MAX_BUFFER;  		// Avanza en el índice circular del buffer

        // Incrementa el contador de sillas ensambladas cuando se consumen todas las piezas necesarias
        if (piezaId == numProductos - 1) {
            sillasProducidas++;
            cout << "Consumidor " << id << " ha ensamblado una silla completa. Sillas ensambladas: "
                 << sillasProducidas << "/" << MAX_SILLAS << endl;
        }

        pthread_mutex_unlock(&mutex);
        sem_post(&vacios);  				// Incrementa el número de espacios vacíos
        
        sleep(2);  							// Simula el tiempo de ensamblaje
    }

    return NULL;
}

int main() {
    int numProductores, numConsumidores;

    // Solicitar la cantidad de productores y consumidores
    cout << "Ingrese el numero de productores: ";
    cin >> numProductores;
    cout << "Ingrese el numero de consumidores: ";
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

    // Destruye semáforos y mutex
    sem_destroy(&vacios);
    sem_destroy(&llenos);
    pthread_mutex_destroy(&mutex);

    return 0;
}
