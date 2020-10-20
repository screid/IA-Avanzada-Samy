//Cargo las librerias:
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/time.h>
#include <unistd.h>
#include <math.h>

//Defino parámetros globales?(preguntar)
#define max(x,y) ( (x) > (y) ? (x) : (y) )
#define min(x,y) ( (x) < (y) ? (x) : (y) )

//Defino un parámetro global LARGE (número muy grande) y un máximo de ciudades
#define LARGE 10000000.0
#define MAX_CITIES 1002

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
//7. Defino un upper y un lowerbound 
double upperbound = LARGE;
double lowerbound;
//8. Define un número de ciudades visitadas
short cities_visited[MAX_CITIES];
//9. Me imagino que son variables auxiliares
long long int expanded = 0;
long long int pruned = 0;
long long int touched = 0;
double w = 0;




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


//Función para generar los nodos de manera ordenada 
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


//Función heurística (estimar el costo de un tour/cota inferior)
//Link: http://citeseerx.ist.psu.edu/viewdoc/download;jsessionid=34E3783104AC1F84D786A71AA1127AD2?doi=10.1.1.331.962&rep=rep1&type=pdf
double i_int_out(short cities_visited[MAX_CITIES],int city, int missing_cities) 
{
  int i,j,added;
  double value=0;
  if (missing_cities == 1) return distance_matrix[city][initial_city]; 
  for (i=0; i<ncities;i++) 
    if (i!=city && cities_visited[i]==0){ 
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


//Función heurística anterior con una pequeña mejora.
double int_out(short cities_visited[MAX_CITIES],int city, int missing_cities) 
{
  int i,j,added;
  double value=0;
  if (missing_cities == 1) return distance_matrix[city][initial_city]; 
  for (i=0; i<ncities;i++) 
    if (i!=city && cities_visited[i]==0){
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
  value += distance_matrix[initial_city][succ_matrix[initial_city][1]];
  value += distance_matrix[city][succ_matrix[city][1]];
  
  return value/2;
}


//Función para algoritmo Deep First-Branch and Bound para UN NODO (o LA ciudad). Esta necesita (en este orden): LA ciudad, vector de ciudades visitadas (cuántas veces se han visitado las ciudades), el número de ciudades que faltan por visitar, el valor del largo de la ruta y el tour (ruta) para todas las ciudades  
void DFBnB(int city,
	    short cities_visited[MAX_CITIES],
	    int missing_cities,
	    double path_value, 
	    int tour[MAX_CITIES]) {

//1) Declaro variables auxiliares:
  int i,succ;
  double path_values[MAX_CITIES],g,h=0;
  
//2)Cambio los estados de LA ciudad. Cuando la ciudad entra:
  //Se disminuye la ciudades faltantes en 1.
  missing_cities--;
  //Al tour se le agrega LA ciudad
  tour[missing_cities] = city;
  //La cantidad de veces que se visito LA ciudad cambia a 1 (fue visitada).
  cities_visited[city] = 1;
  
//3)Si ya no me quedan más ciudades faltantes (es decir, todas fueron visitadas)
  if (missing_cities==0) {
    //Se calcula el largo de la ruta como el valor que ya se tenía más el valor desde ese nodo final al nodo inicial.
    path_value = path_value + distance_matrix[city][initial_city];

    //Se va guardando el valor mínimo
    if (path_value < upperbound) {
      upperbound = path_value;

      //Así puedo imprimir el tour
      printf("Ruta:\t");
      for (i = ncities-1; i > -1 ;i--){
        printf("[%d]",tour[i]);
      }

      printf("\n Valor tour : %f  expanded:%lld touched:%lld pruned:%lld\n",upperbound,expanded,touched,pruned);
    }
  }

//4) Si aún faltan ciudades por visitar 
  else {
    //Aumento el contador para número de ciudades que EL nodo ha visitado otras ciudades (o se ha expandido)
    expanded++;   

    //Recorro todas las ciudades
    for (i = 0; i < ncities;i++) {

      //Aumento el contador (PREGUNTAR: para qué es este valor?)
      touched++;
      //El sucesor va a ser el que le siga directamente a LA ciudad.
      succ = succ_matrix[city][i];

      //Si el sucedor de LA ciudad ya fue visitado (es decir, 1), entonces lo paso de largo con el continue . Por lo tanto, pasa directamente a otro sucesor.
      if (cities_visited[succ]) continue;

      //Le asigno el valor de la ruta a los sucesores i
      path_values[i] = path_value;
      //Le calculo la distancia recorrida desde LA ciudad hasta el sucesor i
      g = path_values[i] + distance_matrix[city][succ];
      //Valor calculado por heurística
      //h = int_out(cities_visited,succ,missing_cities);
      h = i_int_out(cities_visited,succ,missing_cities); //Versión mejorada
      
      //Si ambos valores son menores que el upperbound (que inicialmente es enorme)
      if (g + w*h < upperbound){
        //El largo de la ruta hasta el nodo sucesor i va a ser la distancia actual + la distancia recorrida  	 	
        path_values[i] = path_values[i] + distance_matrix[city][succ];
        //Se vuelve a llamar a la función (se hace de manera recursiva)
        DFBnB(succ,cities_visited,missing_cities,path_values[i],tour);
      }
      else {
        //Sino, se va contando las veces que no hubo mejora (es decir, que se PODÓ)
        pruned++;
      }

    }
  }
  cities_visited[city] = 0;
  return;
}


//Función para algoritmo Deep First-Branch and Bound mejorado
void DFBnB_Driver(double w,int initial_city)
{
//1) Inicializo variables 
  int i;
  //Al inicial, no he recorrido ninguna ciudad 
  int missing_cities = ncities; 
  //Guarda la mejor solución que he encontrado hasta el momento 
  int tour[MAX_CITIES];   
  //Inicializo recorrido en 0
  double path_value = 0;

//2) Inicializo las ciudades visitadas. Seteo cada ciudad visitada en 0 para no volver a visitarla. Además, inicializo tour, para todos los valores (ciudades), en -1
  for (i = 0; i < ncities;i++) {
    cities_visited[i] = 0;
    tour[i] = -1; 
  }

  //Llamo a la función 
  DFBnB(initial_city,cities_visited,missing_cities,path_value,tour);
  printf("\n %f %lld %lld %lld\n",upperbound,expanded,touched,pruned);//getchar();
  return;
}

//Main, donde se inicializan las cosas 
int main()
{
  //read the file and matrix generation
  ncities = 10;
  read_problem("./51.mtsp");
  distance_matrix_caculation();
  succ_matrix_caculation();
  
  // Llamar a algoritmo de búsqueda en profundidad 
  w = 1; 
  initial_city=0;
  DFBnB_Driver(w,initial_city);

  return 0;
}
