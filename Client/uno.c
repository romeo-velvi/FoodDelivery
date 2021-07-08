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
int sockfd;

void before_close(int sig) {
    int var = 9;
    FullWrite(sockfd, & var, sizeof(int));
    exit(0);
}


int main(int argc, char ** argv) {

/* ##################_INZIO INIZIALIZZAZIONE_########################## */
    if (argc != 2) {
        fprintf(stderr, "usage: %s <IPaddress>\n Assegnazione automatica\n\n", argv[0]);
        //exit(1);
		argv[1]="127.0.0.1";
    }

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        fprintf(stderr, "socket error\n");
        exit(1);
    }

	struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(1024); // 1024 server; 1025 ristorante
    if (inet_pton(AF_INET, argv[1], & servaddr.sin_addr) < 0) {
        fprintf(stderr, "inet_pton error for %s\n", argv[1]);
        exit(1);
    }
    if (connect(sockfd, (struct sockaddr * ) & servaddr, sizeof(servaddr)) < 0) {
        fprintf(stderr, "connect error\n");
        exit(1);
    }
/* ##################_FINE  INIZIALIZZAZIONE_########################## */


//************************ DICHIARAZIONI VARIABILI ***********************//
//@TODO eliminare alcune fariabili

    //ID CLIENT
	char id_client[id_size];
	rand_string(id_client,id_size);
	printf("BENVENUTO client [%s]\n", id_client);
	int type=1; // 1= identifica il client nel protocollo
	
	//LISTE
	list* resturant; //per i ristoranti
    list* menu;//per il menu
    list* order; //per l'ordine
	
	void *v; //per il passaggio di dati
	int fdr; // per l'invio del fd del ristorante scelto
	int i,scelta; 
	int var;
	
    node *nodo; // variabile nodo
    Prodotto *p; 
    Prodotto prod;
    Ordine o;
	
    char *nome_scelto;
  
    int response;
    char id_rider[id_size];
    int loop = 1;
    int cnt;
    Ristorante *R;
	
    signal(SIGINT, before_close); //gestione segnale SIGINT ->invia prima il messaggio "6" al server ("sono il client, me ne sto andando libera il decrittore che mi hai allocato")
	

//************************* FINE DICHIARAZIONE *****************************//

    do {
    
		list* resturant = create_list(); //per i ristoranti
		list* menu = create_list(); //per il menu
		list* order = create_list(); //per l'ordine
    	
        do {
            //system("clear");
			
		//->	1
			fdr=-1;
			/* una volta che si connette gli invia un identificativo -> 1 "sono client e aspetto i ristoranti" */
            FullWrite(sockfd, & type, sizeof(int));
			
			/* il server mi risponde con una lista dei ristoranti */
			/* si riceve il numero dei irsoranti nella lista */
            FullRead(sockfd, & cnt, sizeof(int));


            if (cnt > 0) { // se ci sono ristoranti
				printf("\nCi sono attualmente %d ristoranti attivi:\n",cnt);
				
				R=malloc(sizeof(Ristorante));
				for(int i=0;i<cnt;i++){
					FullRead(sockfd,R,sizeof(Ristorante));
					push_front(resturant,create_Ristorante(R->nome_rist,R->fd_rist));
				}	
				loop=0; // non richiedere più quanti ristoranti ci sono al client  -> già li hai
            } 
			else { // se NON ci sono ristoranti
                do {
                    printf("Attualmente non è connesso nessun ristorante.\n");
					printf("Premere [1] per aggiornare la lista. \n");
					printf("Premere [2] per uscire. \n");
					printf("\n -> ");
					fflush(stdin);
                    scanf("%d", & scelta);
                    if (scelta < 1 || scelta > 2) 
						printf("Opzione non disponibile, reinserire: ");
					else if ( scelta == 2){
						before_close(SIGINT); // esci
					}
                } while (scelta != 1);
            }
			
        } while (loop); // cicla affinchè non abbia la lista dei ristoranti


		/* funzione che dati i ristoranti, permette di sceglierne uno e di ritornare il fd aperto sul server che gestisce quest'ultimo */
		fdr = show_choose_resturant(resturant);	

	//->	2
		/* invio messaggio contenente 2 -> "ti sto per inviare l'fd del ristorante scelto, aspetto il menu" */
        var = 2; 
        FullWrite(sockfd, &var, sizeof(int));

        /* invio il fd del ristorante scelto */
		FullWrite(sockfd, &fdr, sizeof(int));
		
		/* in risposta, il server mi inva il numero di prodotti nel menu */
        FullRead(sockfd, & cnt, sizeof(int));

		// riceve n pacchetti contenenti i prodotto
		for(i=0;i<cnt;i++){
			// riceve il pacchetto
			FullRead(sockfd,&prod,sizeof(Prodotto));
					//p=(Prodotto*)v;
			// inserire elemento nella lista
			push_front(menu,create_Prodotto(prod.items,prod.prezzo));
		}
		printf("\nho ricevuto i (%d) prodotti dal server\n",i);
        
		/* crea ed invia ordine (serie elaborate di struct ordine)*/
		printf("\nMenu del ristorante selezionato: \n");
		order = show_choose_product(menu);
		
		// stampo order
		traverse(order,print_struct_Ordine);
		
	//->	4		
		/* invia un messaggio contenente 4 -> "ti invio l'ordine, aspetto l'fd del rider che mi deve fare la consegna" */
        var = 4;
        FullWrite(sockfd, &var, sizeof(int));
		
		// si ricava il size dell'ordine
		cnt=size(order);
		/* invio quanti pacchetti dovrà ricevere */
		FullWrite(sockfd,&cnt,sizeof(int));
		// ricezione dei pacchetti contenenti i nomi dei ristoranti
		nodo=order->head;
		for(int i=0;i<cnt && nodo!=NULL;i++){
			o = *(Ordine*)nodo->data;
			// invia pacchetto
			FullWrite(sockfd,&o,sizeof(o));
			nodo=nodo->next;
		}
				
				
		/* aspetta la recezione dell'id rider */
        FullRead(sockfd, id_rider, sizeof(char)*id_size);
        
        FullWrite(sockfd, id_client, sizeof(char)*id_size);
		
        printf("\nConsegan presa in carica dal rider %s.\n", id_rider);

        do {
            printf("\nPremi [1] per effettuare un altro ordine");
			printf("\nPremi [2] per uscire.");
			printf("\n-> ");
			fflush(stdin);
            scanf("%d", & scelta);
            if (scelta < 1 || scelta > 2) 
				printf("Opzione non disponibile. Sceglierne una tra quelle elencate.\n");
        } while (scelta < 1 || scelta > 2);
		
	/* cancellare precedenti liste */
	empty_list(resturant,free_Ristorante); //per i ristoranti
	empty_list(menu,free_Prodotto); //per i ristoranti
	empty_list(order,free_Ordine); //per i ristoranti

	
    } while (scelta == 1);
	
    if (scelta == 2) {
        before_close(SIGINT);
    }
//*****************************_FINE_PROTOCOLLO_******************************************************//

    exit(0);
} //fine Main


