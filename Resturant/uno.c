#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<errno.h>
#include<string.h>
#include <signal.h>
#include "../include/llist.h"


int serv;

void before_close(int sig) {
    int var = 10;
    FullWrite(serv, & var, sizeof(int));
    exit(0);
}

int main(int argc, char ** argv) {

/* ##################_CREAZIONE SOCKET_########################## */
	int listen_fd;
    if ((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        exit(0);
    }

    struct sockaddr_in serv_add;
    serv_add.sin_family = AF_INET;
    serv_add.sin_port = htons(1025); // port 1025
    serv_add.sin_addr.s_addr = htonl(INADDR_ANY);

    int enable = 1;
    if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, & enable, sizeof(int)) < 0) {
        perror("setsockopt");
        exit(1);
    }
    if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEPORT, & enable, sizeof(int)) < 0) {
        perror("setsockopt");
        exit(1);
    }

    if (bind(listen_fd, (struct sockaddr * ) & serv_add, sizeof(serv_add)) < 0) {
        perror("bind");
        exit(1);
    }

    if (listen(listen_fd, 1024) < 0) {
        perror("listen");
        exit(1);
    }

/* ##################################################################### */


/* ########################_ COLLEGAMENTO SERVER_ ############################## */
	int servfd;
    if ((servfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        fprintf(stderr, "socket error, Assegnazione automatica\n");
        //exit(1);
		argv[1]="127.0.0.1";
    }
	struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(1024);
    if (inet_pton(AF_INET, "127.0.0.1", & servaddr.sin_addr) < 0) {
        fprintf(stderr, "inet_pton error for %s\n", argv[1]);
        exit(1);
    }
    if (connect(servfd, (struct sockaddr * ) & servaddr, sizeof(servaddr)) < 0) {
        fprintf(stderr, "connect error\n");
        exit(1);
    }
/* ############################################################################# */


//------------------------ DICHIARAZIONE VAR ----------------------//
	
	int i,j, n, code, numProdotti, var, loop, scelta, cnt, x;
    int fd_client, fd_rider; // fd dei client e dei rider
	char id_rider[id_size], id_client[id_size], id_operazione[id_size];
	
	node *tmp;

    Ordine ord;
	Info_ordine *lo;
	Prodotto p;
	list* order_info = create_list(); // per la recezione dell'ordine
	
	list* ordini = create_list(); //alloco lista per gli ordini
   	list* riders = create_list(); //alloco lista per i riders

	char nome_rist[max_name];
	strcpy(nome_rist,"Palapizza");
 

   	list *menu=create_list(); //alloco lista per il menu
	/* creazione menu */
	int n_items=5;
	char *items[n_items];
	float prezzo[n_items];
	items[0]="cocacol";	
	items[1]="purpett";
	items[2]="patan";
	items[3]="zampon";
	items[4]="zeppl";
	prezzo[0]=1.5;
	prezzo[1]=3.5;
	prezzo[2]=4.0;
	prezzo[3]=1.0;
	prezzo[4]=1.5;

	for (int i=0;i<n_items;i++)
		push_front(menu,create_Prodotto(items[i],prezzo[i]));
	
	traverse(menu,print_struct_Prodotto);

	int no=0; //identifica il numero degli ordnini disponibili per la consegna del rider 
	
//---------------------------------------------------------------------------------------//


/* ##################_INIZIALIZZAZIONE CAMPI SELECT_########################## */

    serv=servfd;
	
	fd_set rset;
    fd_set allset;
    int client[FD_SETSIZE];

    int maxd = listen_fd;
    int maxi = 1;

    if (servfd > listen_fd)
        maxd = servfd;

    for (i = 1; i < FD_SETSIZE; i++)
        client[i] = -1;

    client[0] = listen_fd;
    client[1] = servfd;

    struct sockaddr_in cliaddr;
    socklen_t cliaddr_len;

    int sockfd;
    int connd;
    int ready;

    FD_ZERO( & allset);
    FD_SET(listen_fd, & allset); 
    FD_SET(servfd, & allset); // setto tra i descrittori in lettura da controllare anche il fd su cui si è collegati al server
	
/* ########################################################################### */

//*****************************_INIZIO_PROTOCOLLO_******************************************************//
	

//->	1	
	/* una volta che si connette gli invia un identificativo -> 1 "sono ristorante e ti sto per inviare i miei dati" */
	var = 1;
    FullWrite(servfd, & var, sizeof(int));

	/* invio nome del ristorante al server */
    FullWrite(servfd, &nome_rist, sizeof(char)*max_name);

    signal(SIGINT, before_close); //gestione segnale SIGINT ->invia prima il messaggio "10" al server (" sono il ristorante me ne sto andando libera il decrittore che mi hai allocato")
	
    while (1) {
		
        rset = allset;
        
		///* SELECT  *////
        if ((ready = select(maxd + 1, & rset, NULL, NULL, NULL)) < 0)
            perror("errore nella select");

		/* si controlla se il descrittore per le connessioni è pronto in lettura */
        // gli unici a fare queste richieste sono ovviamente i rider 
        if (FD_ISSET(listen_fd, & rset)) {
            cliaddr_len = sizeof(cliaddr);
            // invoca la accept
            if ((connd = accept(listen_fd, (struct sockaddr * ) & cliaddr, & cliaddr_len)) < 0)
                perror("errore nella accept");
            // inserisce il socket di connessione in un posto libero di client
            for (i = 0; i < FD_SETSIZE; i++)
                if (client[i] < 0) {
                    client[i] = connd;
                    break;}
					
            // se non ci sono posti segnala errore
            if (i == FD_SETSIZE)
                perror("troppi client");
			
			// registra il socket ed aggiorna (opportunamente) maxd e maxi
            FD_SET(connd, & allset);
            if (connd > maxd)
                maxd = connd;
            if (i > maxi)
                maxi = i;
			
			// decrementa il numero di "fd" da leggere (in quanto uno è già stato soddisfatto)	
            if (--ready <= 0)
                continue;
        }

        // controlla tutti i socket di ascolto se sono leggibili
        for (i = 1; i <= maxi; i++) {
			
			// se il client i-esimo non è settato (=-1), passa avanti
            if ((sockfd = client[i]) < 0)
                continue;

//*****************************_INIZIO_PROTOCOLLO_******************************************************//
			
			//sleep(1);
            if (FD_ISSET(sockfd, & rset)) {

				// sockfd -> fd rider; servfd -> fd server, nei casi "-1" e "-2" sono scambiabili 
				
				/***********************************************************************/
				/*** AD OGNI CONNESSIONE INDIPENDENTE DAL CLIENT RICEVE UN MESSAGGIO ***/
				/*** QUESTO MESSAGGIO CONTIENE IL "COSA DEVE FARE" IL SEREVER **********/
				/***********************************************************************/
				
				/** CONTROLLA IL MESSAGGIO IN INGRESSO **/
                FullRead(sockfd, & code, sizeof(int));
				
				//printf("\n--------------->CODICE: %d -> fd: %d \n",code, sockfd);
				
				switch (code) {
					
					/** NEL CASO SI RICEVA -1:  INDICA CHE IL SERVER VUOLE I PRODOTTI NEL MENU **/
					case 1: { 
						//-> in questo caso sockfd == servfd
						
				//->	4
						/* invio 4 per dire al server di posizionarsi nel caso in cui devi ricevere i prodotto */
						var = 4;
						FullWrite(servfd, & var, sizeof(int));
						
						/* calclolo ed invio della quantità di prodotti nel menu*/
						cnt=n_items;
						FullWrite(servfd, &cnt, sizeof(int));
						tmp=(node*)menu->head;
						p = *(Prodotto*)tmp->data;
						print_struct_Prodotto(&p);
						for(i=0;i<cnt;i++){
							p = *(Prodotto*)tmp->data;
							/* invio prodotti del menu */
							FullWrite(servfd,&p,sizeof(p));
							tmp=tmp->next;
						}
						printf("\nHo inviato i (%d) prodotti richiesti dal server\n",i);
											
						break;
					}


					/** NEL CASO SI RICEVA -2:  INDICA CHE IL RISTORANTE DEVE RICEVERE L'ORDINE DI UN CLIENTE DAL SERVER **/
					case 2: { 
						
						/* recezione dell'id dell'operazione ordine a cui sarà associato l'ordine*/
						FullRead(servfd,id_operazione,sizeof(char)*id_size);
						
						/* ricezione della qt di oggetti nell'ordine */
						FullRead(servfd,&cnt,sizeof(int));
						
						for(int i=0;i<cnt;i++){
							/* lettura degli oggetti dell'ordine */
							FullRead(servfd,&ord,sizeof(ord));
							push_back(ordini,&ord);
						}
						
						printf("\n Ordine %s di %d elementi ricevuto dal server\n",id_operazione, cnt);
						
						/* aggiunge ordine ricevuto in coda alla lista degli ordini*/
						push_back(order_info, create_Info_ordine(ordini,id_operazione,1,-1));
						
						no++; // incrementa il numero degli ordini disponibili al rider;
						
						// elimino gli elementi della lista per futuri ordini
						 	//empty_list(ordini,free_Ordine);
						free(ordini);
						
						break;
					}

					/** NEL CASO SI RICEVA 0: INDICA CHE IL RIDER VUOLE SAPERE QUANTI ORDINI CI SONO DA CONSEGNARE **/
					case 3: {
					
						if (no <= 0){ // non ci sono ordini
							printf("\n Non ci sono attualmente ordini disponibili per i rider\n");
							cnt=0;
						}
						else
							printf("\n Ci sono %d ordini disponibili \n",no);
							
						/* invio il numero degli ordini disponibili al rider */
						FullWrite(sockfd, &no, sizeof(int));
						
						break;
					}
					
					/** NEL CASO SI RICEVA 1: VUOL DIRE CHE IL RIDER E' DISPOSTO A CONSEGNARE UN ORDINE **/
					case 4: {
					
						tmp = order_info->head;
						loop=1;
						x=1; // variabile assegnazione ordine;
						
						/** Ricerca ordine disponibile */
						while(tmp!=NULL && loop==1 && no>0){
							lo=(Info_ordine*)tmp->data;
							x = lo->stato_ordine;
							printf("ordine %s = stato %d",lo->id_operazione,x);
							if(x==1){ // ordine non assegnato a nessun rider -> di conseguenza lo assegno al rider.
								
								// setto lo stato ad 1 = "ordine assegnato ad un rider"
								lo->stato_ordine=2; 
								
								// decremento il numero di ordini disponibili ai rider
								no--; 
								
								// setto l'fd del rider attuale come attributo nella lista ordini
								lo->fd_rider=sockfd;
								
								//copio l'id della Operazione per inviarla al server
								stpcpy(id_operazione,lo->id_operazione);
								
								loop=0; // esco dal while- interrompo il ciclo
								
								break;
							}
							tmp=tmp->next;
						}
						
						if(loop==0){ // se un rider si è preso un ordine ed ha interrotto di conseguenza il ciclo
							
						//->	1
							/* invio 1 al rider che indica che può "accaparrarsi" l'ordine per la consegna */
							var = 1;
							FullWrite(sockfd, &var, sizeof(int));
							
							/* allora il rider mi invia il suo id */
							FullRead(sockfd, id_rider, sizeof(char)*id_size);
							
							/* invio l'id_operazione dell'ordine che ha preso in carico*/
							FullWrite(sockfd,id_operazione,sizeof(char)*id_size);
							
						//->	6
							/* invio al server 6, che significa che un ordine è preso in carico dal rider*/
							var = 6;
							FullWrite(servfd, & var, sizeof(int));
							
							/* invio id del rider al server */
							FullWrite(servfd, id_rider, sizeof(char)*id_size);
							
							/* invio id_operazione presa in carico dal rider */
							FullWrite(servfd, id_operazione, sizeof(char)*id_size);
							
							/* aspetto che il server mi invii l'id del client che ha effettuato l'ordine */
							FullRead(servfd, id_client, sizeof(char)*id_size);
							
							/* invio l'id del client al rider */
							FullWrite(sockfd, id_client, sizeof(char)*id_size);
							
							break;
						}
						
						else{ // se il rider non ha trovato ordini disponibili
						
						//->	2
							/* invio 2 al rider che indica che non può consegnare più l'ordine */
							var = 2;
							FullWrite(sockfd, &var, sizeof(int));
							
							break;
						}
						
						break;
					}
					
					/** NEL CASO SI RICEVA 2: VUOL DIRE CHE IL RIDER HA CONSEGNATO */
					case 5:{
						
						/*il rider mi invia un messaggio contenente l'id_operazione dell'ordine che ha inviato*/
						FullRead(sockfd, id_operazione, sizeof(char)*id_size);
						
				//->	7
						/* invio al server 7 -> ordine dato al rider consegnato */
						var = 7;
						FullWrite(servfd, & var, sizeof(int));
						
						/* invio al server id_operazione da eliminare */
						FullWrite(servfd,id_operazione,sizeof(char)*id_size);
						
						printf("Ordine %s effettuato dal rider: %s, al cliente %s.\n",id_operazione, id_rider, id_client);
						
						/* trovo l'ordine in questione e lo setto come consegnato */
						lo=find_l_ordine(order_info,id_operazione);
						lo->stato_ordine=3; 
						
						break;
					}
					
					default: // se ricevo un segnale non registrato
							// non faccio niente
						break;
						
                }
				 
                code = 9; 

                if (--ready <= 0) // se si sono visti tutti i fd in lettura, allora esci 
                    break;
            }

        }
		
    }

}
