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

void before_close(int sig);

int serv;

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

/* ##################_FINE CREAZIONE SOKET_##################### */


/* ##################_ SOCKET RECEZIONE_########################## */
	int sockfd;
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
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
    if (connect(sockfd, (struct sockaddr * ) & servaddr, sizeof(servaddr)) < 0) {
        fprintf(stderr, "connect error\n");
        exit(1);
    }
/* ##################_FINE CREAZIONE SOCKET RECEZIONE_############### */


//------------------------ DICHIARAZIONE VAR ----------------------//
    
	
	int i,j; //invarianti
    int n, code, numProdotti;
    int fd_client, fd_rider; // fd dei client e dei rider
	char id_rider[id_size], id_client[id_size], id_richiesta[id_size];

    int var, loop;
    void *v;
	
    int conf, scelta, cnt, stat,x, consegna;
	
	node *tmp;

    Ordine *o, oo;
	L_ordini *lo;
	Prodotto p;
	char mmmm;


	char nome_rist[max_name];
	strcpy(nome_rist,"Palapizza");

	list* ordini = create_list(); //alloco lista per gli ordini
   	list* riders = create_list(); //alloco lista per i riders
   	list* stato_ordini = create_list();
	
   	list *menu=create_list();

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
	
//-------------------- FINE DICHIARAZIONE VAR ----------------------//


/* ##################_INIZIALIZZAZIONE CAMPI SELECT_########################## */
    serv=sockfd;
	
	fd_set rset;
    fd_set allset;
    int client[FD_SETSIZE];

    int maxd = listen_fd;
    int maxi = 1;

    if (sockfd > listen_fd)
        maxd = sockfd;

    for (i = 1; i < FD_SETSIZE; i++)
        client[i] = -1;

    client[0] = listen_fd;
    client[1] = sockfd;

    struct sockaddr_in cliaddr;
    socklen_t cliaddr_len;

    int ridfd;
    int connd;
    int ready;

    FD_ZERO( & allset);
    FD_SET(listen_fd, & allset);
    FD_SET(sockfd, & allset);
/* ##################_FINE INIZIALIZZAZIONE_########################## */

//*****************************_INIZIO_PROTOCOLLO_******************************************************//
	

//->	0	
	/* una volta che si connette gli invia un identificativo -> 0 "sono ristorante e ti sto per inviare i miei dati" */
	var = 0;
    FullWrite(sockfd, & var, sizeof(int));

	/* invio dati del ristorante al server*/
    FullWrite(sockfd, &nome_rist, sizeof(char)*max_name); //invio dei dati al server


    signal(SIGINT, before_close); //gestione segnale SIGINT ->invia prima il messaggio "10" al server (" sono il ristorante me ne sto andando libera il decrittore che mi hai allocato")
	
    while (1) {
        rset = allset;
        /* 
          chiama la select: 
            - esce quando un descrittore è pronto
            - restituisce il numero di descrittori pronti
        */
        if ((ready = select(maxd + 1, & rset, NULL, NULL, NULL)) < 0)
            perror("errore nella select");

		//* CONTROLLO SE IL DESCRITTORE PER LE CONNESSIONI E PRONTO IN LETTURA *//
		//* INDICA CHE QUALCUNO VUOLE CONNETTERSI AL RISTORANTE                *//
        // OVVIAMENTE SOLO I RIDER SI CONNETTERANNO
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
            FD_SET(connd, & allset);
            // registra il socket ed aggiorna maxd
            if (connd > maxd)
                maxd = connd;
            if (i > maxi)
                maxi = i;
            if (--ready <= 0)
                continue;
        }

        // controlla tutti i socket di ascolto se sono leggibili
        for (i = 1; i <= maxi; i++) {
			
			// se il client i-esimo non è settato (=-1), passa avanti
            if ((ridfd = client[i]) < 0)
                continue;

//*****************************_INIZIO_PROTOCOLLO_******************************************************//
			
			//sleep(1);
            if (FD_ISSET(ridfd, & rset)) {

				// ridfd -> fd rider; sockfd -> fd server, nei casi "-1" e "-2" sono scambiabili 
				
				/***********************************************************************/
				/*** AD OGNI CONNESSIONE INDIPENDENTE DAL CLIENT RICEVE UN MESSAGGIO ***/
				/*** QUESTO MESSAGGIO CONTIENE IL "COSA DEVE FARE" DEL RISTORANTE ******/
				/**** CONTROLLA IL TIPO DI MESSAGGIO IN INGRESSO ***********************/
				
				
                FullRead(ridfd, & code, sizeof(int));
				printf("\n--------------->CODICE: %d -> fd: %d \n",code, ridfd);
				

				switch (code) {
					
					/**** NEL CASO SI RICEVA IL MESSAGGIO -1 INDICA CHE IL SERVER VUOLE I PRODOTTI NEL MENU */
					case -1: { //Il ristorante invia i prodotti al Server
						//-> in questo caso ridfd == sockfd
						
				//->	3
						/* invio 3 per dire al server di posizionarsi nel caso in cui devi ricevere i prodotto */
						var = 3;
						FullWrite(sockfd, & var, sizeof(int));
						
						/* invio della quantità di prodotti */
						//cnt = size(menu);
						cnt=n_items;
						FullWrite(sockfd, &cnt, sizeof(int));
						tmp=(node*)menu->head;
						p = *(Prodotto*)tmp->data;
						print_struct_Prodotto(&p);
						for(i=0;i<cnt;i++){
							p = *(Prodotto*)tmp->data;
							/* invio prodotti */
							FullWrite(sockfd,&p,sizeof(p));
							tmp=tmp->next;
						}
						printf("\nHo inviato i (%d) prodotti richiesti dal server\n",i);
											
						break;
					}


					/**** NEL CASO SI RICEVA IL MESSAGGIO -2 INDICA CHE IL RISTORANTE DEVE RICEVERE L'ORDINE DAL SERVER*/
					case -2: { 
						
							/* ricezione della qt di oggetti nell'ordine */
							FullRead(sockfd,&cnt,sizeof(int));
							
							/* recezione dell'id della richiesta ordine*/
							FullRead(sockfd,id_richiesta,sizeof(char)*id_size);
							
							for(int i=0;i<cnt;i++){
								FullRead(sockfd,&oo,sizeof(oo));
								push_back(ordini,&oo);
							}
						
						printf("\n Ordine %s di %d elementi ricevuto dal server\n",id_richiesta, cnt);
						
						/* aggiunge ordine ricevuto in coda alla lista degli ordini*/
						push_back(stato_ordini, create_L_ordini(ordini,id_richiesta,0,-1));
						
						no++; // incrementa il numero degli ordini disponibili al rider;
						
						// elimino gli elementi della lista per futuri ordini
						 	//empty_list(ordini,free_Ordine);
						free(ordini);
						
						break;
					}

					/**** NEL CASO SI RICEVA IL MESSAGGIO 0 INDICA CHE IL RIDER VUOLE SAPERE QUANTI ORDINI CI SONO DA CONSEGNARE */
					case 0: {
					
						if (no <= 0){ // non ci sono ordini
							printf("\n Non ci sono attualmente ordini disponibili per i rider\n");
							cnt=0;
						}
						else
							printf("\n Ci sono %d ordini disponibili \n",no);
							
						/* invio il numero degli ordini disponibili al rider */
						FullWrite(ridfd, &no, sizeof(int));
						
						break;
					}
					
					/**** NEL CASO SI RICEVA IL MESSAGGIO 1 VUOL DIRE CHE IL RIDER E' DISPOSTO A CONSEGNARE UN ORDINE*/
					case 1: {
					
						tmp = stato_ordini->head;
						loop=1;
						x=0; // variabile assegnazione ordine;
						
						/* Ricerca ordine disponibile */
						while(tmp!=NULL && loop==1 && no>0){
							lo=(L_ordini*)tmp->data;
							x = lo->stato_ordine;
							printf("ordine %s = stato %d",lo->id_richiesta,x);
							if(x==0){ // ordine non assegnato a nessun rider -> di conseguenza lo assegno al rider.
								
								// setto lo stato ad 1 = "ordine assegnato ad un rider"
								lo->stato_ordine=1; 
								
								// decremento il numero di ordini disponibili ai rider
								no--; 
								
								// setto l'fd del rider attuale come attributo nella lista ordini
								lo->fd_rider=ridfd;
								
								//copio l'id della richiesta per inviarla al server
								stpcpy(id_richiesta,lo->id_richiesta);
								
								loop=0; // esco dal while- interrompo il ciclo
								
								break;
							}
							tmp=tmp->next;
						}
						
						if(loop==0){ // se un rider si è preso un ordine ed ha interrotto di conseguenza il ciclo
							
						//->	1
							/* invio 1 al rider che indica che può "accaparrarsi" l'ordine per la consegna */
							var =1;
							FullWrite(ridfd, &var, sizeof(int));
							
							/* allora il rider mi invia il suo id */
							FullRead(ridfd, id_rider, sizeof(char)*id_size);
							
							/* invio l'id_richiesta dell'ordine che ha preso in carico*/
							FullWrite(ridfd,id_richiesta,sizeof(char)*id_size);
							
						//->	5
							/* invio al server 5, che significa che un ordine è preso in carico dal rider*/
							var = 5;
							FullWrite(sockfd, & var, sizeof(int));
							
							/* invio id del rider al server */
							FullWrite(sockfd, id_rider, sizeof(char)*id_size);
							
							/* invio id_richiesta presa in carico dal rider */
							FullWrite(sockfd, id_richiesta, sizeof(char)*id_size);
							
							/* aspetto che il server mi invii l'id del client che ha effettuato l'ordine */
							FullRead(sockfd, id_client, sizeof(char)*id_size);
							
							/* invio l'id del client al rider */
							FullWrite(ridfd, id_client, sizeof(char)*id_size);
							
							break;
						}
						
						else{ // se il rider non ha trovato ordini disponibili
						
						//->	0
							/* invio 0 al rider che indica che non può consegnare più l'ordine */
							var = 0;
							FullWrite(ridfd, &var, sizeof(int));
							
							break;
						}
						
						break;
					}
					
					/**** NEL CASO SI RICEVA IL MESSAGGIO 2 VUOL DIRE CHE IL RIDER HA CONSEGNATO */
					case 2:{
						
						/*il rider mi invia un messaggio contenente l'id_richiesta dell'ordine che ha inviato*/
						FullRead(ridfd, id_richiesta, sizeof(char)*id_size);
						
				//->	8
						/* invio al server 8 -> ordine dato al rider consegnato */
						var = 8;
						FullWrite(sockfd, & var, sizeof(int));
						
						/* invio al server id_richiesta da eliminare */
						FullWrite(sockfd,id_richiesta,sizeof(char)*id_size);
						
						printf("Ordine %s effettuato dal rider: %s, al cliente %s.\n",id_richiesta, id_rider, id_client);
						
						/* trovo l'ordine in questione e lo setto come consegnato */
						lo=find_l_ordine(stato_ordini,id_richiesta);
						lo->stato_ordine=2; 
						
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



void before_close(int sig) {
    int var = 10;
    FullWrite(serv, & var, sizeof(int));
    exit(0);
}