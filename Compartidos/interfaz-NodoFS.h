/*
 * interfaz-NodoFS.h
 *
 *  Created on: 12/5/2015
 *      Author: utnso
 */

#ifndef COMPARTIDOS_INTERFAZ_NODOFS_H_
#define COMPARTIDOS_INTERFAZ_NODOFS_H_

//El FileSystem u otro proceso Nodo podrán solicitar las siguientes operaciones:

//- getBloque(numero) ​
//:   Devolverá   el   contenido   del   bloque   solicitado   almacenado   en   el
//Espacio de Datos.

//- setBloque(numero,   [datos]) ​
//:   Grabará   los   datos   enviados   en   el   bloque   solicitado   del
//Espacio de Datos

// getFileContent(nombre) ​
//:   Devolverá   el   contenido   del   archivo   de   Espacio   Temporal
//solicitado.

#endif /* COMPARTIDOS_INTERFAZ_NODOFS_H_ */
