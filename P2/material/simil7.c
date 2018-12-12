
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ctype.h>
#include <omp.h>

#define VERDADERO 1
#define FALSO 0
#define MAX_LON 2500

// Cargar un fichero en memoria, rellenando ptabla con la lista de sus lineas
// Si *ptabla != NULL, hay que liberar memoria después
// ----
// Load a file into memory, filling ptabla with the list of its lines
// If *ptabla != NULL, memory will have to be freed later on
int lee_lineas(char ruta[],int *pn,char ***ptabla)
{ int i,n;
  FILE *f;
  struct stat s;
  char *p,**q;
  size_t l;

  *ptabla = NULL;

  if ( stat(ruta,&s) != 0 ) {
    fprintf(stderr,"Error al obtener información del fichero \"%s\"\n",ruta);
    return 1;
  }

  l = s.st_size;
  p = malloc(l*sizeof(char));
  if ( p == NULL ) {
    fprintf(stderr,"Error al pedir memoria para texto\n");
    return 2;
  }

  f = fopen(ruta,"r");
  if ( f == NULL ) {
    free(p);
    fprintf(stderr,"Error al abrir el fichero \"%s\"\n",ruta);
    return 3;
  }
  fread(p,1,l,f);
  fclose(f);

  n = 0;
  for ( i = 0 ; i < l ; i++ ) {
    if ( iscntrl(p[i]) ) {
      ++n;
      do ++i; while ( i < l && iscntrl(p[i]) );
    }
  }
  *pn = n;

  q = malloc(n*sizeof(char*));
  if ( q == NULL ) {
    free(p);
    fprintf(stderr,"Error al pedir memoria para punteros\n");
    return 4;
  }
  *ptabla = q;

  q[0] = p;
  n = 0;
  for ( i = 0 ; i < l ; i++ ) {
    if ( iscntrl(p[i]) ) {
      p[i] = '\0';
      ++n;
      do ++i; while ( i < l && iscntrl(p[i]) );
      if ( i < l ) q[n] = &p[i];
    }
  }

  return 0;
}

// Free lines
void libera_lineas(char *tabla[])
{
  free(tabla[0]);// texto (text)
  free(tabla);   // punteros (pointers)
}

// Escribir los códigos (o sus indices) en el fichero de salida
// ---
// Write the codes (or their indexes) into the output file
void escribe_resultado(char ruta[],char *tabla[],int n,int ps[],int numeros)
{ int i;
  FILE *f;

  if (ruta == NULL) f = stdout;
  else {
    f = fopen(ruta,"w");
    if ( f == NULL ) {
      fprintf(stderr,"Error al escribir el fichero \"%s\"\n",ruta);
      return;
    }
  }

  if ( numeros ) {
    for ( i = 0 ; i < n ; i++ )
      fprintf(f,"%d\n",1+ps[i]);
  } else {
    for ( i = 0 ; i < n ; i++ )
      fprintf(f,"%s\n",tabla[ps[i]]);
  }

  if (ruta != NULL) fclose(f);
}

// Calcular la distancia entre dos cadenas
// ---
// Compute the distance between two strings
#define MIN3(a,b,c) ( (a)<=(b) && (a)<=(c) ? (a) : (b)<=(c) ? (b) : (c) )
int distancia(char s[],char t[])
{ int i,j,a,b,d[MAX_LON];
  if ( t[0] == '\0' ) return strlen(s);
  for ( j = 1 ; t[j] != '\0' ; j++ ) d[j] = j + 1;
  a = 1; d[0] = s[0] == t[0] ? 0 : 1;
  i = 0;
  while ( s[i] != '\0' ) {
    for ( j = 1 ; t[j] != '\0' ; j++ ) {
      b = d[j];
      d[j] = ( s[i] == t[j] ? a : MIN3(d[j-1],a,b) + 1 );
      a = b;
    }
    i++;
    a = d[0]; d[0] = s[i] == t[0] ? i : a + 1;
  }
  return d[j-1];
}

// Buscar para cada código de la tabla2 el código más parecido de tabla1
// ---
// For each code in tabla2, look for the most similar code in tabla1
void busca_mas_parecidos(int nt1,char *tabla1[],int nt2,char *tabla2[],int ps[])
{ int i,j,d,im,dm;// dm es la distancia del código más parecido, im su índice// dm is the distance of the most similar code, im is its index

  #pragma omp for private(d,dm,im,i,j)

    for ( j = 0 ; j < nt2 ; j++ ) {
      dm = MAX_LON + 1;
      
      
      for ( i = 0 ; i < nt1 ; i++ ) {
	d = distancia(tabla1[i],tabla2[j]);
	if ( d < dm ) {
	  dm = d;
	  im = i;
	}
      }
      ps[j] = im;

    }
}

int main(int argc,char *argv[])
{ int err,err2,nt1,nt2,*ps,numeros,j;
  double t1, t2;
  char **tabla1,**tabla2,*salida;

  if ( argc > 1 && argv[1][0] == '+' ) {// escribir los códigos (write the codes)
    argv++; argc--;
    numeros = FALSO;
  } else numeros = VERDADERO;// o sólo su posición (or only their positions)

  if ( argc < 3 ) {
    fprintf(stderr,"Opciones: [+] fich_datos1 fich_datos2 [fich_salida]\n");
    return 1;
  }

  salida = argc < 4 ? NULL : argv[3];
  #pragma omp parallel sections 
  {
	#pragma omp section
  		err  = lee_lineas(argv[1],&nt1,&tabla1);

  	#pragma omp section 
  		err2 = lee_lineas(argv[2],&nt2,&tabla2);

}
  int hilos,idHilo;
  #pragma omp parallel 
  {
    if(omp_get_thread_num() == 0) {
      hilos = omp_get_num_threads();
    }
  
  }
  printf("Nº de hilos: %d\n", hilos);
  
  
  
  //EJERCICIO 7---------------------------------------------------------------------------------------
  //Definir arrays con valor minimo, máximo, contador y medio de cada hilo
  volatile int hiloMin[hilos],hiloMax[hilos],cont[hilos];  
  volatile double hiloMed[hilos];
  
  //Abrir zona paralela
  #pragma omp parallel 
  {
   //Asignar valor inicial para cada posición de los arrays max, min y cont.
   hiloMin[omp_get_thread_num()] = strlen(tabla2[omp_get_thread_num()]);
   hiloMax[omp_get_thread_num()] = strlen(tabla2[omp_get_thread_num()]);
   cont[omp_get_thread_num()] = 0;
   //Clausula OpenMP for
   #pragma omp for schedule(static,1)
   for(j = 0; j < nt2; j++) {
      if(strlen(tabla2[j])<hiloMin[omp_get_thread_num()]) { hiloMin[omp_get_thread_num()] = strlen(tabla2[j]); }
      if(strlen(tabla2[j])>hiloMax[omp_get_thread_num()]) { hiloMax[omp_get_thread_num()] = strlen(tabla2[j]); }
      hiloMed[omp_get_thread_num()] = hiloMed[omp_get_thread_num()] + strlen(tabla2[j]);
      cont[omp_get_thread_num()]++; 
   }
   //Cada hilo muestra por consola sus valores
   hiloMed[omp_get_thread_num()] = hiloMed[omp_get_thread_num()] / cont[omp_get_thread_num()];
   printf("Hilo %d de %d: Longitud mín/med/max: %d/%0.2f/%d\n",omp_get_thread_num(),hilos,hiloMin[omp_get_thread_num()],hiloMed[omp_get_thread_num()],hiloMax[omp_get_thread_num()]);
  }
  //Declarar variables con los valores totales.
  int valorMax, valorMin;
  double valorMed;
  valorMax = 0;
  valorMin = hiloMin[0];
  valorMed = 0;
  //Calcular dichos valores
  for(j = 0; j < hilos; j++) {
    if(valorMax < hiloMax[j]) { valorMax = hiloMax[j]; }
    if(valorMin > hiloMin[j]) { valorMin = hiloMin[j]; }
    valorMed = valorMed + hiloMed[j];
    
  }
  //Mostrar por pantalla el total
  valorMed = valorMed / hilos;
  printf("TOTAL: Longitud mín/med/max: %d/%0.2f/%d\n",valorMin,valorMed,valorMax);


//FIN EJERCICIO 7--------------------------------------------------------------------------------------------  
  if ( err ) err = 2;
  else if ( err2 ) err = 3;
  else {

    ps = malloc(nt2*sizeof(*ps));
    if ( ps == NULL ) {
      err = 4;
      fprintf(stderr,"Error pidiendo memoria para el resultado\n");
    } else {
      t1 = omp_get_wtime();
      
      #pragma omp parallel 
      {
	hilos = omp_get_num_threads();
	idHilo = omp_get_thread_num();
      
	busca_mas_parecidos(nt1,tabla1,nt2,tabla2,ps);
	
      }
      t2 = omp_get_wtime();

      printf("Tiempo: %f\n", t2-t1);
      escribe_resultado(salida,tabla1,nt2,ps,numeros);

      free(ps);

    }

  }

  if ( tabla2 != NULL ) libera_lineas(tabla2);
  if ( tabla1 != NULL ) libera_lineas(tabla1);

  return err;
}

