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
    serv_add.sin_addr.s_addr = htonl(INADDR_ANY); 

    int enable = 1;
    if (setsockopt(list_fd, SOL_SOCKET, SO_REUSEADDR, & enable, sizeof(int)) < 0) {
        perror("setsockopt");
        exit(1);
    }
    if (setsockopt(list_fd, SOL_SOCKET, SO_REUSEPORT, & enable, sizeof(int)) < 0) {
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
/* ############################################################################# */


//------------------------ DICHIARAZIONE VAR ----------------------//
 
	int cnt,i,j,code,var;
	int fdr, fdc; //fd del ristorante e del client
	
	char id_client[id_size], id_rider[id_size], id_operazione[id_size], nome_rist[max_name];
	
	node* RS_node,*tmp; 
	Ristorante rist; 
	Operazione *ope;
	Prodotto p;
	Ordine *ord;
	
	list* ristoranti = create_list(); // lista ristoranti
	list* operazioni = create_list(); // lista operazioni


//----------------------------------------------------------------//


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
    FD_SET(list_fd, &allset);
	
/* ######################################################################### */

//*****************************_INIZIO_PROTOCOLLO_**************************************************//

    while (1) {
        
		// setta  l’insieme  dei  descrittori  da  controllare  in  lettura
        rset = allset;
		
		///* SELECT  *////
        if ((ready = select(maxd + 1, & rset, NULL, NULL, NULL)) < 0)
            perror("errore nella select");

		/* si controlla se il descrittore per le connessioni è pronto in lettura */
        // I tipo di client che si connetterennao i CLIENTI ed i RISTORANTI
        if (FD_ISSET(list_fd, & rset)) {
            cliaddr_len = sizeof(cliaddr);
            // invoca la accept
            if ((connfd = accept(list_fd, (struct sockaddr * ) &cliaddr, &cliaddr_len)) < 0)
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
				
            // registra il socket ed aggiorna (opportunamente) maxd e maxi
            FD_SET(connfd, & allset);
            if (connfd > maxd)
                maxd = connfd;
            if (i > maxi)
                maxi = i;
	
            // decrementa il numero di "fd" da leggere (in quanto uno è già stato soddisfatto)
            if (--ready <= 0)
                continue;
        }

        // controlla tutti i socket di ascolto se sono leggibili
        for (i = 0; i <= maxi; i++) {

			// se il client i-esimo non è settato (=-1), passa avanti
            if ((sockfd = client[i]) < 0)
                continue;

//*****************************_INIZIO_PROTOCOLLO_******************************************************//
				
			/***********************************************************************/
			/*** AD OGNI CONNESSIONE INDIPENDENTE DAL CLIENT RICEVE UN MESSAGGIO ***/
			/*** QUESTO MESSAGGIO CONTIENE IL "COSA DEVE FARE" IL SEREVER **********/
			/***********************************************************************/
			
            if (FD_ISSET(sockfd, & rset)) {
				          
				/** CONTROLLA IL MESSAGGIO IN INGRESSO **/
                read(sockfd, &code, sizeof(int));
				
				//printf("\n--------------->CODICE: %d -> fd: %d \n",code, sockfd);

				switch (code) {

						
					/** NEL CASO SI RICEVA 0: ALLORA IL RISTORANTE SI CONNETTE **/
					case 1: {
						/* si riceve il nome del ristorante e aggiunge il ristorante alla lista */
						FullRead(sockfd, nome_rist, sizeof(char)*max_name);
						push_front(ristoranti, create_Ristorante(nome_rist,sockfd));
						printf("Si è connesso il Ristorante: %s (%d)\n", nome_rist,sockfd);
						break;
					}
					
					/** NEL CASO SI RICEVA 1: IL CLIENT SI CONNETTE E VUOLE SAPERE I RISTORANTI ATTIVI **/
					case 2: {
						
						/* calcolo ed invio al client il numero dei ristoranti */
						cnt=size(ristoranti);			
						FullWrite(sockfd, & cnt, sizeof(int));
						/* si invia, se ci sono, i ristoranti attivi, uno ad uno, scorrendo la lista */
						if (cnt > 0) {
							RS_node=ristoranti->head;
							for(i=0;i<cnt && RS_node!=NULL;i++){
								rist= *(Ristorante*)RS_node->data;
								FullWrite(sockfd,&rist,sizeof(Ristorante));
								RS_node=RS_node->next;
							}
						}
						printf("Sono stati inviati i nomi dei ristoranti al client.\n");
						break;
						
					}
					
					/** NEL CASO SI RICEVA 2:  INDICA CHE IL CLIENT HA SCELTO IL RISTORANTE **/
					case 3: {
						
						/***********************************************************************/
						/*** QUANDO IL CLIENT SCEGLIE IL RISTORANTE SI CREA UNA OPERAZIONE  ****/
						/*** L'OPERAZIONE SERVE PER INDICARE E GESTIRE MEGLIO GLI ORDINI E *****/	
						/*** MANTENERE LA TRACCIABILITA' DELLE OPERAZIONE TRA MITTENTE E *******/
						/*** DESTINATARIO, QUINDI, IL COLLEGAMENTO TRA CLIENTE E RISTORANTE. ***/
						/*** DA QUI IN POI QUANDO I CLIENT INTERAGISCONO CON IL SERVER SI ******/
						/*** PRENDERA' LA LISTA PER VERIFICARE LO STATO DELL'OPERAZIONE ********/
						/*** DELL'ORDINE E GESTIRE OGNI CLIENT IN MODO EFFICIENTE **************/	
						/***********************************************************************/
						
						//il cliente, grazie alla struttura inviata, ha automaticamente inviato l'fd del ristorante
						
						/* il client invia il ristorante scelto */
						FullRead(sockfd, &fdr, sizeof(int));
						
						//crea ed aggiunge una Operazione, si utilizzano i fd del cliente e del ristorante
						rand_string(id_operazione,id_size);
						push_back(operazioni, create_Operazione(id_operazione,sockfd, fdr,1)); // STATO = 1
						//printf("\n Operazione %s aggiunta alla lista operazioni\n",id_operazione);

				//-->	1
						/* inviare 1 prima per avvisare il ristorante che deve inviare i prodotti del Menu*/
						var = 1; 
						FullWrite(fdr, & var, sizeof(int));
						
						break;

					}
					
					/** NEL CASO SI RICEVA 3: INDICA CHE IL RISTORANTE INVIA IL MENU AL CLIENT CHE HA FATTO RICHIESTA **/
					case 4: {
						
						/* legge la quantità di prodotti nel menu inviati dal ristorante al client*/
						FullRead(sockfd, & cnt, sizeof(int));
						
						/** trova l'Operazione che ha queste caratteristiche:
								1- 	il fd del ristorante è pari a quella dell'attuale connessione
								2- 	si trova in stato 1, il che vuol dire che il server prende un
									qualsiasi client (indicato dal campo "client") che fa richiesta 
									di menu per inviargli i prodotti */
						ope = find_client_operation( operazioni, sockfd, 1);
						fdc=ope->fd_client; // si trova l'fd del client che ha richiesto il menu da questo ristorante
						
						/* invia il numero di prodotti del menu al client */
						FullWrite(fdc, & cnt, sizeof(int));
						for (j = 0; j < cnt; j++) {
							/* recezione ed inoltro dei prodotti dal ristorante al client direttamente*/
							FullRead(sockfd,&p,sizeof(Prodotto));
							FullWrite(fdc,&p,sizeof(Prodotto));
							//print_struct_Prodotto(&p);
						}
						
						//printf("Invio dei (%d) prodotti del menu al client [connesso su fd: %d]\n",j,fdc);
						
						// fa "evolvere" lo stato dell'operazione da effettuare
						ope -> stato_operazione = 2;
						
						break;
					}
					
					
					/** NEL CASO SI RICEVA 4: INDICA CHE IL CLIENT DEVE INVIARE L'ORDINE ED INOLTRARLO AL RISTORANTE*/
					case 5: {

						/* leggo la quantità di elementi nell'ordine del client*/
						FullRead(sockfd, &cnt, sizeof(int));
						
						/** trova l'Operazione che ha queste caratteristiche:
							1- 	la socket del client è pari a quella dell'attuale in connessione.
							2- 	si trova nello stato 2, il che vuol dire che il serve prende il ristorante
								(indicato dal campo apposito) e gli inoltre l'ordine del client.*/
						ope = find_resturant_operation( operazioni, sockfd, 2);
						fdr = ope->fd_ristorante; // si trova l'fd del ristorante richiesto dal client*/

						
				//->	2
						/* inviare 2 prima per avvisare il ristorante che sta arrivando l'ordine */
						var = 2;
						FullWrite(fdr, & var, sizeof(int));
						
						/* invio id_operazione al ristorante */
						stpcpy(id_operazione,ope->id_operazione);
						FullWrite(fdr, id_operazione, sizeof(char)*id_size);
						/** Questo servirà successivamente per tener traccia dello stato di avanzamento della richiesta del client e del ristorante fino alla consegna dell'ordine */

						/* invio size dell'ordine al ristorante ricevuto all'inizio*/
						FullWrite(fdr, &cnt, sizeof(int));

						ord=malloc(sizeof(Ordine));
						for (j = 0; j < cnt; j++) {
							/* recezione ed inoltro ordine dal client al ristorante*/
							FullRead(sockfd,ord,sizeof(Ordine));
							FullWrite(fdr,ord,sizeof(Ordine));
							print_struct_Ordine(ord);
							//ord=(Ordine*)NULL;
						}
						
						printf("\nIl client [connesso su fd: %d] ha effettuato l'ordinazione ed è stata inoltrata al ristorante [connesso su fd: %d].\n",sockfd,fdr);
						
						// fa "evolvere" lo stato dell'operazione da effettuare 
						ope -> stato_operazione = 3;
						
						break;
					}
					
					
					/** NEL CASO SI RICEVA 5: INDICA CHE IL RISTORANTE HA ELABORATO L'ORDINE **/
					case 6: {

						/* il ristorante invia l'id del rider che ha preso in carico l'ordine */
						FullRead(sockfd, id_rider, sizeof(char)*id_size);

						/* ed inoltre invia pure l'id relativo all'operazione da aggiornare*/
						FullRead(sockfd, id_operazione, sizeof(char)*id_size);
						/* 	Si è scelto l'invio e la successiva recezione dell'id-operazione in quanto
							bisognava tener conto di una specifica richiesta, avente uno specifico client. 
							Per cui, se si dovesse utilizzare la funzione find_client_operation 
							si troverà un generico client, che ha ordinato dallo stesso ristorante, ma non
							è detto che sia il client a cui effettivamente il rider sta inviando l'ordine	*/

						/* trova l'operazione a cui è associata quell'id */
						ope = find_id_operation( operazioni, id_operazione);
						fdc=ope->fd_client;// si ricava l'fd del client

						/* invia l'id del rider (ricevuto) al client*/
						FullWrite(fdc, id_rider, sizeof(char)*id_size);

						/* riceve l'id_client dal client*/
						FullRead(fdc, id_client, sizeof(char)*id_size);

						/* invia l'id_client (ricevuto) al ristorante*/
						FullWrite(sockfd, id_client, sizeof(char)*id_size);
						
						printf("Consegna assegnata al rider: %s per il cliente %s.\n", id_rider, id_client);
						
						// fa "evolvere" lo stato dell'operazione
						ope -> stato_operazione = 4;
						
						break;
					}
					
					/** NEL CASO SI RICEVA: 8 INDICA CHE IL RISTORANTE (attraverso il rider) HA CONSEGNATO L'ORDINE **/
					case 7:{
						
						/* riceve l'id del'operazione contenente l'fd del client che ha ricevuto l'ordine */
						FullRead(sockfd, id_operazione, sizeof(char)*id_size);
				
						/* trova IL NODO dell'Operazione che ha id pari a quello passato dal ristorante 
							e la elimina, in quanto non c'è più necessità di memorizzarla, in quanto
							sono state svolte e soddisfatte tutte le operazioni relative a:
							- richiesta menu, 
							- passaggio ordine
							- elaborazione ordine
							- consegna ordine 	*/
						tmp = find_id_operation_node( operazioni, id_operazione);
						delete_node(operazioni, tmp);
						
						printf("Consegna [%s] effettuata dal rider: %s.\n",id_operazione,id_rider);
						
						break;
					}
					
					case 8: {
						//disconnessione client
						if (close(sockfd) == -1)
							perror("Errore nella close");
						FD_CLR(sockfd, & allset);
						client[i] = -1;
						break;
					}
					
					case 9: {
						// disconnessione ristorante
						if (close(sockfd) == -1)
							perror("Errore nella close");
						FD_CLR(sockfd, & allset);
						client[i] = -1;
						tmp = find_resturant_by_fd(ristoranti, sockfd);
						delete_node( ristoranti, tmp);
						break;
					}
 
					default: // se ricevo un segnale non registrato
							// non faccio niente
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






