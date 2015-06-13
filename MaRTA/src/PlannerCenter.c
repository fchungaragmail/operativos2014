/*
 * PlannerCenter.c
 *
 *  Created on: 23/5/2015
 *      Author: utnso
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "FilesStatusCenter.h"
#include "Utilities.h"
#include <commons/temporal.h>
#include <commons/string.h>
#include "Serializador.h"


// Constantes
#define MaxPedidosEnRed 5


// Variables Globales
int jobSocket;
int cantidadDeBloquesAProcesar;
int cantidadDeBloquesProcesados;
int cantidadDePedidosAlFS;

//-->PedidoRealizado
char *pedidoRealizado_Nodo;
char *pedidoRealizado_Bloque;
char *pedidoRealizado_Path;
StatusBlockState *pedidoRealizado_TipoPedido;
char *pedidoRealizado_PathArchTemporal;

// Funciones privadas
int obtenerIdParaComando(Message *recvMessage);
Message* planificar(Message *recvMessage,TypesMessages type);
void actualizarPedidoRealizado(char *bloque, char* ipnodo, char* path, char *pathTemporal, StatusBlockState tipo );
Message* obtenerProximoPedido(Message *recvMessage);
bool* obtenerRequestResponse(Message *recvMessage,TypesMessages type);
t_dictionary *obtenerCopiaDeConMenosCarga(Message *recvMessage,char *path,int bloqueNro);
char* crearPathTemporal(char *path);
Message* armarMensajeParaEnvio(Message *recvMessage,void *stream,char *comando);
void actualizarTablas_RtaDeMapExitosa(Message *recvMessage);
Message *createFSrequest();

// Funciones publicas
void initPlannerCenter();
void processMessage(Message *recvMessage);

void initPlannerCenter()
{
	initFilesStatusCenter();
	jobSocket=0;
	cantidadDeBloquesAProcesar=0;
	cantidadDeBloquesProcesados=0;
	cantidadDePedidosAlFS=0;
	pedidoRealizado_TipoPedido = malloc(sizeof(StatusBlockState));
}

void processMessage(Message *recvMessage)
{
	int comandoId = obtenerIdParaComando(recvMessage);
	switch (comandoId) {
		case K_NewConnection:
			printf("PlannerCenter : planificar NewConnection\n");
			addNewConnection(recvMessage->sockfd);
			jobSocket=recvMessage->sockfd;
			printf("***************\n");
			break;
		case K_Job_NewFileToProcess:

			printf("PlannerCenter : planificar Job_NewFileToProcess:\n");

			char* filePath = deserializeFilePath(recvMessage,K_Job_NewFileToProcess);
			bool *soportaCombiner = deserializeSoportaCombiner(recvMessage);
			addNewFileForProcess(filePath,soportaCombiner,recvMessage->sockfd);	//se agrega un file_StatusData a
																				//filesToProcessPerJob
			actualizarPedidoRealizado(NULL,NULL,filePath,NULL,K_Pedido_FileData);
			Message *sendMessage = planificar(recvMessage,K_Job_NewFileToProcess);
			//enviar(sendMessage->sockfd,sendMessage->mensaje);

			free(filePath);

			printf("***************\n");
			break;

		case K_FS_FileFullData:

			printf("PlannerCenter : planificar FS_FileFullData\n");
			//FS me responde con el pedido de datos que le hice.

			bool *response = deserializeRequestResponse(recvMessage,K_FS_FileFullData);

			if(!(*response)){
				//si es false entonces no esta disponible el archivo
				//QUE HACER ???????
				printf("el pedido al FS del archivo %s fallo, MaRTA no puede continuar la operacion\n",pedidoRealizado_Path);
				printf("no se pudo procesar la parte nro : %d \n",cantidadDeBloquesProcesados);
			}

			if(*response){

				Message *planifiedMessage = planificar(recvMessage,K_FS_FileFullData);
				//enviar(planifiedMessage->sockfd,planifiedMessage->mensaje);
			}
			free(response);

			printf("***************\n");
			break;

		case K_Job_MapResponse:
			printf("PlannerCenter : planificar Job_MapResponse\n");

			Message *planifiedMsj = planificar(recvMessage,K_Job_MapResponse);
			//enviar(planifiedMsj->sockfd,planifiedMsj->mensaje);

			printf("***************\n");
			break;
		case K_Job_ReduceResponse:
			printf("PlannerCenter : planificar Job_ReduceResponse");

			bool *_response = deserializeRequestResponse(recvMessage,K_Job_ReduceResponse);

			if(*_response){
				//reduce realizado con exito
				//actualizar tablas !!
				actualizarTablas_RtaDeMapExitosa(recvMessage);
				printf("archivo %s reducido con exito !\n",pedidoRealizado_Path);

			}else{

				//QUE HACER SI FALLA EL REDUCE ???

			}
			free(_response);

			printf("***************\n");
			break;
		default:

			printf("PlannerCenter: ERROR !! Comando no identificado !!\n");
			printf("***************\n");

			break;
	}
	free(recvMessage);
}

int obtenerIdParaComando(Message *recvMessage)
{
	char *comando = deserializeComando(recvMessage);
	TypesMessages type;
	if(strcmp(comando,"newConnection")==0){type =  K_NewConnection;}
	if(strcmp(comando,"archivoAProcesar")==0){type = K_Job_NewFileToProcess;}
	if(strcmp(comando,"mapFileResponse")==0){type = K_Job_MapResponse;}
	if(strcmp(comando,"reduceFileResponse")==0){type = K_Job_ReduceResponse;}
	if(strcmp(comando,"DataFileResponse")==0){type = K_FS_FileFullData;}

	free(comando);
	return type;
}

//Planificacion
Message* planificar(Message *recvMessage,TypesMessages type)
{
	char *path = pedidoRealizado_Path;

	if(type == K_Job_NewFileToProcess){
		//pido al FS la tabla de direcciones del archivo
		//segun protocolo ---> -Comando: "DataFile rutaDelArchivo" /// -Data: Vacio
		Message *fsRequest = createFSrequest();
		return fsRequest;
	}

	if(type==K_FS_FileFullData){

		//ACTUALIZO TABLAS
		char *path = deserializeFilePath(recvMessage,K_FS_FileFullData);

		int nroDeBloques = deserializarFullDataResponse_nroDeBloques(recvMessage);
		int nroDeCopias = deserializarFullDataResponse_nroDeCopias(recvMessage);
		t_list *listaPadreDeBloques = deserializarFullDataResponse(recvMessage);
		cantidadDeBloquesAProcesar=nroDeBloques;

		if(cantidadDePedidosAlFS==0){
			addFileFullData(jobSocket, path,nroDeBloques,nroDeCopias,listaPadreDeBloques);//se completa filesToProcess y se crea un fileState
		}

		if(cantidadDePedidosAlFS>0){
			//reload fullData
			reloadFileFullData(jobSocket, path,nroDeBloques,nroDeCopias,listaPadreDeBloques);
		}
		//obtengo proximoPedido CON INFO ACTUALIZADA
		Message *sendMessage = obtenerProximoPedido(recvMessage);

		return sendMessage;
	}

	if(type==K_Job_MapResponse){

		//*data:sizeRutaArchivoTemporal-rutaArchivoTemporal-Respuesta
		bool *requestResponse = deserializeRequestResponse(recvMessage,K_Job_MapResponse);

		if(*requestResponse){

			//actualizo tablas y obtengo prox pedido
			printf("pedido de map realizado con exito, se mapeo la parte nro %d \n",cantidadDeBloquesProcesados);
			actualizarTablas_RtaDeMapExitosa(recvMessage);
			Message *sendMessage = obtenerProximoPedido(recvMessage);
			return sendMessage;
		}

		if(!(*requestResponse)){
			//REPLANIFICAR
			printf("fallo pedido de map del archivo %s al nodo %s bloque %s \n",pedidoRealizado_Path,pedidoRealizado_Nodo,pedidoRealizado_Bloque);

			//actualizar tablas y reenviar si existen copias
			char *IPnroNodo =  pedidoRealizado_Nodo;
			char *nroBloque =  pedidoRealizado_Bloque;
			//PONER -1 EN COPIAS
			darDeBajaCopiaEnBloqueYNodo(path,recvMessage->sockfd,nroBloque,IPnroNodo,cantidadDeBloquesProcesados);
			decrementarOperacionesEnProcesoEnNodo(IPnroNodo);

			//OBTENER PROXIMO PEDIDO (se va a enviar devuelta el mismo, siempre y cuando haya copias disponibles)
			Message* sendMessage = obtenerProximoPedido(recvMessage);
			free(requestResponse);
			return sendMessage;
		}
	}

	if(type==K_Job_ReduceResponse){

		//*data:sizeRutaArchivoTemporal-rutaArchivoTemporal-sizeRespuesta-Respuesta
		bool *requestResponse = deserializeRequestResponse(recvMessage,K_Job_ReduceResponse);

		if(!(*requestResponse)){
			//re planificar
			// ver que hacer si el reduce falla !!
		}
	}

	Message *error; //no deberia nunca llegar aca
	return error;
}

Message* obtenerProximoPedido(Message *recvMessage)
{
	char *path = pedidoRealizado_Path;
	t_dictionary *fileState = getFileStateForPath(path);

	int size_fileState= dictionary_get(fileState,K_FileState_size);
	t_list *blockStateArray = dictionary_get(fileState,K_FileState_arrayOfBlocksStates);

	int i;
	for(i=0;i<size_fileState;i++){//itero en fileState hasta encontrar el siguiente a mappear

		t_dictionary *blockState = list_get(blockStateArray,i);//(*blockStateArray)[i];
		char *nroNodo = dictionary_get(blockState,K_BlockState_nroNodo);
		StatusBlockState *statusBlock = dictionary_get(blockState,K_BlockState_state);

		if(((*statusBlock)==K_MAPPED) && i==(size_fileState-1)){
			//estan todos mappeados , inculuidos el ultimo
			printf("estan todos mapeados ! \n");

			//Actualizo estados
			actualizarPedidoRealizado(NULL,NULL,NULL,NULL,K_IN_REDUCING);

			//HACER REDUCE !!! --> usar fileState para ver las ubicaciones
			//VER SI SOPORTA COMBINER O NO
			//hacer reduce en el nodo que contenga mas archivos mappeados

			bool *tieneCombinerMode = soportaCombiner(recvMessage->sockfd,path);
			char *IPnroNodoLocal = obtenerNodoConMayorCantidadDeArchivosTemporales(path);//falla

			int cantidadDeNodos = obtenerCantidadDeNodosDiferentesEnBlockState(path);
			t_list *nodosEnBlockState = obtenerNodosEnBlockStateArray(path);

			char *pathTemporalLocal = crearPathTemporal(path);

			// iniciar el serializado --> nroNodoLocal-pathTemporalLocal
			char *stream = createStream();
			addStringToStream(&stream,IPnroNodoLocal);
			addStringToStream(&stream,pathTemporalLocal);

			if(*tieneCombinerMode){
				//planificar con combiner

			}

			if(!(*tieneCombinerMode)){
				//planificar sin combiner

				//****************************************************
				//PRIMERO PONER LOS PATH CORRESPONDIENTES AL NODO LOCAL

				t_list *pathsTemporalesParaNodoLocal = obtenerPathsTemporalesParaNodo(path, IPnroNodoLocal);
				int cantidadDePathsTempEnNodoLocal = obtenerCantidadDePathsTemporalesEnNodo(path,IPnroNodoLocal);
				int k;
				for(k=0;k<cantidadDePathsTempEnNodoLocal;k++){

					char *tempPath = list_get(pathsTemporalesParaNodoLocal,k);
					//agregar a el serializado "tempPath"
					addStringToStream(&stream,tempPath);
				}
				//*****************************************************
				int i;
				for(i=0;i<cantidadDeNodos;i++){
					char *IPnodoEnBlockState = list_get(nodosEnBlockState,i);

					if( strcmp(IPnodoEnBlockState,IPnroNodoLocal) != 0 ){
						int cantidadDePathsTempEnNodo = obtenerCantidadDePathsTemporalesEnNodo(path,IPnodoEnBlockState);
						t_list *pathsTemporalesParaNodo = obtenerPathsTemporalesParaNodo(path, IPnodoEnBlockState);

						//agregar a el serializado --> "cantidadDePathsTempEnNodo"
						addIntToStream(stream,cantidadDePathsTempEnNodo,K_int16_t);

						int j;
						for(j=0;j<cantidadDePathsTempEnNodo;j++){

							char *tempPath = list_get(pathsTemporalesParaNodo,j);
							//agregar a el serializado "tempPath"
							addStringToStream(&stream,tempPath);
						}
					}
				}
			}

			Message *sendMessage = armarMensajeParaEnvio(recvMessage,stream,"reduceFile");
			printf("el stream q se va a enviar es : %s\n",stream);
			return sendMessage;
		}

		if((*statusBlock)==K_UNINITIALIZED){ //este bloque no esta procesado

			//obtengo conjunto (#nodo;#bloque) con menos carga de operaciones en #nodo
			t_dictionary *copiaConMenosCarga = obtenerCopiaDeConMenosCarga(recvMessage,path,i);
			char *IPnroDeNodo = dictionary_get(copiaConMenosCarga,K_Copia_IPNodo);

			if( strcmp(IPnroDeNodo,K_Copia_DarDeBajaIPNodo) == 0){//checkeo q haya copias disponibles

				//NO HAY COPIAS DISPONIBLES !!!!!
				if(cantidadDePedidosAlFS==0){
					printf("no hay mas copias disponibles!!!\n");
					printf("hacer pedido FS_FileFullData aver si al FileSystem se le cargo nuevos nodos !\n");
					Message *fsRequest = createFSrequest();
					cantidadDePedidosAlFS++;
					return fsRequest;
				}else{
					printf("no hay mas copias disponibles!!!\n");
					printf("ya se le hizo un pedido al FS previamente, MaRTA no puede continuar con la operacion \n");
					//HACER UN RETURN ACA !!!
				}
			}

			//armo pedido de map, segun protocolo es

			//*comando: "mapFile"
			//*data:sizeDireccionNodo-direccionNodo-nroDeBloque-...
			//...-sizeRutaArchivoTemporal-rutaArchivoTemporal
			//VIEJO !!!!!

			char *path_with_temporal = crearPathTemporal(path);
			char *ipNodo = dictionary_get(copiaConMenosCarga,K_Copia_IPNodo);
			char *nroDeBloque = dictionary_get(copiaConMenosCarga,K_Copia_NroDeBloque);

			char *stream = createStream();
			addStringToStream(&stream,ipNodo);
			addStringToStream(&stream,path_with_temporal);

			printf("la copia con menos carga tiene nroDeBloque %s \n",nroDeBloque);

			Message *msjParaEnviar = armarMensajeParaEnvio(recvMessage,stream,"mapFile");

			//ACTUALIZAR PedidoRealizado, NODOState y BLOCKState
			actualizarPedidoRealizado(nroDeBloque, ipNodo, path, path_with_temporal, K_IN_MAPPING);
			incrementarOperacionesEnProcesoEnNodo(ipNodo);

			//actualizoBlockState---> MAL !!!!! ASI NO SE ACTUALIZA !!!!
			dictionary_put(blockState,K_BlockState_state,K_IN_MAPPING);
			dictionary_put(blockState,K_BlockState_nroNodo,ipNodo);
			dictionary_put(blockState,K_BlockState_nroBloque,nroDeBloque);
			dictionary_put(blockState,K_BlockState_temporaryPath,path_with_temporal);

			printf("se envia pedido de map al nodoIP %s - path %s - nroDeBloqe %s \n",ipNodo,path,nroDeBloque);

			return msjParaEnviar;
		}
	}
}

char* crearPathTemporal(char *path){
	char *temporal = temporal_get_string_time();
	//char *path_with_temporal = malloc(strlen(path)+strlen("-")+strlen(temporal)+1);
	char *path_with_temporal = string_new();
	/*strcpy(path_with_temporal,path);
	strcpy(path_with_temporal+(strlen(path_with_temporal)),"-");
	strcpy(path_with_temporal+(strlen(path_with_temporal)),temporal);*/
	string_append(&path_with_temporal,path);
	string_append(&path_with_temporal,"-");
	string_append(&path_with_temporal,temporal);

	return path_with_temporal;
}

void actualizarPedidoRealizado(char *bloque, char *ipNodo, char* path, char *pathTemporal, StatusBlockState tipo )
{
	//HACER FREE DE TODOS LOS ANTERIORES ??

	if(bloque != NULL){
		pedidoRealizado_Bloque = bloque;
	}

	if(ipNodo != NULL){
		pedidoRealizado_Nodo = ipNodo;
	}

	if(path != NULL){
		pedidoRealizado_Path = path;
	}

	if(pathTemporal != NULL){
		pedidoRealizado_PathArchTemporal = pathTemporal;
	}

	if(tipo != NULL){

		(*pedidoRealizado_TipoPedido) = tipo;
	}
}

t_dictionary *obtenerCopiaDeConMenosCarga(Message *recvMessage,char *path,int bloqueNro){

	int nroDeCopias = obtenerNumeroDeCopiasParaArchivo(jobSocket,path);

	t_list *copias = obtenerCopiasParaBloqueDeArchivo(jobSocket,bloqueNro,path);
	t_dictionary *copiaConMenosCarga = list_get(copias,0);

	int j;
	for(j=1;j<(nroDeCopias);j++){

		//obtengo nodo con menos carga de operaciones

		t_dictionary *copia = list_get(copias,j);

		char *IPnroNodoCopia = dictionary_get(copia,K_Copia_IPNodo);
		char *IPnroNodoCopiaPivot = dictionary_get(copiaConMenosCarga,K_Copia_IPNodo);

		if((strcmp(IPnroNodoCopia,K_Copia_DarDeBajaIPNodo) == 0) &&
				(strcmp(IPnroNodoCopiaPivot,K_Copia_DarDeBajaIPNodo)==0) ){
			//ninguno de los nodos esta disponible
			//dejo la copia ya asignada

		}else if((strcmp(IPnroNodoCopia,K_Copia_DarDeBajaIPNodo)== 0)
					&& (strcmp(IPnroNodoCopiaPivot,K_Copia_DarDeBajaIPNodo)!=0)){

			//dejo la copia pivot

		}else if((strcmp(IPnroNodoCopia,K_Copia_DarDeBajaIPNodo)!=0) &&
					(strcmp(IPnroNodoCopiaPivot,K_Copia_DarDeBajaIPNodo)==0)){

			copiaConMenosCarga = copia;
		}else{

			int opsEnNodoCopia = getCantidadDeOperacionesEnProcesoEnNodo(IPnroNodoCopia);
			int opsEnNodoCopiaPivot = getCantidadDeOperacionesEnProcesoEnNodo(IPnroNodoCopiaPivot);

			if(opsEnNodoCopia < opsEnNodoCopiaPivot){
				copiaConMenosCarga = copia;
			}else{
				//la copiaPivot es la que tiene menos carga. La dejo.
			}
		}
	}

	return copiaConMenosCarga;
}

void actualizarTablas_RtaDeMapExitosa(Message *recvMessage)
{
	//*data:sizeRutaArchivoTemporal-rutaArchivoTemporal-Respuesta

	char *temporaryPath = deserializeFilePath(recvMessage,K_Job_MapResponse);
	char *IPnroNodo =  pedidoRealizado_Nodo;
	char *nroBloque =  pedidoRealizado_Bloque;

	t_dictionary *fileState = getFileStateForPath(pedidoRealizado_Path);
	t_list *blockStatesList = dictionary_get(fileState,K_FileState_arrayOfBlocksStates);
	t_dictionary *blockState = list_get(blockStatesList,cantidadDeBloquesProcesados);

	//******************************************
	//Actualizo blockState

	char *ptrNodo = malloc(strlen(pedidoRealizado_Nodo));
	ptrNodo=pedidoRealizado_Nodo;
	char *ptrBlq = malloc(strlen(pedidoRealizado_Bloque));
	ptrBlq=pedidoRealizado_Bloque;
	char *ptrPathTempo = malloc(strlen(pedidoRealizado_PathArchTemporal));
	ptrPathTempo=pedidoRealizado_PathArchTemporal;

	StatusBlockState *state = malloc(sizeof(StatusBlockState));
	*state=K_MAPPED;
	dictionary_destroy(blockState);

	t_dictionary *newBlckState = dictionary_create();
	dictionary_put(newBlckState,K_BlockState_nroBloque,ptrBlq);
	dictionary_put(newBlckState,K_BlockState_nroNodo,ptrNodo);
	dictionary_put(newBlckState,K_BlockState_temporaryPath,ptrPathTempo);
	dictionary_put(newBlckState,K_BlockState_state,state);

	list_remove(blockStatesList,cantidadDeBloquesProcesados);
	list_add_in_index(blockStatesList,cantidadDeBloquesProcesados,newBlckState);
	//******************************************
	//Actualizo nodoState
	decrementarOperacionesEnProcesoEnNodo(IPnroNodo);
	addTemporaryFilePathToNodoData(IPnroNodo,temporaryPath);
	//******************************************
	//Actualizo contador de bloques procesados
	cantidadDeBloquesProcesados++;
}

Message *createFSrequest(){

	char *path = pedidoRealizado_Path;
	Message *fsRequest = malloc(sizeof(Message));
	fsRequest->mensaje = malloc(sizeof(mensaje_t));

	//armo stream a mano
	char *stream = string_new();
	string_append(&stream,"DataFile ");
	string_append(&stream,path);

	fsRequest->sockfd = getFSSocket();
	fsRequest->mensaje->comandoSize = strlen(stream);
	fsRequest->mensaje->comando = stream;
	fsRequest->mensaje->dataSize = 0;
	fsRequest->mensaje->data = "";
	return fsRequest;
}
Message* armarMensajeParaEnvio(Message *recvMessage,void *stream,char *comando)
{
	Message *msjParaEnvio = malloc(sizeof(Message));
	msjParaEnvio->mensaje = malloc(sizeof(mensaje_t));
	msjParaEnvio->mensaje->comando = malloc(strlen(comando));
	msjParaEnvio->mensaje->data = malloc(strlen(stream)+1);

	msjParaEnvio->sockfd = recvMessage->sockfd;
	msjParaEnvio->mensaje->comandoSize = strlen(comando);
	msjParaEnvio->mensaje->comando = comando;
	msjParaEnvio->mensaje->dataSize = strlen(stream);
	msjParaEnvio->mensaje->data = stream;

	return msjParaEnvio;
}

// Idea de planificaion : se hara map y reduce de los bloques en orden ascendente.
// Hago map de todos los bloques de un archivo y ahi lanzo el reduce.
// Habra un MAXIMO de envios por la red, hasta que no responda un envio, nose hara otro(Productor-Consumidor)
// Si vuelve con error, entonces se rePlanifica y se vuelve a mandar el mismo pero a otro NODO.
// Si NO hay otro NODO disponible entonces, se informa que el proceso del archivo quedo incompleto--> no se que hacer luego.
// Si la operacion enviada falla porque se cayo el Nodo o simplemente fallo entonces se RE-PLANIFICA, si se cae el Job NO hay nada mas que hacer.
