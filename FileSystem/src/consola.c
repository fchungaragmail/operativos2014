/*
 * consola.c
 *
 *  Created on: 2/5/2015
 *      Author: utnso
 */

#include "consola.h"
#include <string.h>
#include <commons/collections/dictionary.h>
#include <commons/string.h>

void initConsola();
void esperarComando();
void procesarEntrada(char[],bool*);
void mostrarAyuda();
void cerrarConsola();
t_dictionary* diccionarioComandos; //En este diccionario se guardan
							//todos los pares comando -> int
							//en initConsola se pueden ver todos

void execConsola()
{
	initConsola();
	char* menuInicial = "Bienvenido al FileSystem\n";
	system("clear");
	printf(menuInicial);
	esperarComando();
}

void esperarComando()
{
	char entrada[256];
	bool continuar;
	continuar = true;
	while(continuar)
	{
	scanf("%s", entrada);
	procesarEntrada(entrada,&continuar);
	}
}

void procesarEntrada(char entrada[],bool* continuar)
{
	char** comando = string_n_split(entrada,1," ");

	*continuar = true;
	int eleccion = dictionary_get(diccionarioComandos,comando[0]);
	switch (eleccion)
	{
	case 1:
	{
		mostrarAyuda();
		break;
	}
	case 2:
	{
		//Mover/renombrar archivo/directorio
		break;
	}
	case 3:
	{
		//Borrar archivo/directorio
		break;
	}
	case 4:
	{
		//Crear directorio
		break;
	}
	case 5:
	{
		//Importar archivo
		break;
	}
	case 6:
	{
		//Exportar archivo
		break;
	}
	case 7:
	{
		//Mostrar md5
		break;
	}
	case 90:
	{
		cerrarConsola();
		*continuar = false;
		break;
	}
	default:
	{
		printf("Comando desconocido.\n"
				"Ingrese 'help' para ver los comandos disponibles.\n");
		break;
	}
	}
}

void initConsola()
{
	//Aca vemos todos los comandos y sus integer
	//para el case de "procesarComandos()
	diccionarioComandos = dictionary_create();
	dictionary_put(diccionarioComandos,"help",1);
	dictionary_put(diccionarioComandos,"exit",90);
	dictionary_put(diccionarioComandos,"mv",2);
	dictionary_put(diccionarioComandos,"rm",3);
	dictionary_put(diccionarioComandos,"mkdir",4);
	dictionary_put(diccionarioComandos,"import",5);
	dictionary_put(diccionarioComandos,"export",6);
	dictionary_put(diccionarioComandos,"md5",7);
	dictionary_put(diccionarioComandos,"showb",8);
	dictionary_put(diccionarioComandos,"rmb",9);
	dictionary_put(diccionarioComandos,"cpb",10);
	dictionary_put(diccionarioComandos,"addn",11);
	dictionary_put(diccionarioComandos,"deln",12);
}

void mostrarAyuda()
{
	printf("\nComandos disponibles:\n"
			"help	Muestra el menu de ayuda\n"
			"exit	Sale de la consola\n"
			"mv 	Mover/renombrar arcivo/direcorio. Simil UNIX\n"
			"rm 	Borrar archivo/directorio. Simil UNIX\n"
			"mkdir	Crear directorio. Simil UNIX\n"
			"import	Importa un archivo al MDFS\n"
			"export	Exporta un archivo desde el MDFS\n"
			"md5	Mostrar md5 de un archivo\n"
			"showb	Muestra los bloques que componen un archivo\n"
			"rmb	Borra un bloque\n"
			"cpb	Copia un bloque\n"
			"addn	Aniadir nodo\n"
			"deln	Quitar nodo\n");
}

void cerrarConsola()
{
	printf("Saliendo de la consola...\n");
	dictionary_destroy(diccionarioComandos);
}
