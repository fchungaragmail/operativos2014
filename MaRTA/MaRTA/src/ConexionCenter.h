/*
 * ConexionCenter.h
 *
 *  Created on: 22/5/2015
 *      Author: utnso
 */

#ifndef CONEXIONCENTER_H_
#define CONEXIONCENTER_H_

t_list* listaConexiones;

void initConexiones();
void *listenFS();
void *listenJobs();
void closeServidores();

#endif /* CONEXIONCENTER_H_ */
