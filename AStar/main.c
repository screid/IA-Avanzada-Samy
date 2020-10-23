//Cargo las librerias:
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/time.h>
#include <unistd.h>
#include <math.h>


//Defino un parámetro global LARGE (número muy grande) y un máximo de ciudades
#define LARGE 200000000.0
#define MAX_CITIES 1002
#define HEAPSIZE 100000 //(QUEUQSIZE)

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

//7. Contadores
int generated = 0;    //Cuántos nodos se generan
int expanded = 0;     //Cuántos nodos se expande


//Declaro estructuras de datos
struct node;      //Va a ser un subtour
typedef struct node node;

//Cuerpo de la estructura 
struct node{
  int city;   //ID del nodo
  int d;      //Profundidad, qué tan profundo está en el árbol.
  float g;      //Costo del subtour. Se acumula el valor del subtour en ese momento 
  float key;    //Clave. Por ahora es como el costo acumulado


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


//Función para hacer la heurística
double int_out(node *tmp)
{
  int i,j,added;
  double value=0;
  int cities_visited[MAX_CITIES];
  int depth = tmp->d;

  int city = tmp->city;

  for (i=0;i<ncities;i++){
  	cities_visited[i] = 0;
  }
  
  while(tmp->parent != NULL){
  	cities_visited[tmp->city] = 1;
  	tmp = tmp->parent;
  }
  cities_visited[initial_city] = 1;
    
  for (i=0; i<ncities;i++) 
    if (cities_visited[i]==0){
      int city_to_check = i;
      added = 0;
      for (j=1; j<ncities;j++){
		if (cities_visited[succ_matrix[city_to_check][j]] == 0 || succ_matrix[city_to_check][j] == city || succ_matrix[city_to_check][j] == initial_city){
			added++;
			value+=distance_matrix[city_to_check][succ_matrix[city_to_check][j]];
			if (added == 2) break;
		}
	  }
	}
	
    //value += distance_matrix[initial_city][succ_matrix[initial_city][1]];
    //value += distance_matrix[city][succ_matrix[city][1]];
    for (j=1; j<ncities;j++)
		if (cities_visited[succ_matrix[initial_city][j]] == 0){
			value += distance_matrix[initial_city][succ_matrix[initial_city][j]];
			break;
		}
    for (j=1; j<ncities;j++)
		if (cities_visited[succ_matrix[city][j]] == 0){
			value += distance_matrix[city][succ_matrix[city][j]];
			break;
		}

  return value/2;
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


//Crear procedimiento de Djikstra con prioridad

//Se crea el monticulo binario (Binary heap)
node *heap[HEAPSIZE];
int heapsize = 0;          //Si es 0 es que está vacío

//Aquí agrego un nodo, asumiendo que el heap parte vacío
void agregarHeap(node *nodoInsertado){

  //Variable para ver la posición inicial
  int current_pos = 0;
  int current_parent = 0;
  //Nodo auxiliar
  node *auxNode = NULL;
  //Inserto el nodo en la posición heapsize en el heap
  heap[heapsize] = nodoInsertado;

  heapsize++;

  //Si el índice es mayor que 1
  if (heapsize > 1){
      current_pos = heapsize - 1;
      current_parent = (int) (current_pos - 1) / 2;

      while (heap[current_parent]->key > heap[current_pos]-> key ){
        auxNode = heap[current_pos];
        heap[current_pos] = heap[current_parent];
        heap[current_parent] = auxNode;
        current_pos = current_parent;
        if (current_pos == 0){
          break;
        }
        current_parent = (int) (current_pos - 1) / 2;
      } 
  }
}  

//Suponemos que tenemos elementos
node* removerMejor(){
  node *nodoRetornado = heap[0];
  node *aux;
  double min = LARGE;

  //Si tenemos un unico elemento
  if (heapsize == 1){
    heapsize--;
    return nodoRetornado;
  }

  //Pusimos el elemento en la primera celda
  heap[0] = heap[heapsize - 1];
  heapsize--;     //Le disminuyo el largo de la cola en 1
  
  //Me queda comparar con los hijos
  int current_pos = 0;
  int pos_min;      //Variable para la posición mínima

  int izq = 2*current_pos +1;
  int der = 2*current_pos +2;

  //Si el nodo izquierdo no tiene hijos,
  if (izq >= heapsize){
    return nodoRetornado;
  }

  //Si el nodo sólo tiene hijo izquierdo,
  if (der >= heapsize){
    heap[der]->key = LARGE;
  }

  //Si el hijo izquierdo es mejor que el derecho
  if (heap[izq]->key < heap[der]->key ){
    pos_min = izq;
    min = heap[izq]->key;
  }
  else {
    pos_min = der;
    min = heap[der]->key;
  }

  //Mientras el padre es mayor que el mínimo. Aquí hay que mover el nodo en posición correcta en el heap
  while(heap[current_pos]->key > min){
    aux = heap[current_pos];
    heap[current_pos] = heap[pos_min];
    heap[pos_min] = aux;

    //Todo esto es para hacer el intercambio:
    current_pos = pos_min;
    izq = 2*current_pos +1;
    der = 2*current_pos +2;

    //Si el nodo izquierdo no tiene hijos,
    if (izq >= heapsize){
      break;
    }

    //Si el nodo sólo tiene hijo izquierdo,
    if (der >= heapsize){
      heap[der]->key = LARGE;
    }

    //Si el hijo izquierdo es mejor que el derecho
    if (heap[izq]->key < heap[der]->key ){
      pos_min = izq;
      min = heap[izq]->key;
    }
    else {
      pos_min = der;
      min = heap[der]->key;
    }
  }
  return nodoRetornado;
}

//Función para comprobar si la cola está vacía
int emptyHeap(){
  if (heapsize == 0) {
    return 1;     //Sí está vacía
  }
  else{
    return 0;
  }
}





void Astar(double w){

//1) Inicializo 4 datos de tipo nodo, hasta el momento vacíos 
  node *parent = NULL;  //Padre
  node *succ = NULL;    //Sucesor
  node *aux = NULL;     //Auxiliar
  node *state = NULL;   //Estado o nodo nuevo

  //Contador 
  int i;
  //Variable que después me dirá pa qué sirve 
  int city_avoid;

//2) Inicializamos el nodo inicial/padre ( o "A") 
  parent = create_node();
  //Le asociamos una ciudad al padre, la cual será el nodo inicial 
  parent->city = initial_city;
  //Le asociamos la profundidad al padre 
  parent->d = 0;
  //Le asociamos el costo acumulado 
  parent->g = 0.0;
  //Le asociamos el key
  parent->key = 0.0;
  //Imprimir el estado inicial:
  printf("[%d]\n",parent->city);

  printf("%f\n",int_out(parent));

//3) Inicializamos la cola.
  //En la posición final se agrega el padre y la pos. final aumenta en 1
  agregarHeap(parent);

  parent = removerMejor();    //Le elimino el mejor

  expanded++;

  //Se expande el estado inicial e insertaremos los nodos sucesores a la cola (Sólo para TSP)

  //Empiezo con 0 porque mi ciudad inicial es 0, si fuera otro valor se le cambia. Por lo tanto, empiezo desde la ciudad 1 en adelante.
  for(i = 1; i < ncities; i++){
    //Genero los sucesores (o los creo), junto con la ciudad, la profundidad y la distancia recorrida, además del padre 
    succ = create_node();    
    succ -> city = i;   //Asigno la ciudad
    succ -> d = parent->d + 1;
    succ -> parent = parent;
    succ -> g = parent->g + distance_matrix[parent->city][succ->city];
    succ -> key = succ->g + w*int_out(succ);

    //Imprimo info del succesor
    //printf("[%d-%d](%f)",succ->city, succ->d, succ->g);  

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
    agregarHeap(succ);  //Vamos agregando los sucesores 

  }
  //printf("\n");

//4) Procedimiento o ciclo de la búsqueda en anchura:
  //Se hace mientas la cola NO esté vacía
  while (!(emptyHeap()) ){

    //Hay que sacar el primer elemento de la cola, y luego avanzo
    parent = removerMejor();

    //Si llegamos al nodo de inicio, que me imprima toda la información
    if (parent->city == initial_city){
      printf("costo: %f, nodos expandidos: %d, estados generados: %d \n",parent->g, expanded,generated);
      while (parent != NULL){
        printf("[%d] \t",parent->city);
        parent = parent->parent;
      }
      break;
    }

    expanded++;

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
        generated++;
        //Genero los sucesores (o los creo), junto con la ciudad, la profundidad y la distancia recorrida, además del padre 
        succ = create_node();
        succ->city = state->city;   //Ahora la ciudad del sucesor es la ciudad del hermano 
        succ->d = parent->d + 1;
        succ->parent = parent;
        succ->g = parent->g + distance_matrix[parent->city][succ->city];
        succ->key = succ->g + w*int_out(succ);
        
        //Imprimo info del succesor
        //printf("[%d-%d](%f)",succ->city, succ->d, succ->g);   

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
        agregarHeap(succ);   //Vamos agregando los sucesores 

      }
      //Después de que termino de recorrer el nodo, tengo que ir al nodo hermano
      state = state->sibling;
    }
    //Si no tengo más hermanos, entonces significa que no tendré más hijos, por lo que puedo hacer un retorno al nodo inicial.
    if (aux == NULL){
      generated++;
      //Actualizo los valores de la ciudad, la profundidad y el costo
      succ = create_node();
      succ->city = initial_city;
      succ->d = parent->d + 1;
      succ->parent = parent;
      succ->g = parent->g + distance_matrix[parent->city][succ->city];
      succ->key = succ->g + w*int_out(succ);
      
      agregarHeap(succ);

    }
  }
}


//Main, donde se inicializan las cosas 
int main()
{
  //read the file and matrix generation
  ncities = 30;
  read_problem("./51.mtsp");
  distance_matrix_caculation();
  succ_matrix_caculation();

  //Llamar a búsqueda en anchura 
  initial_city = 0;
  double w=1.5;
  Astar(w);
  

  return 0;
}
