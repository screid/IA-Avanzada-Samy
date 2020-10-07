//Cargo las librerias:
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/time.h>
#include <unistd.h>
#include <math.h>


//Defino un parámetro global LARGE (número muy grande) y un máximo de ciudades
#define LARGE 10000000.0
#define MAX_CITIES 1002
#define LARGOCOLA 100 //(QUEUQSIZE)

//Inicializo parámetros 
//1. Número de ciudades 
int ncities = 0;
//2. Matriz de distancia
double distance_matrix[MAX_CITIES][MAX_CITIES]; 
//3. Paso matriz (????)
double paso_matrix[MAX_CITIES][MAX_CITIES];
//4. ???
int succ_matrix[MAX_CITIES][MAX_CITIES];
//5. Defino ciudad de origen
short initial_city;
//6. Defino una lista con las coordenadas de las ciudades
int cities_coor[MAX_CITIES][2];


//Declaro estructuras de datos
struct node;      //Va a ser un subtour
typedef struct node node;

//Cuerpo de la estructura 
struct node{
  int city;   //ID del nodo
  int d;      //Profundidad, qué tan profundo está en el árbol.
  float g;      //Costo del subtour. Se acumula el valor del subtour en ese momento 
  node *parent;   //Nodo padre del nodo 
  node *sibling;   //Nodo hermano 
  node *firstChild;  //Puntero al primer hijo de este nodo padre (nodo de más a la izquierda.
};



//Procedimiento para crear nodo (que asigna memoria tmb)
node *create_node(){
  //1. Creo el nodo nuevo (state) y lo inicializo con ningún valor
  node *state = NULL;
  //2. Le asigno memoria:
  state = (node *)malloc(sizeof(node));
  //3. Inicializo al padre, hermano y primer hijo como nulo.
  state->parent = NULL;
  state->sibling = NULL;
  state->firstChild = NULL;

  return state;
}


//Función para leer una instancia
void read_problem(const char *filename)
{
    FILE *f;
    int y,x,num;
    f = fopen(filename,"r");
    if (f==NULL){printf( "No se puede abrir el fichero.\n" );exit(1);}
    rewind (f);
    for (y = 0; y < ncities; y++) {
      for (x = 0; x < 3; x++) {
        fscanf(f, "%d", &num);
        if (x > 0) cities_coor[y][x-1] = num;
      }
    }
    fclose (f);
}


//Función para calcular la distancia de una matriz 
void distance_matrix_caculation() {
  int y,y1;
  
  for (y = 0; y < ncities; y++) {
    for (y1 = 0; y1 < ncities; y1++) {
      if (y != y1) distance_matrix[y][y1] = sqrt(pow((cities_coor[y1][1]-cities_coor[y][1]),2) + pow((cities_coor[y1][0]-cities_coor[y][0]),2));
      else distance_matrix[y][y1] =0;
      
      //Se inicializa matrices para ordenar sucesores
      paso_matrix[y][y1] = distance_matrix[y][y1];
      succ_matrix[y][y1] = y1;
    }
  } 
}


//Crea la matriz de sucesores. Ordena las ciudades numéricamente
void succ_matrix_caculation() {
  int y,x,j,aux1,aux2;
  for (y = 0; y < ncities; y++) {
    for (x = 0; x < ncities; x++) {
      for (j = 0; j < ncities-1; j++) {
	if (paso_matrix[y][j] > paso_matrix[y][j+1]) {
	  aux1 = paso_matrix[y][j];
	  aux2 = succ_matrix[y][j];
	  paso_matrix[y][j] = paso_matrix[y][j+1];
	  succ_matrix[y][j] = succ_matrix[y][j+1];
	  paso_matrix[y][j+1] = aux1;
	  succ_matrix[y][j+1] = aux2;
	}
      }
    }
  }
}


//Crear procedimiento del Breadth First Search (búsqueda en anchura)
void BrFS(){

//1) Inicializo 4 datos de tipo nodo, hasta el momento vacíos 
  node *parent = NULL;  //Padre
  node *succ = NULL;    //Sucesor
  node *aux = NULL;     //Auxiliar
  node *state = NULL;   //Estado o nodo nuevo
  //Inicializo la cola con dimensión muy grande
  node *queue[LARGOCOLA]; 
  //Inicializo una variable entera que guarda la posición del inicio de la cola
  int pi = 0;
  //Variable entera que me indica la posición final de la cola 
  int pf = 0;
  //Contador 
  int i;
  //Variable que después me dirá pa qué sirve 
  int city_avoid;

  //Inicializo una variable para el valor Mínimo
  float valorMinimo = LARGE;
  node *ruta = NULL;

//2) Inicializamos el nodo inicial/padre ( o "A") 
  parent = create_node();
  //Le asociamos una ciudad al padre, la cual será el nodo inicial 
  parent->city = initial_city;
  //Le asociamos la profundidad al padre 
  parent->d = 0;
  //Le asociamos el costo acumulado 
  parent->g = 0.0;
  //Imprimir el estado inicial:
  //printf("[%d]\n",parent->city);

//3) Inicializamos la cola.
  //En la posición final se agrega el padre y la pos. final aumenta en 1
  queue[pf] = parent;
  pf++;
  //Se expande el estado inicial e insertaremos los nodos sucesores a la cola (Sólo para TSP)
  parent = queue[pi];   //Padre es el primer elemento de la cola 
  pi++;                 //Aumento la posición de la cola 

  //Empiezo con 0 porque mi ciudad inicial es 0, si fuera otro valor se le cambia. Por lo tanto, empiezo desde la ciudad 1 en adelante.
  for(i = 1; i < ncities; i++){
    //Genero los sucesores (o los creo), junto con la ciudad, la profundidad y la distancia recorrida, además del padre 
    succ = create_node();
    succ->city = i;   //Asigno la ciudad
    succ->d = parent->d + 1;
    succ->g = parent->g + distance_matrix[parent->city][succ->city];

    //Imprimo info del succesor
    //printf("[%d-%d](%f)",succ->city, succ->d, succ->g);  

    succ->parent = parent;

    //Hago esto para asignarle el primer hijo al padre
    if (parent->firstChild == NULL){
      parent->firstChild = succ;
    }

    //Ahora creo a los hermanos
    //Como aux inicializa en NULL, esta condición no la tomará en i=1. En i=2, el hermano de aux tomará el valor de 1
    if (aux != NULL){
      aux->sibling = succ;
    }
    //Para i = 1, aux será el valor de sucesor (me guarda el anterior)
    aux = succ;

    //Debo ingresar los sucesores a la cola:
    queue[pf] = succ;   //Vamos agregando los sucesores 
    pf++;                //Adicionamos el valor pf.

  }
  //printf("\n");

//4) Procedimiento o ciclo de la búsqueda en anchura:
  //Se hace mientas la cola tenga elementos:
  while (pi < pf){

    //Por si acaso se termina el espacio en la cola 
    if (pi == LARGOCOLA){
      pi = 0;
    }

    //Hay que sacar el primer elemento de la cola, y luego avanzo
    parent = queue[pi];
    pi++;

    //Para generar sucesores: State es el padre de parent. Esto significa que los sucesores de un nodo son los mismos que los hijos de mi padre, menos el nodo.  Obtener el primer hijo del padre de parent 
    state = parent->parent->firstChild;  //Obtengo el primer hijo del padre (es decir, mi hermano)
    
    //Guardo la ciudad de parent.
    city_avoid = parent->city;

    //Esto me ayudará a recorrer los hermanos
    aux = NULL;

    //Recorremos los hijos de mi padre(o hermanos)
    while ( state != NULL ){
      //Me aseguro de no tomar mi mismo nodo, por eso gaurde mi ciudad en una variable.
      if (state->city != city_avoid){
        //Genero los sucesores (o los creo), junto con la ciudad, la profundidad y la distancia recorrida, además del padre 
        succ = create_node();
        succ->city = state->city;   //Ahora la ciudad del sucesor es la ciudad del hermano 
        succ->d = parent->d + 1;
        succ->g = parent->g + distance_matrix[parent->city][succ->city];
        
        //Imprimo info del succesor
        //printf("[%d-%d](%f)",succ->city, succ->d, succ->g);   
        
        succ->parent = parent;

        //Hago esto para asignarle el primer hijo al padre
        if (parent->firstChild == NULL){
          parent->firstChild = succ;
        }

        //Ahora creo a los hermanos
        //Como aux inicializa en NULL, esta condición no la tomará en i=1. En i=2, el hermano de aux tomará el valor de 1
        if (aux != NULL){
          aux->sibling = succ;
        }
        //Para i = 1, aux será el valor de sucesor (me guarda el anterior)
        aux = succ;

        //Debo ingresar los sucesores a la cola:
        queue[pf] = succ;   //Vamos agregando los sucesores 
        pf++;                //Adicionamos el valor pf.

        //Si me quedo con poco espacio en la cola
        if (pf == LARGOCOLA){
          pf = 0;
        }
      }
      //Después de que termino de recorrer el nodo, tengo que ir al nodo hermano
      state = state->sibling;
    }
    //Si no tengo más hermanos, entonces significa que no tendré más hijos, por lo que puedo hacer un retorno al nodo inicial.
    if (aux == NULL){
      //Actualizo los valores de la ciudad, la profundidad y el costo
      parent->city = initial_city;
      parent->d = parent->d + 1;
      parent->g = parent->g + distance_matrix[parent->city][initial_city];

      //Guardo el valor de los resultados en el vector resultados
      if (parent->g < valorMinimo){
        valorMinimo = parent->g;
        ruta = parent->parent;      //Preguntar cómo puedo poner la ruta
      }


      //Impresión simple (depende de los printf's anteriores)
      //printf("[%d-%d](%f)",parent->city, parent->d, parent->g);

      //Impresión bkn:
      printf("Costo: %f. Recorrido: ",parent->g);
      while(parent->parent != NULL){
        printf("[%d]",parent->city);
        parent = parent->parent;
      }
      printf("\n");

    }
    //printf("\n");
  }

  printf("Valor mínimo: %f", valorMinimo);
  printf("Ruta: %d", ruta->parent);

}


//Main, donde se inicializan las cosas 
int main()
{
  //read the file and matrix generation
  ncities = 5;
  read_problem("./51.mtsp");
  distance_matrix_caculation();
  succ_matrix_caculation();

  //Llamar a búsqueda en anchura 
  initial_city = 0;
  BrFS();

  return 0;
}
