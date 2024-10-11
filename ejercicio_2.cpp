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

//Semaforo
sem_t sem;

//Funcion para poder retirar dinero 
void* retirar(void* arg){
    double * dinero_retirado = (double*)arg;
    
    //Siempre intenta acceder al semaforo antes
    sem_wait(&sem);

    if(*dinero_retirado <= saldo){
        saldo -= *dinero_retirado;
        printf("Saldo de %d ha sido retirado con exito\n",*dinero_retirado);
        printf("Saldo restante es de: %d", saldo);
    }
    else{
        printf("AVISO! No se pudo realizar la transaccion, el saldo no es suficiente \n");
        printf("Saldo restante es de %d\n",saldo);
    }

    sem_post(&sem);
    pthread_exit(NULL);


}

int main(){
    int cant_clientes;

    //Inicializacion del semaforo para 1 cliente a la vez
    sem_init(&sem,0,1);

    printf("Ingrese el numero de clientes >\n") ;
    cin >> cant_clientes;

    //Crear arreglos
    pthread_t hilos[cant_clientes];
    double montos[cant_clientes];

    //Pedir la cantidad que desea cada cliente y guardarlo en montos
    for (int i = 0; i < cant_clientes; i++ ){
        printf("Para el cliente # %i, ingrese su monto a reritar > \n",i+1);
        cin >> montos[i];
    }

    printf("Montos y Clientes guardados con exito!");

    //Crear hilos para cada cliente
    for(int i = 0; i < cant_clientes; i++){
        pthread_create(&hilos[i],NULL,retirar,&montos[i]);
    }

    //Unir todos
     for(int i = 0; i < cant_clientes; i++){
        pthread_join(hilos[i],NULL);
    }

    //destruir el sem
    sem_destroy(&sem);


    printf("Las transacciones han terminado");


    return 0;

}