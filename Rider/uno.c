#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<errno.h>
#include<time.h>
#include "../include/llist.h"


int main(int argc, char ** argv) {

	
/* ##################_INZIO INIZIALIZZAZIONE_########################## */
    if (argc != 3) {
        fprintf(stderr, "usage: %s <IPaddress> and <port>\n", argv[0]);
        //exit(1);
		argv[1]="127.0.0.1";
		argv[2]="1025";
    }
	int sockfd; //descrittore per comunicare con il ristorante
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        fprintf(stderr, "socket error\n");
        exit(1);
    }
	struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(atoi(argv[2]));
    if (inet_pton(AF_INET, argv[1], & servaddr.sin_addr) < 0) {
        fprintf(stderr, "inet_pton error for %s\n", argv[1]);
        exit(1);
    }
    if (connect(sockfd, (struct sockaddr * ) & servaddr, sizeof(servaddr)) < 0) {
        fprintf(stderr, "connect error\n");
        exit(1);
    }
/* ##################_INZIO INIZIALIZZAZIONE_########################## */

//************************ DICHIARAZIONI VARIABILI ***********************//
	//ID RIDER
	char id_rider[id_size];
	rand_string(id_rider,id_size);
    printf("RIDER CON ID: %s\n", id_rider);
		
    int n, cnt, verify;
    int var = 0, consegnaEffettuata = 0;
	char id_cliente[id_size], id_Operazione[id_size];
    int scelta;

//************************ FINE DICHIARAZIONI  ***********************//

    while (1) {
		
        cnt = 0;
        do{
	//-> 	0
			/* invia al ristorante 0-> intenzione di sapere quanti ordini ci sono*/  
			var = 0;
            FullWrite(sockfd, & var, sizeof(int));
			
			/* legge dal ristorante quanti ordini ci sono da consegnare*/
            FullRead(sockfd, & cnt, sizeof(int)); 

            if (cnt > 0) { // se ci sono ristoranti
				do {
					printf("\nCi sono nuovi ordini da consegnare");
					printf("\nPremere [1] per prenderlo in carico");
					printf("\nPremere [2] per rifiurare");
					printf("\n->");
					fflush(stdin);
					scanf("%d", & scelta);
					if (scelta != 1 && scelta !=2){
						printf("Opzione non disponibile. Sceglierne una tra quelle elencate.\n");
					}
				} while (scelta != 1 && scelta !=2);
            } 
			else { // se NON ci sono ristoranti
                do {
                    printf("\nAttualmente non c'è nessun ordine da consegnare.");
					printf("\nPremi [0] per aggiorare.");
					printf("\n-> ");
					fflush(stdin);
                    scanf("%d", &scelta);
                    if (scelta != 0) 
						printf("\nOpzione non disponibile. Sceglierne una tra quelle elencate.");
                } while (scelta != 0);
            }
        } while (scelta == 0 || scelta ==2); // cicla affinchè non abbia scelto il ristorante / accettato un ordine


//->	1
		/* invio messaggio 1 al ristorante -> ho intenzione di prendere in carico l'ordine */
        var = 1;
        FullWrite(sockfd, & var, sizeof(int));
		/* espatta una risposta di riuscita o meno */
        FullRead(sockfd, & verify, sizeof(int)); 
        if (verify == 0) { //riceve 1 se un'ordine è ancora disponibile, 0 in caso contrario
            printf("Non è stato possibile prendere in carico un ordine\n");
            continue; // riparte dalla scelta degli ordini
        }
		
		// se tutto è andato a buon fine (ho ricevuto 1 alla conferma della "presa" dell'ordine) ed ho preso in carico l'ordine:
		/* invio il mio id_rider al ristorante */
        FullWrite(sockfd, id_rider, sizeof(char)*id_size);
		
		/* ricevo l'id relativo all'ordine (e, di conseguenza, collegato alla Operazione del client) */
		FullRead(sockfd, id_Operazione, sizeof(char)*id_size);
		
		printf("Ordine [%s] da consegnare.\n",id_Operazione);
		
		/* ricevo l'id del client proprietario dell'ordine in consegna */
        FullRead(sockfd, id_cliente, sizeof(char)*id_size);
        
        /* SIMULO CONSEGNA */
		
		printf("\nInizio consegna...\n");
		// simulo invio
		sleep(5);
//->	2
		/* invio messaggio 2 al ristorante -> ho consegnato l'ordine */		
		var = 2;
        FullWrite(sockfd, & var, sizeof(int));
        printf("Consegna effettuata al cliente %s.\n", id_cliente);
        
        /* invio al ristorante la Operazione da "marchiare" in quanto cconsegnata */
        FullWrite(sockfd, id_Operazione, sizeof(char)*id_size);
       
       
    }
    exit(0);
} //fine main

