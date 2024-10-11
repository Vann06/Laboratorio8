/*---------------------------------------
UNIVERSIDAD DEL VALLE DE GUATEMALA
CC3056 - Programacion de Microprocesadores
Autores:  Vianka Castro 23201
          Ricardo Arturo 23247
Fecha: 10/10/2024
Mod:   --/10/2024
ejercicio_2.c
Manejar un recurso compartido (ATM)
con muchos hilos (Clientes)
---------------------------------------*/
#include <iostream>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

using namespace std;
//Variable global
double saldo = 100000.00;

//Semáforo
sem_t sem;

//Función para poder retirar dinero 
void* retirar(void* arg) {
    double* dinero_retirado = (double*)arg;
    
    //Verificar que exista disponibilidad o sino esperar
    sem_wait(&sem);

    if (*dinero_retirado <= saldo) {
        saldo -= *dinero_retirado;
        printf("Saldo de Q%.2f ha sido retirado con éxito.\n", *dinero_retirado);
        printf("Saldo restante es de: Q%.2f\n", saldo);
    } else {
        printf("AVISO! Retiro de Q%.2f no se pudo realizar\n",*dinero_retirado);
        printf("Saldo restante es de: Q%.2f\n", saldo);
    }

    //Liberar el semáforo
    sem_post(&sem);
    pthread_exit(NULL);
}

int main() {
    int cant_clientes;

    //Inicialización del semáforo para 1 cliente a la vez
    sem_init(&sem, 0, 1);

    //Solicitar la cantidad de clientes
    cout << "Ingrese el número de clientes: ";
    cin >> cant_clientes;

    //Crear arreglos para hilos y montos
    pthread_t hilos[cant_clientes];
    double montos[cant_clientes];

    //Pedir la cantidad que desea retirar cada cliente 
    for (int i = 0; i < cant_clientes; i++) {
        cout << "Para el cliente #" << i + 1 << ", ingrese su monto a retirar: Q";
        //guardarlo en montos
        cin >> montos[i];
    }

    cout << "Montos y clientes guardados con éxito!\n";

    //Crear hilos por cliente
    for (int i = 0; i < cant_clientes; i++) {
        pthread_create(&hilos[i], NULL, retirar, &montos[i]);
    }

    // Unir los hilos
    for (int i = 0; i < cant_clientes; i++) {
        pthread_join(hilos[i], NULL);
    }

    // Destruir el semáforo
    sem_destroy(&sem);

    cout << "Las transacciones han terminado.\n";

    return 0;
}
