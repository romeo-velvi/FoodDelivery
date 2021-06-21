
#ifndef LLIST_H
#define LLIST_H

#define max_name 1024
#define id_size 6

/***************************************************
** Prototypes for struct.  						  **
***************************************************/

// STRUCT PER LISTE
typedef struct lnode
{
  struct lnode* prev; 
  /* Pointer to previous node */
  struct lnode* next; 
  /* Pointer to next node */
  void* data; 
  /* User data */
} node;

typedef struct llist
{
  struct lnode* head; 
  /* Head pointer either points to a node with data or NULL */
  struct lnode* tail; 
  /* Tail pointer either points to a node with data or NULL */
  unsigned int size; 
  /* Size of the linked list */
} list;


/* STRUCT PROTOCLLO */

typedef struct Prodotto{
	char items[max_name]; // oggetti nel menu (vedere il passaggio da [] a * nell'inizio di ristorante {quando fa il Menu statico})
	float prezzo;
}Prodotto;

typedef struct n_q{
	char items[max_name];
	int qt;
} Ordine;

typedef struct ristorante{
	char nome_rist[max_name];
	int fd_rist; // quando invio l'ordine (client-part in server)
} Ristorante;

typedef struct rider{
	char id_rider[id_size];
	int fd_rider;
} Rider;


typedef struct richiesta { //Tiene traccia delle richieste client-ristorante nel server
	char id_richiesta[id_size];
    int fd_client;
    int fd_ristorante;
    int stato_richiesta; 
	/*
		SE LO STATO E UGUALE A:
			3:  ->  per indicare che server prende un qualsiasi client (indicato dal campo "client") che fa richiesta del menu ricevuto dal ristorante.
			4:  ->	per indicare che il serve prende il ristorante (indicato dal campo "ristorante") e gli inoltre l'ordine ricevuto dal client.
			5:  ->  per indicare che il serve prende il client (indicato dal campo "client") e gli inoltre l'id del rider ricevuto dal ristorante.
			6:  ->  vuol dire che l'ordine è in fase di consegna
	*/
} Richiesta;


typedef struct lista_ordini{
	list* ordini;
	char id_richiesta[id_size]; //per identificarla nel server.
	int fd_rider; // rider che effettuerà la consegna
	int stato_ordine;
	/*
		SE LO STATO E UGUALE A:
		0: -> ordine nuovo
		1: -> ordine assegnato
		2: -> ordine consegnato
	*/
} L_ordini;


/***************************************************
** Prototypes for linked list library functions.  **
**                                                **
** For more details on their functionality,       **
** check llist.c.                                 **
***************************************************/

/* Creating */
static node* create_node(void* data);
list* create_list(void);

/* Adding */
void push_front(list* llist, void* data);
void push_back(list* llist, void* data);

/* Removing */
int remove_front(list* llist, void (*free_func)(void *data));
int remove_back(list* llist, void (*free_func)(void *data));

/* Querying List */
void* front(list* llist);
void* back(list* llist);
int is_empty(list* llist);
int size(list* llist);

/* Freeing */
void empty_list(list* llist, void (*free_func)(void *data));

/* Traversal */
void traverse(list* llist, void (*do_func)(void *data));


/***************************************************
** Prototypes for function for protocol.  		  **
**                                                **
** For more details on their functionality,       **
** check llist.c.                                 **
***************************************************/

/* CONTROLLARE LA CORRISPONDENZA TIPI-DATO E VALORI PASSATI */

// cancella elemento in mezzo
void delete_node(list* llist, node* llnode);

// funzioni per creare gli oggetti
Prodotto* create_Prodotto (char *items, float prezzo);
Ristorante* create_Ristorante (char*nome_R, int fd_rist);
Ordine* create_Ordine (char*items,int qt);
Rider* create_Rider (char*id_rider, int fd_rider);
Richiesta* create_Richiesta(char *n, int a, int b, int c);
L_ordini* create_L_ordini(list*l, char *c, int s, int fd_rider);

// funzioni per cancellare gli oggetti
void free_Prodotto(void*data);
void free_Ristorante(void*data);
void free_Ordine(void*data);
void free_Rider(void*data);
void free_Richiesta(void*data);
void free_L_ordini(void *data);

// funzioni per stampare i risultati dei dati passati
void print_struct_Prodotto(void*data);
void print_struct_Ristorante(void*data);
void print_struct_Ordine(void*data);
void print_struct_Rider(void*data);
void print_struct_Richiesta(void*data);
void print_struct_L_ordini(void*data);




/* §§§§§§§§§§§ ALTRE FUNZIONI UTILI §§§§§§§§§§§*/

// stampa e scelta Ristorante nella list (con arg i nomi dei Ristorante) {#CLIENT}
int show_choose_resturant(list* llist);  // GLI RITORNA DIRETTAMENTE L'ID

// stampa e scelta Prodotti nella list (menu) {#CLIENT}
list* show_choose_product(list* llist);

// generare un stringa random {#SERVER} {#RIDER}
char *rand_string(char *str, size_t size);

// Function che cerca nella lista di Ristorante per ritornare il nodo {#SERVER}
node* find_resturant_by_name (list*l,char*nome_rist);
node* find_resturant_by_fd (list*l, int fd);

// ritorna Rider nella lista situato alla posizione x {#RISTORANTE}
Rider* find_Rider(list *llist, int x);

// invia messaggio a tutti i rider {#Ristorante}
void send_rider_message(list *l);

// cerca la richiesta dato l'fd del client {#SERVER}
Richiesta* find_resturant_request(list *l,int fd,int st);

// cerca la richiesta dato l'fd del ristorante {#SERVER}
Richiesta* find_client_request(list *l,int fd,int st);

// cerca la richiesta dato l'id della richiesta {#SERVER}
Richiesta* find_id_request(list *l, char*id);

// ritorna il nodo nella lista che ha id pari a quello dato {#SREVER}
node* find_id_request_node(list*l, char*id);

// cerca tra i l_o con fd pari a quello dato come argomento. {#RISTORANTE}
L_ordini* find_l_ordine(list*l, char *id);

//* FULL READ & FULL WRITE*//
void FullRead(int fd, void * buf, size_t count); //funzione per la gestione del valore di ritorno nella fullRead
void FullWrite(int fd, const void * buf, size_t count); //funzione per la gestione del valore di ritorno nella fullWrite
ssize_t fullRead(int fd, void * buf, size_t count);
ssize_t fullWrite(int fd, const void * buf, size_t count);


#endif
