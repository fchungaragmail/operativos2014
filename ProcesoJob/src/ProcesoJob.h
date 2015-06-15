/*
 * ProcesoJob.h
 *
 *  Created on: 17/5/2015
 *      Author: utnso
 */

#ifndef SRC_PROCESOJOB_H_
#define SRC_PROCESOJOB_H_

#include "Definiciones.h"


void IniciarConexionMarta();
void HacerPedidoMarta();
void IniciarConfiguracion();
void Terminar(int);
char* LeerArchivo(char*);
void ReportarResultadoHilo(HiloJob*, EstadoHilo);

/*
 *
 * PLANIFICACION DE HILOS
 *
 */
void PlanificarHilosMapper(mensaje_t*);
void PlanificarHilosReduce(mensaje_t*);
/*
 * HANDLERS
 */
void* pedidosMartaHandler(void*);

#endif /* SRC_PROCESOJOB_H_ */
