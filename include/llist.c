#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include "llist.h"


/***************************************************
** Prototypes for linked list library functions.  **
***************************************************/




static node* create_node(void* data){
    node* newNode = malloc(sizeof(node));
    newNode->prev = NULL;
    newNode->next = NULL;
    newNode->data = data;
    return newNode;
}




list* create_list(void)
{
     list* newList = malloc(sizeof(list));
    if (!newList){
      return NULL;
    }
    newList->head = NULL;
    newList->tail = NULL;
    newList->size = 0;
    return newList;
}




void push_front(list* llist, void* data)
{
    node* llnode = create_node(data);
    llnode->data = data;
    llnode->next = llist->head;
    llist->head = llnode;
    if (llist->tail == NULL) {
      llist->tail = llnode;
    } else {
      llnode->next->prev = llnode;
    }
    llist->size++;
}




void push_back(list* llist, void* data)
{
    node* llnode = create_node(data);
    llnode->data = data;
    llnode->prev = llist->tail;
    llist->tail = llnode;
    if (llist->head == NULL){
      llist->head = llnode;
    } else {
      llnode->prev->next = llist->tail;
    }
    llist->size++;
}




void* front(list* llist)
{

  if (llist->head == NULL) {
    return NULL;
  } else {
    node* llnode = llist->head;
    return llnode->data;
  }
}



void* back(list* llist)
{
  if (llist->head == NULL) {
    return NULL;
  } else {
    node* llnode = llist->tail;
    return llnode->data;
  }
}



int remove_front(list* llist, void (*free_func)(void *data))
{

    if (llist->head == NULL){
      return -1;
    } else if (llist->head == llist->tail){ 
      node* llnode = llist->head;
      free_func(llnode->data);
      free(llnode);
      llist->head = NULL;
      llist->tail = NULL;
      llist->size = 0;
      return 0;
    } else { 
      node* llnode = llist->head;
      free_func(llnode->data);
      llist->head = llnode->next;
      llnode->next->prev = NULL;
      free(llnode);
      llist->size--;
      return 0;
    }
}



int remove_back(list* llist, void (*free_func)(void *data))
{
    if (llist->head == NULL){
      return -1;
    } else if (llist->head == llist->tail){ 
      node* llnode = llist->head;
      free_func(llnode->data);
      free(llnode);
      llist->head = NULL;
      llist->tail = NULL;
      llist->size = 0;
      return 0;
    } else { 
      node* llnode = llist->tail;
      free_func(llnode->data);
      llist->tail = llnode->prev;
      llnode->prev->next = NULL;
      free(llnode);
      llist->size--;
      return 0;
    }}




int size(list* llist)
{
    return llist->size;
}




int is_empty(list* llist)
{

  if ((llist->size == 0) && (llist->head == NULL)) {
    return 1;
  } else {
    return 0;
  }
}




void empty_list(list* llist, void (*free_func)(void *))
{

    if (llist == NULL) {
      return;
    }

    node* llnode = llist->head;
    node* temp;

    while (llnode != NULL)
    {
      temp = llnode;
      free_func(llnode->data);
      llnode = llnode->next;
      free(temp);
    }

    llist->head = NULL;
    llist->tail = NULL;
    llist->size = 0;
    return;
}




void traverse(list* llist, void (*do_func)(void *))
{
    if (llist == NULL) {
      return;
    }
    node* llnode = llist->head;
    while (llnode != NULL) {
      do_func(llnode->data);
      llnode = llnode->next;
    }
}







/***************************************************
** Prototypes for function for protocol.  		  **
***************************************************/



/* FUNZIONE DI CREAZIONI ELEMENTI */





Info* create_Info(int id, int fd, char*c){
    Info* p = (Info*) malloc(sizeof(Info));
    p->id = id;
    p->fd = fd;
    p->c = c;
    return p;
}

Prodotto* create_Prodotto (char *items, float prezzo){
    Prodotto* p = (Prodotto*) malloc(sizeof(Prodotto));
    /*items=malloc(sizeof(char)*max_name);
	for (int i=0;i<max_name;i++){
	    p->items[i]=items[i];
	}*/
	strcpy(p->items,items);
    p->prezzo = prezzo;
    return p;
}

Ristorante* create_Ristorante (char*nome_R, int fd_rist){
    Ristorante* p = (Ristorante*) malloc(sizeof(Ristorante));
    strcpy(p->nome_rist,nome_R);
    p->fd_rist = fd_rist;
    return p;
}


Ordine* create_Ordine (char*items, int qt){
    Ordine* p = (Ordine*) malloc(sizeof(Ordine));
    strcpy(p->items,items);
    p->qt = qt;
    return p;
}


Rider* create_Rider (char*id_rider, int fd_rider){
    Rider* p = (Rider*) malloc(sizeof(Rider));
    strcpy(p->id_rider, id_rider);
    p->fd_rider = fd_rider;
    return p;
}



Operazione* create_Operazione (char *n, int a, int b, int c){
    Operazione* p = (Operazione*) malloc(sizeof(Operazione));
	stpcpy(p->id_operazione,n);
    p->fd_client=a;
    p->fd_ristorante=b;
    p->stato_operazione=c;
    return p;
}

Info_ordini* create_Info_ordini (list *l, char *c, int s, int x){
    Info_ordini* p = (Info_ordini*) malloc(sizeof(Info_ordini));
	strcpy(p->id_operazione,c);
    p->ordini=l;
    p->stato_ordine=s;
	p->fd_rider=x;
    return p;
}



/* FUNZIONI PER ELIMINARE UN ELEMENTO */






void free_Info(void* data)
{
    Info *p = (Info*) data;
    free(p);
}

void free_Prodotto(void*data){
	Prodotto *p = (Prodotto*) data;
    free(p);
}


void free_Ristorante(void*data){
	Ristorante *p = (Ristorante*) data;
    free(p);
}


void free_Ordine(void*data){
	Ordine *p = (Ordine*) data;
    free(p);
}


void free_Rider(void*data){
	Rider *p = (Rider*) data;
    free(p);
}


void free_Operazione(void*data){
	Operazione *p = (Operazione*) data;
    free(p);
}

void free_Info_ordini(void*data){
	Info_ordini *p = (Info_ordini*) data;
    free(p);
}




/* FUNZIONI PER STAMPARE UN ELEMENTO */





void print_struct_Info(void *st)
{
    if (st == NULL) 
        return;
    Info *tmp = (Info*)st;
    printf("\n id:%d, ex/fd:%d, string:%s\n",tmp->id, tmp->fd, tmp->c);
}

void print_struct_Prodotto(void*st){
    if (st == NULL) 
        return;
    Prodotto *tmp = (Prodotto*)st;

}

void print_struct_Ristorante(void*st){
    if (st == NULL) 
        return;
    Ristorante *tmp = (Ristorante*)st;
    printf("\nNome Ristorante: %s con FD: %d",tmp->nome_rist, tmp->fd_rist);
    /* se si vuole visualizzare anche il menu
    print_struct_Menu(&(tmp->M)); */ // prova anche con tmp->(&M)
}


void print_struct_Ordine(void*st){
    if (st == NULL) 
        return;
    Ordine *tmp = (Ordine*)st;
    printf("\nElement:%s, in quantita':%d\n",tmp->items,tmp->qt);
    
}

void print_struct_Rider(void*st){
    if (st == NULL) 
        return;
    Rider *tmp = (Rider*)st;
    printf("\nRider:%s, su fd:%d\n",tmp->id_rider,tmp->fd_rider);
}


void print_struct_Operazione(void*st){
    if (st == NULL) 
        return;
    Operazione *tmp = (Operazione*)st;
	printf("\n Operazione id:%s",tmp->id_operazione);
    printf("fd_client:%d, fd_ristorante:%d, stato:%d\n",tmp->fd_client,tmp->fd_ristorante,tmp->stato_operazione);
}

void print_struct_Info_ordini(void*st){
    if (st == NULL) 
        return;
    Info_ordini *lo = (Info_ordini*)st;
	list*l=(list*)lo->ordini;
	node*tmp=l->head;
	printf("\nStato ordine: %d\n",lo->stato_ordine);
	int i=0;
	printf("\nOrdinazione %s",lo->id_operazione);
	while(tmp!=NULL){
		Ordine*o=tmp->data;
		printf("\n[%d] Piatto: %s, quantità :%d\n",i,o->items, o->qt);
		tmp=tmp->next;
		i++;
	}
	
}


/* FUNZIONI UTILI */



void delete_node(list* llist, node* llnode){
	if(llnode==NULL)
		return;
	if(llnode->prev==NULL)
		return;
	if(llnode->next==NULL){
		llnode->prev->next=(node*)NULL;
		free(llnode);
		return;
	}
	
	llnode->prev->next=llnode->next;
	free(llnode);
	llist->size--;
}




int show_choose_resturant(list* llist){
	// visualizzo Ristorante
	if(llist==NULL)
		return -1;
    node* llnode = llist->head;
    // mostro i Ristorante
    Ristorante *r;
    int i = 0;
    int arg [size(llist)];
    while (llnode != NULL) {
    	r=(Ristorante*)llnode->data;
    	printf("[%d] %s\n",i,r->nome_rist);
		llnode = llnode->next;
		arg[i]=r->fd_rist;
		i++;
    }
    // scelgo Ristorante
    int cnt= size(llist);
    int scelta;
    int loop=1;
    while(loop){
		printf("\nInserire il numero del ristorante scelto: ");
		fflush(stdin);
		scanf("%d",&scelta);
		if(scelta>=0 && scelta<cnt){
			int fdr= arg[scelta];
			return fdr;
		}
		else{
			printf("\nID ristorante inserito non trovato!");
		}
	}

	return -1;
}




list* show_choose_product(list* llist){
	// visualizzo Ristorante
	if(llist==NULL)
		return (list*)NULL;
	
    int cnt=size(llist);
	char *ar[cnt];
    node* llnode = llist->head;
    // mostro i Ristorante
    Prodotto *r;
    for (int i=0;i<cnt && llnode != NULL;i++) {
    	r=(Prodotto*)llnode->data;
    	fflush(stdin);
    	printf("[%d] %s %.2f\n",i,r->items,r->prezzo);
    	ar[i]=malloc(sizeof(char)*max_name);
    	ar[i]=r->items;
		llnode = llnode->next;
    }
    // scelgo Ristorante
    list* l=create_list();
    int scelta;
    int opzione;
    int qt;
    int loop=1;
    while(loop){
    	
    	printf("\nInserire [1] per scegliere un prodotto.\nInserire [2] per uscire\n->  ");
		scanf("%d",&scelta);
		if(scelta==1){
			printf("\nInserire id del prodotto: ");
			scanf("%d",&scelta);
			if(scelta>=0 && scelta<cnt){
				printf("\nInserire quantita': ");
				scanf("%d",&qt);
				if(qt!=0){
					push_front(l,create_Ordine(ar[scelta], qt));		
				}

			}
			else{
				printf("\nID prodotto inserito non valido!");
			}
		}
		else if(scelta==2){
			printf("\nOrdine composto!");
			loop=0;
		}
		else{
			printf("\nComando non valido");	
		}
	}
	return l;
}




char *rand_string(char *str, size_t size)
{
	srand(time(NULL));
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJK12345";
    if (size) {
        --size;
        for (size_t n = 0; n < size; n++) {
            int key = rand() % (int) (sizeof charset - 1);
            str[n] = charset[key];
        }
        str[size] = '\0';
    }
    return str;
}




node* find_resturant_by_name (list*l,char*nome_rist){
	if(l==NULL)
		return (node*)NULL;
	node*n=l->head;
	Ristorante *r;
	while(n!=NULL){
		r=(Ristorante*)n->data;
		if(strcmp(r->nome_rist,nome_rist)==0){
			return n;
		}
		n=n->next;
	}
	return (node*)NULL;
}




node* find_resturant_by_fd (list*l,int fd_rist){
	if(l==NULL)
		return (node*)NULL;
	node*n=l->head;
	Ristorante *r;
	while(n!=NULL){
		r=(Ristorante*)n->data;
		if(r->fd_rist==fd_rist){
			return n;
		}
		n=n->next;
	}
	printf("\n--------->Nodo non trovato\n");
	return (node*)NULL;
}




Operazione* find_resturant_request(list *l,int fd,int st){
	if(l==NULL)
		return (Operazione*)NULL;
	node*n=l->head;
	Operazione *r;
	while(n!=NULL){
		r=(Operazione*)n->data;
		if(r->fd_client==fd && r->stato_operazione==st){
			return r;
		}
		n=n->next;
	}
	printf("\n--------->Nodo non trovato\n");
	return (Operazione*)NULL;
}




Operazione* find_client_request(list *l,int fd,int st){
	if(l==NULL)
		return (Operazione*)NULL;
	node*n=l->head;
	Operazione *r;
	while(n!=NULL){
		r=(Operazione*)n->data;
		if(r->fd_ristorante==fd && r->stato_operazione==st){
			return r;
		}
		n=n->next;
	}
	printf("\n--------->Nodo non trovato\n");
	return (Operazione*)NULL;
}



Operazione* find_id_request(list *l,char* id){
	if(l==NULL)
		return (Operazione*)NULL;
	node*n=l->head;
	Operazione *r;
	while(n!=NULL){
		r=(Operazione*)n->data;
		if(strcmp(r->id_operazione,id)==0){
			return r;
		}
		n=n->next;
	}
	printf("\n--------->Nodo non trovato\n");
	return (Operazione*)NULL;
}



node* find_id_request_node(list*l, char*id){
	if(l==NULL)
		return (node*)NULL;
	node*n=l->head;
	Operazione *r;
	while(n!=NULL){
		r=(Operazione*)n->data;
		if(strcmp(r->id_operazione,id)==0){
			return n;
		}
		n=n->next;
	}
	printf("\n--------->Nodo non trovato\n");
	return (node*)NULL;
}



Info_ordini* find_l_ordine(list*l, char *id){
	
	if(l==NULL)
		return (Info_ordini*)NULL;
	node*n=l->head;
	Info_ordini *r;
	while(n!=NULL){
		r=(Info_ordini*)n->data;
		if(strcmp(r->id_operazione,id)==0){
			return r;
		}
		n=n->next;
	}
	printf("\n--------->Nodo non trovato\n");
	return (Info_ordini*)NULL;
}






ssize_t fullRead(int fd, void * buf, size_t count) {
    size_t nleft;
    ssize_t nread;
    nleft = count;
    while (nleft > 0) {
        if ((nread = read(fd, buf, nleft)) < 0) {

            if (errno == EINTR)
                continue;
            else
                exit(nread);
        } else if (nread == 0) {
            break;
        } else {
            nleft -= nread;
            buf += nread;
        }
    }
    buf = 0;
    return nleft;
}

ssize_t fullWrite(int fd, const void * buf, size_t count) {
    size_t nleft;
    ssize_t nwritten;
    nleft = count;
    while (nleft > 0) {
        /* repeat until no left */
        if ((nwritten = write(fd, buf, nleft)) < 0) {
            if (errno == EINTR)
                continue;
            else exit(nwritten);

        }

        nleft -= nwritten;
        buf += nwritten;
    }
    return nleft;
}

void FullRead(int fd, void * buf, size_t count) {
    if ((fullRead(fd, buf, count)) == -1)
        perror("Errore nella read.\n");
}

void FullWrite(int fd,
    const void * buf, size_t count) {
    if ((fullWrite(fd, buf, count)) == -1)
        perror("Errore nella write.\n");
}













