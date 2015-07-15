
#include"LecturaYEscritura.h"
#include <stdint.h>

/*
* Lee datos del socket. Supone que se le pasa un buffer con hueco suficiente para los datos. Devuelve el numero de bytes leidos o
* 0 si se cierra fichero o -1 si hay error.
*/
int32_t leer_socket (int32_t nuevo_socket, char *buffer, size_t size)
{
	size_t leido = 0;
	size_t aux = 0;

	/*Comprobacion de que los parametros de entrada son correctos*/

	if ((nuevo_socket == -1) || (buffer == NULL) || (size < 1))
		return -1;

	/* Mientras no hayamos leido todos los datos solicitados*/
	while (leido < size){
		aux = recv(nuevo_socket, buffer + leido, size - leido, 0);
		if (aux > 0){
			/*
			* En caso de leer datos, se incrementa la variable que contiene los datos leidos hasta el momento
			*/
			leido = leido + aux;
		}
		else{
			/*
			* Si read devuelve 0, es que se cerro el socket. Se devuelven los caracteres leidos hasta ese momento
			*/
			if (aux == 0)
			{
				return leido;
			}
			if (aux == -1){
				/*
				* En caso de error:
				* EINTR se produce hubo una  interrupcion del sistema antes de leer ningun dato. No
				* es un error realmente.
				* EGAIN significa que el socket no esta disponible por el  momento.
				* Ambos errores se tratan con una espera de 100 microsegundos y se vuelve a intentar.
				* El resto de los posibles errores provocan que salgamos de la funcion con error.
				*/

				//Mover la logica de señales al Sig_handler

				switch (errno){
					case EINTR:
					case EAGAIN:
						usleep (100);
						break;
					default:
						return -1;
				}
			}
		}
	}
	/*
	* Se devuelve el total de los caracteres leidos
	*/
	return leido;
}

/*
* Escribe dato en el socket cliente. Devuelve numero de bytes escritos, o -1 si hay error.
*/
int32_t escribir_socket (int32_t nuevo_socket, char *datos, size_t longitud)
{
	size_t escrito = 0;
	size_t aux = 0;

	/*
	* Comprobacion de los parametros de entrada
	*/
	if ((nuevo_socket == -1) || (datos == NULL) || (longitud < 1))
		return -1;

	/*
	* Bucle hasta que hayamos escrito todos los caracteres que se indicaron.
	*/
	while (escrito < longitud){

		aux = send(nuevo_socket, datos + escrito, longitud - escrito, 0);

		if (aux > 0){
			/*
			* Si se consiguio escribir caracteres, se actualiza la variable escrito
			*/
			escrito = escrito + aux;
		}
		else
		{
			/*
			* Si se cerro el socket, devolvemos el numero de caracteres leidos.
			* Si hubo un error, devuelve -1
			*/
			if (aux == 0){
				return escrito;
			}
			else
			{
				return -1;
			}
		}
	}

	/*
	* Devolvemos el total de caracteres leidos
	*/
	return escrito;
}

/*
 * Deprecado


/*Para recibir un paquete, se pasa como parametro el socket en el que se va esperar, un puntero a un tipo header
 Si falla devuelve un valor negativo

int32_t recibirPaquete(int32_t enlace,t_header* header_a_recibir)
{

	int leido;
	if((leido = leer_socket(enlace, (char*)header_a_recibir, TAM_HEADER)) <= 0)
		return 0;

	if(header_a_recibir->size > 0)
		header_a_recibir->data = malloc(header_a_recibir->size);
		if((leido += leer_socket(enlace, header_a_recibir->data,header_a_recibir->size)) > 0)
			return leido;
		else //cerro socket
			return 0;
}

int leerCodigoPaquete(int socket, int *codigo);
/*Lee del socket un entero y retorna la cantidad de bytes leidos, -1 error, 0 cerro socket
int leerCodigoPaquete(int socket, int *codigo){
	return read(socket, codigo, sizeof(int));
}

/*Para enviar un paquete, se pasa como parametro a quien va dirigido, junto con el tipo header y data que se van a enviar
 * Si falla devuelve un valor negativo
int32_t enviar_paquete(int32_t enlace,t_header header_a_enviar)
{
	int tamanioPaquete = TAM_HEADER + header_a_enviar.size;
	void* paquete = malloc(tamanioPaquete);
	memcpy(paquete, &header_a_enviar, TAM_HEADER);
	if(header_a_enviar.size > 0)
		memcpy(paquete + TAM_HEADER, header_a_enviar.data, header_a_enviar.size);
	int escrito;
	if ((escrito = write(enlace, paquete, tamanioPaquete)) == -1){
		perror("");
		puts("error al enviar paquete");
		return -1;
	}
	return escrito;


	/*
	int32_t res;
	size_t size_t_header;

	size_t_header = sizeof(t_header) - 4;//solo id|size

	if (enlace != 0)
	{
		res = escribir_socket(enlace,(char*)&header_a_enviar, size_t_header);
		if(res != size_t_header)
		{
			return -1;
		}
		if(header_a_enviar.size != 0)
		{
		puts("C");
			res = escribir_socket(enlace,header_a_enviar.data,header_a_enviar.size);
		puts("D");
			if(res != header_a_enviar.size)
			{
				return -1;
			}
		}
	}
	else
	{
		res = -1;
	}

	return res; 		//Devuelvo el size de lo enviado
}
	*/
