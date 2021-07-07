#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<errno.h>
#include<string.h>
#include<signal.h>
#include<time.h>
#include "../include/llist.h"


int main(int argc, char ** argv) {

/* ##################_CREAZIONE SOCKET_########################## */
	int list_fd;
    if ((list_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        exit(0);
    }
    struct sockaddr_in serv_add;
    serv_add.sin_family = AF_INET;
    serv_add.sin_port = htons(1024); //porta server
    serv_add.sin_addr.s_addr = htonl(INADDR_ANY); //INADDR_ANY viene usato come indirizzo del server. L'applicazione accetterà connessioni da qualsiasi indirizzo associato al server

    int enable = 1;
    if (setsockopt(list_fd, SOL_SOCKET, SO_REUSEADDR, & enable, sizeof(int)) < 0) { //permette di riutilizzare lo stesso indirizzo
        perror("setsockopt");
        exit(1);
    }
    if (setsockopt(list_fd, SOL_SOCKET, SO_REUSEPORT, & enable, sizeof(int)) < 0) { //permette di riutilizzare lo stesso numero di porta
        perror("setsockopt");
        exit(1);
    }

    if (bind(list_fd, (struct sockaddr * ) & serv_add, sizeof(serv_add)) < 0) {
        perror("bind");
        exit(1);
    }

    if (listen(list_fd, 1024) < 0) {
        perror("listen");
        exit(1);
    }
/* ##################_FINE CREAZIONE_########################## */

//------------------------ DICHIARAZIONE VAR ----------------------//
	
	char nome_rist[max_name];
	int cnt;
	char mmmm;
	list* ristoranti = create_list(); // lista ristoranti
	list* richieste = create_list(); // lista richieste
	int fdr, fdc; //fd del ristorante e del client
	char id_client[id_size], id_rider[id_size], id_richiesta[id_size];  // id del ristorante e del client
	node* RS_node,*tmp; 
	Ristorante *rs, rrr; 
	Richiesta *req;
	Prodotto p;
	Ordine o, *oo;
	void *v; 
	int i,j; //invariante
	int code; // codice per gestire il protocollo
	int var;

//-------------------- FINE DICHIARAZIONE VAR ----------------------//

/* ##################_INIZIALIZZAZIONE CAMPI SELECT_########################## */
    fd_set rset;
    fd_set allset;
    int client[FD_SETSIZE];

    int maxd = list_fd;
    int maxi = -1;

    for (i = 1; i < FD_SETSIZE; i++)
        client[i] = -1;

    client[0] = list_fd;

    struct sockaddr_in cliaddr;
    socklen_t cliaddr_len;
    ssize_t n;

    int connfd;
    int sockfd;
    int ready;

    FD_ZERO( & allset);
    FD_SET(list_fd, & allset);
/* ##################_FINE INIZIALIZZAZIONE_########################## */

//*****************************_INIZIO_PROTOCOLLO_**************************************************//

    while (1) {
        
		// setta  l’insieme  dei  descrittori  da  controllare  in  lettura
        rset = allset;

        /* 
          chiama la select: 
            - esce quando un descrittore è pronto
            - restituisce il numero di descrittori pronti
        */

        if ((ready = select(maxd + 1, & rset, NULL, NULL, NULL)) < 0)
            perror("errore nella select");

		//* CONTROLLO SE IL DESCRITTORE PER LE CONNESSIONI E PRONTO IN LETTURA *//
		//* INDICA CHE QUALCUNO VUOLE CONNETTERSI AL SERVER                    *//
        // FARANNO RICHIESTA SIA I CLIENT CHE I RISTORANTI
        if (FD_ISSET(list_fd, & rset)) {
            cliaddr_len = sizeof(cliaddr);
            // invoca la accept
            if ((connfd = accept(list_fd, (struct sockaddr * ) & cliaddr, & cliaddr_len)) < 0)
                perror("errore nella accept");
            // inserisce il socket di connessione in un posto libero di client
            for (i = 0; i < FD_SETSIZE; i++)
                if (client[i] < 0) {
                    client[i] = connfd;
                    break;
          }
            // se non ci sono posti segnala errore
            if (i == FD_SETSIZE)
                perror("troppi client");
            FD_SET(connfd, & allset);
            // registra il socket ed aggiorna maxd
            if (connfd > maxd)
                maxd = connfd;
            if (i > maxi)
                maxi = i;
            if (--ready <= 0)
                continue;
        }

        // controlla tutti i socket di ascolto se sono leggibili
        for (i = 0; i <= maxi; i++) {

			// se il client i-esimo non è settato (=-1), passa avanti
            if ((sockfd = client[i]) < 0)
                continue;

//*****************************_INIZIO_PROTOCOLLO_******************************************************//
			//sleep(1);
            if (FD_ISSET(sockfd, & rset)) {
				
				/***********************************************************************/
				/*** AD OGNI CONNESSIONE INDIPENDENTE DAL CLIENT RICEVE UN MESSAGGIO ***/
				/*** QUESTO MESSAGGIO CONTIENE IL "COSA DEVE FARE" IL SEREVER **********/
            
				/**** CONTROLLA IL TIPO DI MESSAGGIO IN INGRESSO ***********************/
                read(sockfd, & code, sizeof(int));
				printf("\n--------------->CODICE: %d -> fd: %d \n",code, sockfd);
				
								// se la coda finisce chiudi
				/*printf("\nPress Any Key to Continue\n");scanf("%c",&mmmm);  */
				
				switch (code) {

						
					/**** NEL CASO IN CUI 0 ALLORA IL RISTORANTE SI CONNETTE *****/
					case 0: {
						//Riceve la struct ristorante e aggiunge il ristorante alla lista
						
						FullRead(sockfd, nome_rist, sizeof(char)*max_name);
						push_front(ristoranti, create_Ristorante(nome_rist,sockfd));
						printf("Si è connesso il Ristorante: %s (%d)\n", nome_rist,sockfd);
						break;
					}
					
					/**** NEL CASO IN CUI 1 ALLORA IL CLIENT SI CONNETTE *************/
					case 1: {
						/* invio al client le informazioni sui ristoranti */
						// mi calcolo quanti ristoranti ci sono
						cnt=size(ristoranti);
						//printf("Attualmente ci sono %d ristoranti collegati",cnt);
						/* invio al client il numero dei ristoranti */
						FullWrite(sockfd, & cnt, sizeof(int));
						if (cnt > 0) {
							RS_node=ristoranti->head;
							for(i=0;i<cnt && RS_node!=NULL;i++){
								rrr= *(Ristorante*)RS_node->data;
								FullWrite(sockfd,&rrr,sizeof(Ristorante));
								RS_node=RS_node->next;
							}
						}
						printf("Sono stati inviati i nomi dei ristoranti al client.\n");
						break;
					}
					
					/**** NEL CASO SI RICEVA IL MESSAGGIO 2 INDICA CHE IL CLIENT HA SCELTO IL RISTORANTE */
					case 2: {
						//il cliente, grazie alla struttura inviata, ha automaticamente calcolato l'fd del ristorante
						FullRead(sockfd, &fdr, sizeof(int));
						
						/* 
							//stampa ristorante scelto
							RS_node=find_resturant_by_fd(ristoranti,fdr);
							rs=(Ristorante*)RS_node->data;
							printf("Ristorante scelto dal client: %s", rs->nome_rist);
						*/
						
						/***********************************************************************/
						/*** QUANDO IL CLIENT SCEGLIE IL RISTORANTE SI CREA UNA RICHIESTA   ****/
						/*** LA RICHIESTA SERVE PER INDICARE E GESTIRE MEGLIO GLI ORDINI  ******/	
						/*** DA QUI IN POI QUANDO IL CLIENT INTERAGISCE CON IL SERVER SI *******/
						/*** PRENDERA LA LISTA PER VERIFICARE LO STATO DELLA RICHIESTA  ********/
						/*** DELL'ORDINE E GESTIRE OGNI CLIENT (DAL CASE 3 IN POI) *************/
						/********************** IN MODO EFFICIENTE *****************************/	
						/***********************************************************************/						
						
						//crea ed aggiunge una richiesta (fatta dal server al client)
						
						/* 	Il client una volta cliccato il ristorante DEVE ordinare qualcosa*/
						/* 	Se così non fosse gestire l'evento annulla-scelta (caso 4) 
							controllando prima il numero degli elementi dell'ordine:
							se l'ordine presenta un numero di elementi pari a 0, allora 
							cancella la richiesta del client che si sta creando				*/
						
						rand_string(id_richiesta,id_size);
						push_back(richieste, create_Richiesta(id_richiesta,sockfd, fdr,3)); 
						//gli passiamo il canale di comunicazione del client e del ristorante
						
						printf("\n Richiesta %s aggiunta alla lista richieste\n",id_richiesta);

				//-->	-1
						/* inviare -1 prima per avvisare il ristorante che deve inviare i prodotti */
						var = -1; 
						FullWrite(fdr, & var, sizeof(int));
						
						
						break;

					}
					
					/**** NEL CASO SI RICEVA IL MESSAGGIO 3 INDICA CHE IL RISTORANTE INVIA I PRODOTTI NEL MENU AI CLIENT*/
					case 3: {
						/* legge la quantità di prodotti inviati dal ristorante */
						FullRead(sockfd, & cnt, sizeof(int));
						
						/**
							trova la richiesta che ha queste caratteristiche:
								1- 	la socket del ristorante è pari a quella dell'attuale connessione
								2- 	si trova in stato 3, il che vuol dire che il server prende un 
									qualsiasi client (indicato dal campo "client") che fa richiesta 
									di menu e gli invia i prodotti */
						req = find_client_request( richieste, sockfd, 3);
						/* si trova l'fd del client che ha richiesto il menu da questo ristorante */
						fdc=req->fd_client;
						
						//invia il numero di prodotti 
						FullWrite(fdc, & cnt, sizeof(int));
						for (j = 0; j < cnt; j++) {
							/* recezione ed invio dei prodotti dal ristorante al client*/
							FullRead(sockfd,&p,sizeof(Prodotto));
							FullWrite(fdc,&p,sizeof(Prodotto));
							print_struct_Prodotto(&p);
						}
						
						printf("Invio dei (%d) prodotti del menu al client [connesso su fd: %d]\n",j,fdc);
						
						// FA "EVOLVERE" la richiesta -> adesso, il ristorante che farà richiesta 
						req -> stato_richiesta = 4;
						
						break;
					}
					
					
					/**** NEL CASO SI RICEVA IL MESSAGGIO 4 INDICA CHE IL SERVER DEVE RICEVERE L'ORDINE DEL CLIENT ED INVIARLO AL RISTORANTE*/
					case 4: { //riceve l'ordine dal client e lo invia al ristorante

						/* leggo l'ordine del client*/
						FullRead(sockfd, &cnt, sizeof(int));
						/** 
							trova la richiesta che ha queste caratteristiche 
							1- 	la socket del client è pari a quella dell'attuale in connessione.
							2- 	con stato 4, il che vuol dire che il serve prende il ristorante 
								(indicato dal campo "ristorante") e gli inoltre l'ordine del client.*/
						req = find_resturant_request( richieste, sockfd, 4);
						/* si trova l'fd del ristorante a cui il client ha fatto richiesta menu e quindi vuole inviargli l'ordine */
						fdr = req->fd_ristorante;
						
				//->	-2
						/* inviare -2 prima per avvisare il ristorante che sta arrivando l'ordine */
						var = -2;
						FullWrite(fdr, & var, sizeof(int));
						
						/* invio size dell'ordine al ristorante ricevuto all'inizio*/
						FullWrite(fdr, &cnt, sizeof(int));
						
						/*invio id_richiesta al ristorante*/
						stpcpy(id_richiesta,req->id_richiesta);
						FullWrite(fdr, id_richiesta, sizeof(char)*id_size);

						oo=malloc(sizeof(Ordine));
						for (j = 0; j < cnt; j++) {
							/* recezione ed invio ordine dal client al ristorante*/
							FullRead(sockfd,oo,sizeof(Ordine));
							FullWrite(fdr,oo,sizeof(Ordine));
							print_struct_Ordine(oo);
							//oo=(Ordine*)NULL;
						}
						printf("\nIl client [connesso su fd: %d] ha effettuato l'ordinazione ed è stata inoltrata al ristorante [connesso su fd: %d].\n",sockfd,fdr);
						
						// FA "EVOLVERE" la richiesta -> adesso, il ristorante che farà richiesta 
						req -> stato_richiesta = 5;
						
						break;
					}
					
					
					/**** NEL CASO SI RICEVA IL MESSAGGIO 5 INDICA CHE IL RISTORANTE STA ELABORATO UN ORDINE */
					case 5: {

						/* il ristorante invia l'id del rider che ha preso in carico l'ordine */
						FullRead(sockfd, id_rider, sizeof(char)*id_size);

						/* ed inoltre invia pure l'id relativo alla richiesta effettuata dal client */
						FullRead(sockfd, id_richiesta, sizeof(char)*id_size);

						/* trova la richiesta a cui è associata quell'id */
						req = find_id_request( richieste, id_richiesta);
						/* si estrae il fd del client */
						fdc=req->fd_client;

						/* invia l'id del rider al client*/
						FullWrite(fdc, id_rider, sizeof(char)*id_size);

						/* riceve l'id_client dal client*/
						FullRead(fdc, id_client, sizeof(char)*id_size);

						/* invia l'id_client appena ricevuto al ristorante*/
						FullWrite(sockfd, id_client, sizeof(char)*id_size);
						printf("Consegna assegnata al rider: %s per il cliente %s.\n", id_rider, id_client);
						
						// FA "EVOLVERE" la richiesta -> adesso, il ristorante che farà richiesta 
						req -> stato_richiesta = 6;
						
						break;
					}
					
					/**** NEL CASO SI RICEVA IL MESSAGGIO 8 INDICA CHE IL RISTORANTE HA CONSEGNATO L'ORDINE */
					case 8:{ // consegna ordine
						FullRead(sockfd, id_richiesta, sizeof(char)*id_size);
						printf("Richiesta [%s] ",id_richiesta);
						
						/** trova IL NODO della richiesta che ha queste id pari a quello passato dal ristorante*/
						tmp = find_id_request_node( richieste, id_richiesta);
						delete_node(richieste, tmp); //elimina richiesta memorizzata
						
						printf(" effettuata dal rider: %s.\n",id_rider);
						
						break;
					}
					
					case 9: {//disconnessione client
						if (close(sockfd) == -1)
							perror("Errore nella close");
						FD_CLR(sockfd, & allset);
						client[i] = -1;
						break;
					}
					
					case 10: {// disconnessione ristorante
						if (close(sockfd) == -1)
							perror("Errore nella close");
						FD_CLR(sockfd, & allset);
						client[i] = -1;
						tmp = find_resturant_by_fd(ristoranti, sockfd);
						delete_node( ristoranti, tmp);
						break;
					}

					default:
						break;
				}

                // se la coda finisce chiudi
                code = -1; 
				
                if (--ready <= 0) // se ho letto tutti i fd
                    break;
            }
        }
    }

//*****************************_FINE_PROTOCOLLO_******************************************************//


} //fine main






