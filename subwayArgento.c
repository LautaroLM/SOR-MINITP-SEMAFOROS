#include <stdio.h>      // libreria estandar
#include <stdlib.h>     // para usar exit y funciones de la libreria standard
#include <string.h>
#include <pthread.h>    // para usar threads
#include <semaphore.h>  // para usar semaforos
#include <unistd.h>

#define LIMITE 50

//creo estructura de semaforos 
struct semaforos {
    sem_t sem_mezclar;
    sem_t sem_salar;
    sem_t sem_agregar_carne;
    sem_t sem_empanar;
    sem_t sem_freir;
};

//creo los pasos con los ingredientes
struct paso {
    char accion [LIMITE];
    char ingredientes[4][LIMITE];
};

//creo los parametros de los hilos de las acciones
struct parametro {
    int equipo_param;
    struct semaforos semaforos_param;
    struct paso pasos_param[8];
    FILE * archivo;
    pthread_mutex_t * sem_archivo;
    pthread_mutex_t * sem_sarten;
    pthread_mutex_t * sem_salero;
    sem_t * sem_horno;
};

//creo los parametros de los hilos 
struct parametro_equipo {
    int numero_equipo;
    FILE * archivo;
    pthread_mutex_t * sem_archivo;
    pthread_mutex_t * sem_sarten;
    pthread_mutex_t * sem_salero;
    sem_t * sem_horno;
};

//funcion para imprimir las acciones y los ingredientes de la accion
void* imprimirAccion(void *data, char *accionIn) {
    struct parametro *mydata = data;
    //calculo la longitud del array de pasos 
    int sizeArray = (int)( sizeof(mydata->pasos_param) / sizeof(mydata->pasos_param[0]));
    //indice para recorrer array de pasos 
    int i;
    for(i = 0; i < sizeArray; i ++){
        //pregunto si la accion del array es igual a la pasada por parametro (si es igual la funcion strcmp devuelve cero)
        if(strcmp(mydata->pasos_param[i].accion, accionIn) == 0){
            printf("\tEquipo %d - accion %s \n " , mydata->equipo_param, mydata->pasos_param[i].accion);
            //calculo la longitud del array de ingredientes
            int sizeArrayIngredientes = (int)( sizeof(mydata->pasos_param[i].ingredientes) / sizeof(mydata->pasos_param[i].ingredientes[0]) );
            //indice para recorrer array de ingredientes
            int h;
            printf("\tEquipo %d -----------ingredientes : ----------\n",mydata->equipo_param); 
            for(h = 0; h < sizeArrayIngredientes; h++) {
                //consulto si la posicion tiene valor porque no se cuantos ingredientes tengo por accion 
                if(strlen(mydata->pasos_param[i].ingredientes[h]) != 0) {
                    printf("\tEquipo %d ingrediente  %d : %s \n",mydata->equipo_param,h,mydata->pasos_param[i].ingredientes[h]);
                }
            }
        }
    }
}

void imprimirLog(FILE * fd, char * log, int equipo, char * accion, pthread_mutex_t * semaforo) {
    pthread_mutex_lock(semaforo);
    fprintf(fd, log, equipo, accion);
    pthread_mutex_unlock(semaforo);
}

void* cortar(void *data) {
    //creo el nombre de la accion de la funcion 
    char *accion = "cortar";
    //creo el puntero para pasarle la referencia de memoria (data) del struct pasado por parametro (la cual es un puntero). 
    struct parametro *mydata = data;

    usleep(20000);
    

    imprimirAccion(mydata,accion);//.

    //escribo el log
    imprimirLog(mydata->archivo, "¡Equipo %d empezo a %s!\n", mydata->equipo_param, accion, mydata->sem_archivo);

    //escribo el log
    imprimirLog(mydata->archivo, "¡Equipo %d termino de %s!\n", mydata->equipo_param, accion, mydata->sem_archivo);

    

    //doy la señal a la siguiente accion (cortar me habilita mezclar)
    sem_post(&mydata->semaforos_param.sem_mezclar);
    
    pthread_exit(NULL);
}

void* mezclar(void *data) {
    //creo el nombre de la accion de la funcion 
    char *accion = "mezclar";
    //creo el puntero para pasarle la referencia de memoria (data) del struct pasado por parametro (la cual es un puntero). 
    struct parametro *mydata = data;

    usleep(20000);

    imprimirAccion(mydata,accion);//.

    sem_wait(&mydata->semaforos_param.sem_mezclar);

    //escribo el log
    imprimirLog(mydata->archivo, "¡Equipo %d empezo a %s!\n", mydata->equipo_param, accion, mydata->sem_archivo);

    //escribo el log
    imprimirLog(mydata->archivo, "¡Equipo %d termino de %s!\n", mydata->equipo_param, accion, mydata->sem_archivo);

    

    //doy la señal a la siguiente accion (cortar me habilita mezclar)
    sem_post(&mydata->semaforos_param.sem_salar);
    
    pthread_exit(NULL);
}

void* salar(void *data) {
    //creo el nombre de la accion de la funcion 
    char *accion = "salar";
    //creo el puntero para pasarle la referencia de memoria (data) del struct pasado por parametro (la cual es un puntero). 
    struct parametro *mydata = data;

    usleep(5000000);

    imprimirAccion(mydata,accion);//.

    sem_wait(&mydata->semaforos_param.sem_salar);

    pthread_mutex_lock(mydata->sem_salero);

    //escribo el log
    imprimirLog(mydata->archivo, "¡Equipo %d empezo a %s!\n", mydata->equipo_param, accion, mydata->sem_archivo);

    //escribo el log
    imprimirLog(mydata->archivo, "¡Equipo %d termino de %s!\n", mydata->equipo_param, accion, mydata->sem_archivo);

    pthread_mutex_unlock(mydata->sem_salero);

    //doy la señal a la siguiente accion (cortar me habilita mezclar)
    sem_post(&mydata->semaforos_param.sem_agregar_carne);
    
    pthread_exit(NULL);
}

void* agregar_carne(void *data) {
    //creo el nombre de la accion de la funcion 
    char *accion = "agregar carne";
    //creo el puntero para pasarle la referencia de memoria (data) del struct pasado por parametro (la cual es un puntero). 
    struct parametro *mydata = data;

    usleep(20000);

    imprimirAccion(mydata,accion);//.

    sem_wait(&mydata->semaforos_param.sem_agregar_carne);

    //escribo el log
    imprimirLog(mydata->archivo, "¡Equipo %d empezo a %s!\n", mydata->equipo_param, accion, mydata->sem_archivo);

    //escribo el log
    imprimirLog(mydata->archivo, "¡Equipo %d termino de %s!\n", mydata->equipo_param, accion, mydata->sem_archivo);

    //doy la señal a la siguiente accion (cortar me habilita mezclar)
    sem_post(&mydata->semaforos_param.sem_empanar);
    
    pthread_exit(NULL);
}

void* empanar(void *data) {
    //creo el nombre de la accion de la funcion 
    char *accion = "empanar";
    //creo el puntero para pasarle la referencia de memoria (data) del struct pasado por parametro (la cual es un puntero). 
    struct parametro *mydata = data;

    usleep(20000);

    imprimirAccion(mydata,accion);//.

    sem_wait(&mydata->semaforos_param.sem_empanar);

    //escribo el log
    imprimirLog(mydata->archivo, "¡Equipo %d empezo a %s!\n", mydata->equipo_param, accion, mydata->sem_archivo);

    //escribo el log
    imprimirLog(mydata->archivo, "¡Equipo %d termino de %s!\n", mydata->equipo_param, accion, mydata->sem_archivo);

    //doy la señal a la siguiente accion (cortar me habilita mezclar)
    sem_post(&mydata->semaforos_param.sem_freir);
    
    pthread_exit(NULL);
}

void* freir(void *data) {
    //creo el nombre de la accion de la funcion 
    char *accion = "freir";
    //creo el puntero para pasarle la referencia de memoria (data) del struct pasado por parametro (la cual es un puntero). 
    struct parametro *mydata = data;

    usleep(5000000);

    imprimirAccion(mydata,accion);//.

    sem_wait(&mydata->semaforos_param.sem_freir);

    pthread_mutex_lock(mydata->sem_sarten);

    //escribo el log
    imprimirLog(mydata->archivo, "¡Equipo %d empezo a %s!\n", mydata->equipo_param, accion, mydata->sem_archivo);

    //escribo el log
    imprimirLog(mydata->archivo, "¡Equipo %d termino de %s!\n", mydata->equipo_param, accion, mydata->sem_archivo);

    pthread_mutex_unlock(mydata->sem_sarten);

    pthread_exit(NULL);
}

void* cocinar_pan(void *data) {
    //creo el nombre de la accion de la funcion 
    char *accion = "cocinar pan";
    //creo el puntero para pasarle la referencia de memoria (data) del struct pasado por parametro (la cual es un puntero). 
    struct parametro *mydata = data;

    usleep(5000000);

    imprimirAccion(mydata,accion);//.

    sem_wait(mydata->sem_horno);

    //escribo el log
    imprimirLog(mydata->archivo, "¡Equipo %d empezo a %s!\n", mydata->equipo_param, accion, mydata->sem_archivo);
    
    //escribo el log
    imprimirLog(mydata->archivo, "¡Equipo %d termino de %s!\n", mydata->equipo_param, accion, mydata->sem_archivo);
    
    sem_post(mydata->sem_horno);

    pthread_exit(NULL);
}

void* extras(void *data) {
    //creo el nombre de la accion de la funcion 
    char *accion = "extras";
    //creo el puntero para pasarle la referencia de memoria (data) del struct pasado por parametro (la cual es un puntero). 
    struct parametro *mydata = data;

    usleep(20000);

    imprimirAccion(mydata,accion);//.

    //escribo el log
    imprimirLog(mydata->archivo, "¡Equipo %d empezo a %s!\n", mydata->equipo_param, accion, mydata->sem_archivo);

    //escribo el log
    imprimirLog(mydata->archivo, "¡Equipo %d termino de %s!\n", mydata->equipo_param, accion, mydata->sem_archivo);
    
    pthread_exit(NULL);
}

void* ejecutarReceta(void *i) {
    //variables semaforos
    sem_t sem_mezclar;
    sem_t sem_salar;
    sem_t sem_agregar_carne;
    sem_t sem_empanar;
    sem_t sem_freir;
    
    //variables hilos
    pthread_t p1; 
    pthread_t p2; 
    pthread_t p3; 
    pthread_t p4; 
    pthread_t p5; 
    pthread_t p6; 
    pthread_t p7; 
    pthread_t p8; 

    //numero del equipo (casteo el puntero a un int)
    struct parametro_equipo * data_equipo = (struct parametro_equipo *) i;
    
    printf("Ejecutando equipo %d \n", data_equipo->numero_equipo);

    //reservo memoria para el struct
    struct parametro *pthread_data = malloc(sizeof(struct parametro));

    //seteo los valores al struct
    //seteo numero de grupo
    pthread_data->equipo_param = data_equipo->numero_equipo;

    //seteo el archivo de logs
    pthread_data->archivo = data_equipo->archivo;
    pthread_data->sem_archivo = data_equipo->sem_archivo;

    //seteo los semaforos de la cocina
    pthread_data->sem_sarten = data_equipo->sem_sarten;
    pthread_data->sem_salero = data_equipo->sem_salero;
    pthread_data->sem_horno = data_equipo->sem_horno;

    //seteo semaforos
    pthread_data->semaforos_param.sem_mezclar = sem_mezclar;
    pthread_data->semaforos_param.sem_salar = sem_salar;
    pthread_data->semaforos_param.sem_agregar_carne = sem_agregar_carne;
    pthread_data->semaforos_param.sem_empanar = sem_empanar;
    pthread_data->semaforos_param.sem_freir = sem_freir;
    
    //seteo las acciones y los ingredientes (Faltan acciones e ingredientes) ¿Se ve hardcodeado no? SI! ¿Les parece bien? Si, por ahora si!
    strcpy(pthread_data->pasos_param[0].accion, "cortar");
    strcpy(pthread_data->pasos_param[0].ingredientes[0], "ajo");
    strcpy(pthread_data->pasos_param[0].ingredientes[1], "perejil");

    strcpy(pthread_data->pasos_param[1].accion, "mezclar");
    strcpy(pthread_data->pasos_param[1].ingredientes[0], "ajo");
    strcpy(pthread_data->pasos_param[1].ingredientes[1], "perejil");
    strcpy(pthread_data->pasos_param[1].ingredientes[2], "huevo");

    strcpy(pthread_data->pasos_param[2].accion, "salar");
    strcpy(pthread_data->pasos_param[2].ingredientes[0], "mezcla");
    strcpy(pthread_data->pasos_param[2].ingredientes[1], "sal");

    strcpy(pthread_data->pasos_param[3].accion, "agregar carne");
    strcpy(pthread_data->pasos_param[3].ingredientes[0], "mezcla");
    strcpy(pthread_data->pasos_param[3].ingredientes[1], "carne");

    strcpy(pthread_data->pasos_param[4].accion, "empanar");
    strcpy(pthread_data->pasos_param[4].ingredientes[0], "carne");
    strcpy(pthread_data->pasos_param[4].ingredientes[1], "pan rallado");

    strcpy(pthread_data->pasos_param[5].accion, "freir");
    strcpy(pthread_data->pasos_param[5].ingredientes[0], "milanesa");

    strcpy(pthread_data->pasos_param[6].accion, "cocinar pan");
    strcpy(pthread_data->pasos_param[6].ingredientes[0], "mezcla pan");

    strcpy(pthread_data->pasos_param[7].accion, "extras");
    strcpy(pthread_data->pasos_param[7].ingredientes[0], "lechuga");
    strcpy(pthread_data->pasos_param[7].ingredientes[1], "tomate");
    strcpy(pthread_data->pasos_param[7].ingredientes[2], "cebolla morada");
    strcpy(pthread_data->pasos_param[7].ingredientes[3], "pepino");

    //inicializo los semaforos
    sem_init(&(pthread_data->semaforos_param.sem_mezclar), 0, 0);
    sem_init(&(pthread_data->semaforos_param.sem_salar), 0, 0);
    sem_init(&(pthread_data->semaforos_param.sem_agregar_carne), 0, 0);
    sem_init(&(pthread_data->semaforos_param.sem_empanar), 0, 0);
    sem_init(&(pthread_data->semaforos_param.sem_freir), 0, 0);

    //creo los hilos a todos les paso el struct creado (el mismo a todos los hilos) ya que todos comparten los semaforos 
    int rc;
    rc = pthread_create(&p1,            //identificador unico
                        NULL,           //atributos del thread
                        cortar,         //funcion a ejecutar
                        pthread_data);  //parametros de la funcion a ejecutar, pasado por referencia
    //valido que el hilo se halla creado bien 
    if (rc){
       printf("Error:unable to create thread, %d \n", rc);
       exit(-1);
    }

    rc = pthread_create(&p2,            //identificador unico
                        NULL,           //atributos del thread
                        mezclar,        //funcion a ejecutar
                        pthread_data);  //parametros de la funcion a ejecutar, pasado por referencia
    //valido que el hilo se halla creado bien 
    if (rc){
       printf("Error:unable to create thread, %d \n", rc);
       exit(-1);
    }

    rc = pthread_create(&p3,            //identificador unico
                        NULL,           //atributos del thread
                        salar,          //funcion a ejecutar
                        pthread_data);  //parametros de la funcion a ejecutar, pasado por referencia
    //valido que el hilo se halla creado bien 
    if (rc){
       printf("Error:unable to create thread, %d \n", rc);
       exit(-1);
    }

    rc = pthread_create(&p4,            //identificador unico
                        NULL,           //atributos del thread
                        agregar_carne,  //funcion a ejecutar
                        pthread_data);  //parametros de la funcion a ejecutar, pasado por referencia
    //valido que el hilo se halla creado bien 
    if (rc){
       printf("Error:unable to create thread, %d \n", rc);
       exit(-1);
    }

    rc = pthread_create(&p5,            //identificador unico
                        NULL,           //atributos del thread
                        empanar,        //funcion a ejecutar
                        pthread_data);  //parametros de la funcion a ejecutar, pasado por referencia
    //valido que el hilo se halla creado bien 
    if (rc){
       printf("Error:unable to create thread, %d \n", rc);
       exit(-1);
    }

    rc = pthread_create(&p6,            //identificador unico
                        NULL,           //atributos del thread
                        freir,          //funcion a ejecutar
                        pthread_data);  //parametros de la funcion a ejecutar, pasado por referencia
    //valido que el hilo se halla creado bien 
    if (rc){
       printf("Error:unable to create thread, %d \n", rc);
       exit(-1);
    }

    rc = pthread_create(&p7,            //identificador unico
                        NULL,           //atributos del thread
                        cocinar_pan,    //funcion a ejecutar
                        pthread_data);  //parametros de la funcion a ejecutar, pasado por referencia
    //valido que el hilo se halla creado bien 
    if (rc){
       printf("Error:unable to create thread, %d \n", rc);
       exit(-1);
    }

    rc = pthread_create(&p8,            //identificador unico
                        NULL,           //atributos del thread
                        extras,      //funcion a ejecutar
                        pthread_data);  //parametros de la funcion a ejecutar, pasado por referencia
    //valido que el hilo se halla creado bien 
    if (rc){
       printf("Error:unable to create thread, %d \n", rc);
       exit(-1);
    }

    //join de todos los hilos
    pthread_join (p1,NULL);
    pthread_join (p2,NULL);
    pthread_join (p3,NULL);
    pthread_join (p4,NULL);
    pthread_join (p5,NULL);
    pthread_join (p6,NULL);
    pthread_join (p7,NULL);
    pthread_join (p8,NULL);

    fprintf(pthread_data->archivo, "¡Equipo %d termino de armar el sandwich!\n", pthread_data->equipo_param);
    printf("Equipo %d termino el sandwich! \n", data_equipo->numero_equipo);

    //destruccion de los semaforos 
    sem_destroy(&sem_mezclar);
    sem_destroy(&sem_salar);
    sem_destroy(&sem_agregar_carne);
    sem_destroy(&sem_empanar);
    sem_destroy(&sem_freir);
    
    //salida del hilo
    pthread_exit(NULL);
}

void inicializarEquipos(struct parametro_equipo * data_equipo, int numero_equipo, FILE * archivo_logs, 
                        pthread_mutex_t * sem_archivo, pthread_mutex_t * sem_sarten, 
                        pthread_mutex_t * sem_salero, sem_t * sem_horno) {
    data_equipo->numero_equipo = numero_equipo;
    data_equipo->archivo = archivo_logs;
    data_equipo->sem_archivo = sem_archivo;
    data_equipo->sem_sarten = sem_sarten;
    data_equipo->sem_salero = sem_salero;
    data_equipo->sem_horno = sem_horno;
}

int main ()
{
    //creo los nombres de los equipos 
    int rc;
    struct parametro_equipo *data_equipo1 = malloc(sizeof(struct parametro_equipo));
    struct parametro_equipo *data_equipo2 = malloc(sizeof(struct parametro_equipo));
    struct parametro_equipo *data_equipo3 = malloc(sizeof(struct parametro_equipo));
    struct parametro_equipo *data_equipo4 = malloc(sizeof(struct parametro_equipo));

    //archivo con logs
    FILE * archivo_logs;
    archivo_logs = fopen("log.txt", "w");

    //inicializo los semaforos
    pthread_mutex_t sem_archivo;
    pthread_mutex_t sem_sarten;
    pthread_mutex_t sem_salero;
    sem_t sem_horno;

    pthread_mutex_init(&sem_archivo, NULL);
    pthread_mutex_init(&sem_sarten, NULL);
    pthread_mutex_init(&sem_salero, NULL);
    sem_init(&sem_horno, 0, 2);

    //inicializo la data de los archivos
    inicializarEquipos(data_equipo1, 1, archivo_logs, &sem_archivo, &sem_sarten, &sem_salero, &sem_horno); 
    inicializarEquipos(data_equipo2, 2, archivo_logs, &sem_archivo, &sem_sarten, &sem_salero, &sem_horno); 
    inicializarEquipos(data_equipo3, 3, archivo_logs, &sem_archivo, &sem_sarten, &sem_salero, &sem_horno); 
    inicializarEquipos(data_equipo4, 4, archivo_logs, &sem_archivo, &sem_sarten, &sem_salero, &sem_horno); 

    //creo las variables los hilos de los equipos
    pthread_t equipo1; 
    pthread_t equipo2;
    pthread_t equipo3;
    pthread_t equipo4;
  
    //inicializo los hilos de los equipos
    rc = pthread_create(&equipo1,           //identificador unico
                        NULL,               //atributos del thread
                        ejecutarReceta,     //funcion a ejecutar
                        data_equipo1); 
    if (rc){
        printf("Error:unable to create thread, %d \n", rc);
        exit(-1);
    }
    
    rc = pthread_create(&equipo2,           //identificador unico
                        NULL,               //atributos del thread
                        ejecutarReceta,     //funcion a ejecutar
                        data_equipo2);
    if (rc){
        printf("Error:unable to create thread, %d \n", rc);
        exit(-1);
    }

    rc = pthread_create(&equipo3,           //identificador unico
                        NULL,               //atributos del thread
                        ejecutarReceta,     //funcion a ejecutar
                        data_equipo3);
    if (rc){
        printf("Error:unable to create thread, %d \n", rc);
        exit(-1);
    }

    rc = pthread_create(&equipo4,           //identificador unico
                        NULL,               //atributos del thread
                        ejecutarReceta,     //funcion a ejecutar
                        data_equipo4);
    if (rc){
        printf("Error:unable to create thread, %d \n", rc);
        exit(-1);
    }

    //join de todos los hilos
    pthread_join (equipo1,NULL);
    pthread_join (equipo2,NULL);
    pthread_join (equipo3,NULL);
    pthread_join (equipo4,NULL);

    pthread_exit(NULL);

    //destruccion de los semaforos 
    pthread_mutex_destroy(&sem_archivo);
    pthread_mutex_destroy(&sem_sarten);
    pthread_mutex_destroy(&sem_salero);
    sem_destroy(&sem_horno);

    //cerramos el archivo 
    fclose(archivo_logs);
}
