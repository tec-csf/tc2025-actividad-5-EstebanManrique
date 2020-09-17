/**
 * Autor - Esteban Manrique de Lara Sirvent
 * Fecha - 17/09/2020
 * Actividad 5: IPC
 *
*/
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <math.h>

struct Tuberia //Estructura utilizada para arreglo de pipes entre los procesos del Token Ring
{
    int tuberia[2];
};

void numeroProcesos(int*, int, char* const*);
void tokenRing(int, struct Tuberia*);
void habilitarTodasTuberias(struct Tuberia*, int);
void vueltas(int, struct Tuberia*, char);
void escribir(int*, char);
char leer(int*);

int main(int argc, char* const* argv)
{
    struct Tuberia* arregloTuberias; //Arreglo con todas las pipes del Token Ring
    int* numeroProces; //Aqui se almacenara el numero de procesos a ser considerados para el Token Ring
    numeroProces = (int*)malloc(sizeof(int));
    *numeroProces = -1; //Valor predeterminado
    numeroProcesos(numeroProces, argc, argv); //Se obtiene el numero de procesos mediante argumento introducido en consola
    if(*numeroProces == -1) //En caso de que valor introducido sea Mayor o Igual a CERO o NO SEA un numero Entero
    {
        printf("El Token Ring no se pudo crear ya que el numero introucido fue invalido\n");
        free(numeroProces); //Se libera variable de numero de procesos
        return -1; //Se termina el programa
    }
    arregloTuberias = (struct Tuberia*)malloc(*numeroProces * sizeof(struct Tuberia)); //Alocacion de memoria una vez que se sabe numero de procesos
    tokenRing(*numeroProces, arregloTuberias); //Se inicia el Token Ring
    free(arregloTuberias); //Se libera el arreglo con las pipes del Token Ring
    free(numeroProces); //Se libera variable de numero de procesos
    return 0;
}

/**
 * Funcion encargada de recibir y analizar el numero de procesos hijos a ser creados, el cual se introduce como paramtero de -n.
 * 
 * @param numeroHij, Apuntador a variable ubicada en main(), encargada de almacenar el numero de procesos hijos a crear. Si no es entera, manda error y termina programa
 * @param numeroArgumentos, recibe la variable argc del metodo main()
 * @param sizeArca, recibe la variable argv del metodo main()
 * 
 **/
void numeroProcesos(int* numeroProces, int numeroArgumentos, char* const* argumentos)
{
    int aux;
    int auxValidacion;
    while((aux= getopt(numeroArgumentos, argumentos, "n:")) != -1)
    {
        switch (aux)
        {
        case 'n':
            auxValidacion = (atoi(optarg) == 0) ? 1  : (log10((atoi(optarg))) + 1); //Checa que valor introducido por el usuario no combine letras y numeros (16hola)
            if(auxValidacion == strlen(optarg) && atoi(optarg)>0 && strstr(optarg, ".") == NULL) //Se verifica que valor sea mayor a CERO, No sea decimal
            {
                *(numeroProces) = atoi(optarg); //Se convierte a entero valor introducido por el usuario
                printf("%d\n", *numeroProces);
                return;
            }
            else
            {
                printf("La opcion introducida era MENOR o IGUAL a ZERO o NO ERA un entero.\n");
                return;
            }
            break;
        case '?':
            if (optopt == 'n')
            {
                fprintf (stderr, "Opción -%c requiere un argumento. Introducir -n NUMERO DE PROCESOS (ej. -n 4)\n", optopt); //Se imprime una pequena ayuda para el usuario
            }
            else if (isprint (optopt))
            {
                fprintf (stderr, "Opción desconocida.'-%c'. Introducir -n NUMERO DE PROCESOS (ej. -n 4). \n", optopt); //Se imprime una pequena ayuda para el usuario
            }
            else
            {
                fprintf (stderr, "Opción desconocida.'\\x%x'.  Introducir -n NUMERO DE PROCESOS (ej. -n 4) \n",optopt); //Se imprime una pequena ayuda para el usuario
            }
            break;
        default:
            break;
        }
    }
    return;
}

/**
 * Funcion encargada de generar la habilitacion de todas las tuberias del Anillo, definir Testigo e invocar las vueltas del Token Ring.
 * 
 * @param numeroProcesos, Apuntador a variable ubicada en main(), encargada de almacenar el numero de procesos hijos a crear. Si no es entera, manda error y termina programa
 * @param arregloTuberias, Arreglo que contiene todas las pipes a ser utilizadas por el Token Ring
 * 
 **/
void tokenRing(int numeroProcesos, struct Tuberia* arregloTuberias)
{
    char testigo = 'T';
    pid_t pid;
    habilitarTodasTuberias(arregloTuberias, numeroProcesos);
    vueltas(numeroProcesos, arregloTuberias, testigo);
}

/**
 * Funcion encargada de invocar el metodo pipe(), para inicializar cada una de las pipes que se utilizaran para comunciar a los procesos del Token Ring.
 * 
 * @param tuberias, Arreglo que contiene todas las pipes a ser utilizadas por el Token Ring
 * @param numeroProcesos, Apuntador a variable ubicada en main(), encargada de almacenar el numero de procesos hijos a crear. Si no es entera, manda error y termina programa
 * 
 **/
void habilitarTodasTuberias(struct Tuberia* tuberias, int numeroProcesos)
{
    struct Tuberia* auxTuberia = tuberias;
    for(; auxTuberia<(tuberias + numeroProcesos); auxTuberia++)
    {
        pipe(auxTuberia->tuberia);
    }
    return;
}

/**
 * Funcion encargada de comunicar a los procesos del Token Ring, invocar las funciones de lectura y escritura y simular el proceso de comunicacion de un Token Ring.
 * 
 * @param numeroProcesos, Apuntador a variable ubicada en main(), encargada de almacenar el numero de procesos hijos a crear. Si no es entera, manda error y termina programa
 * @param arregloTuberias, Arreglo que contiene todas las pipes a ser utilizadas por el Token Ring
 * @param testigo, el cual es el caracter que los diferentes procesos que conforman al Token Ring se pasaran mutuamente
 * 
 **/
void vueltas(int numeroProcesos, struct Tuberia* arregloTuberias, char testigo)
{
    struct Tuberia* auxTuberia = arregloTuberias; //Variable auxiliar para recorrido de arreglo de tuberias
    int pidTemp;
    pid_t pid;
    int estado;
    pid = fork();
    pidTemp = pid;
    if(pid == 0) //Proceso con el cual se "crea" el Token Ring y se le pasa el Testigo al primer proceso en la primera iteracion
    {
        printf("Se esta creando el Token Ring.\n");
        escribir((auxTuberia + (numeroProcesos - 1))->tuberia, testigo);
        exit(0);
    }
    if (waitpid(pidTemp, &estado, 0) != -1) //Termina este proceso, que como tal no es parte del Token Ring
    {
        if (WIFEXITED(estado))
        {
            printf("Testigo siendo trasladado...\n");
            sleep(1);
        }
    }
    for(int i = 0; i<numeroProcesos; i++)
    {
        pid = fork();
        if(pid == 0 && i == 0) //Cuando Testigo llega al primer proceso del Token Ring
        {
            while(1)
            {
                printf("Soy el proceso %d con PID %d y recibi el testigo %c, el cual tendre por 5 segundos\n", i + 1, getpid(), leer((auxTuberia + (numeroProcesos - 1))->tuberia));
                sleep(5);
                printf("Soy el proceso %d con PID %d y acabo de enviar el testigo %c\n", i + 1, getpid(), leer((auxTuberia + (numeroProcesos - 1))->tuberia));
                escribir(auxTuberia->tuberia, testigo);
                printf("Testigo siendo trasladado...\n");
                sleep(1);
            }
            exit(0);
        }
        else if(pid == 0) //Cuando Testigo se encuentra en cualquier otro proceso del Token Ring
        {
            while(1)
            {
                printf("Soy el proceso %d con PID %d y recibi el testigo %c, el cual tendre por 5 segundos\n", i + 1, getpid(), leer((auxTuberia - 1)->tuberia));
                sleep(5);
                printf("Soy el proceso %d con PID %d y acabo de enviar el testigo %c\n", i + 1, getpid(), leer((auxTuberia - 1)->tuberia));
                escribir(auxTuberia->tuberia, testigo);
                printf("Testigo siendo trasladado...\n");
                sleep(1);
            }
            exit(0);
        }
        auxTuberia++;
    }
    return;
}

/**
 * Funcion encargada de escribir el Testigo en el pipe correspondiente. Se hace una doble escritura ya que el siguiente proceso realiza 2 impresiones que 
 * involucran al testigo. Por ende, se requiere que haya dos elementos en el pipe. En caso contrario, la funcion lectura no podria leer el Testigo 2 veces.
 * 
 * @param tuberia, Tuberia donde se escribira el Testigo
 * @param datoAEscribir, El testigo a ser escrito en el pipe
 * 
 **/
void escribir(int* tuberia, char datoAEscribir)
{
    close(tuberia[0]);
    for(int i = 0; i<2; i++)
    {
        write(tuberia[1], &datoAEscribir, sizeof(char));
    }
    return;
}

/**
 * Funcion encargada de leer el testigo del pipe en curso. Se invoca por cada impresion que el proceso en turno realice.
 * 
 * @param tuberia, Tuberia donde de donde se leera a el Testigo
 * 
 **/
char leer(int* tuberia)
{
    char datoALeer;
    close(tuberia[1]);
    read(tuberia[0], &datoALeer, sizeof(char));
    return datoALeer;
}