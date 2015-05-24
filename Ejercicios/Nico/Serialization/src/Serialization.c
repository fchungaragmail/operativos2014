/*
 ============================================================================
 Name        : Serialization.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>

#include "SerializationFW.h"
#include "PackedSerialization.h"

#define MaxDataSize 90

struct SerializedPacket {
    int senderId;
    int sequenceNumber;
    char data[MaxDataSize];
    char *string;
} __attribute__((packed));

struct No_SerializedPacket {
    int senderId;
    int sequenceNumber;
    char data[MaxDataSize];
    char *string;
};

int main(void) {

/*	char *request = malloc(strlen("saraza")+1);
	strcpy(request,"saraza");
	void* data = malloc(sizeof(int)+sizeof(int)+strlen(request)+1);
	int tipo =22;
	int largo = 44;
	int size = 0, offset = 0;
	offset += size;

	//serializo

	memcpy(data + offset,&tipo, size = sizeof(int));
	offset += size;
	memcpy(data + offset,&largo,size = sizeof(int));
	offset += size;
	memcpy(data + offset,request,size = strlen(request) + 1);
	printf("Quedó %s\n:",(char *)data);

	//deserializo
	int tipo2;
	int largo2;
	char *req2=malloc (strlen("saraza")+1);

	size=0;offset=0;
	memcpy(&tipo2,data + offset, size = sizeof(int));
	printf("Quedó %d\n:",tipo2);
	offset += size;

	memcpy(&largo2,data + offset, size = sizeof(int));
	printf("Quedó %d\n:",largo2);
	offset += size;

	memcpy(req2,data + offset,size = strlen(req2));
	printf("Quedó %s\n:",req2);
	offset += size;
*/
	/*offset += size;
	memcpy(data + offset,&largo,size = sizeof(int));
	offset += size;
	memcpy(data + offset,request,size = strlen(request) + 1);
	*/
//**********************************************************************

	//struct no serializado

	struct No_SerializedPacket *n_s;
	n_s=malloc(sizeof(struct No_SerializedPacket));
	n_s->string = malloc(sizeof(char)*30);

	n_s->senderId = 22;
	n_s->sequenceNumber = 55;
	strcpy(n_s->data,"saraza");
	strcpy(n_s->string,"stringggg");


	//serializacion
	struct SerializedPacket *s;
	s = malloc(sizeof(struct SerializedPacket));
	s->string = malloc(strlen(n_s->string)+1);

	s->senderId = htonl(n_s->senderId);
	s->sequenceNumber = htonl(n_s->sequenceNumber);
	memcpy(s->data, n_s->data, MaxDataSize);
	memcpy(s->string, n_s->string, strlen(n_s->string)+1);


	//deserializacion

	struct No_SerializedPacket *n_s2;
	n_s2=malloc(sizeof(struct No_SerializedPacket));
	n_s2->string = malloc(strlen(n_s->string)+1);

	n_s2->senderId = ntohl(s->senderId);
	n_s2->sequenceNumber = ntohl(s->sequenceNumber);
	memcpy(n_s2->data, s->data, MaxDataSize);
	memcpy(n_s2->string, s->string, strlen(n_s->string)+1);

	printf("senderId deserializado %d ",n_s2->senderId);
	printf("sequenceNumber deserializado %d ",n_s2->sequenceNumber);
	printf("string deserializado %s ",n_s2->string);
	printf("data deserializado %s ",n_s2->data);


	/* prints !!!Hello World!!! */
	/*
	Buffer *buf;
	buf = new_buffer();
	int a =22;
	int b =33;

	serialize_int(a,buf);
	serialize_int(b,buf);



	puts("!!!Hello World!!!");
	a=a+5;
	//printf("la data serializada es %d",(void*)buf->data);

	 */
	return EXIT_SUCCESS;
}
