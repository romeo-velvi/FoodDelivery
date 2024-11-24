# Food Delivery Project

## Scopo del progetto
Questo progetto nasce con lo scopo di fornire una struttra base di comunicazione astratta (basata su socket) che permette il rider-delivery di servizi ed elementi (in questo caso, di alimenti).

## Descrizione del Progetto
Il progetto consiste nella progettazione e implementazione di un servizio di food delivery. Il client si collega al server per ricevere la lista dei ristoranti disponibili. L'utente sceglie un ristorante tramite l'interfaccia del client, effettua un ordine e il server gestisce la comunicazione tra il ristorante e i rider per la consegna.

Questo sistema nasce come progetto per il corso universitario "reti di calcolatori".  

## Tecnologie Utilizzate
- **Linguaggio**: C
- **Comunicazione**: Socket per la comunicazione tra processi
- **Gestione I/O**: IO/Multiplexing implementato tramite `select`

## Architettura
L'architettura del sistema è composta da quattro entità principali:
1. **Server**: Gestisce le comunicazioni tra le altre entità.
2. **Cliente**: Si connette al server per visualizzare i ristoranti e fare ordini.
3. **Ristorante**: Gestisce gli ordini e le connessioni con i rider.
4. **Rider**: Si connette al ristorante per ricevere e consegnare gli ordini.

## Protocollo di Comunicazione
Il protocollo di comunicazione prevede diverse fasi, tra cui:
- Connessione del client al server e ricezione della lista dei ristoranti.
- Scelta del ristorante e ricezione del menu.
- Invio dell'ordine dal client al server e poi al ristorante.
- Gestione della disponibilità dei rider e assegnazione dell'ordine.
- Notifica di consegna completata dal rider al ristorante e poi al server.

## Strutture Dati
Sono state utilizzate diverse strutture dati, tra cui:
- **Lista doppiamente linkata**: Per memorizzare e organizzare i dati.
- **Prodotto**: Contiene il nome e il prezzo degli articoli nel menu.
- **Ordine**: Memorizza gli ordini dei clienti.
- **Ristorante**: Memorizza i ristoranti attivi.
- **Rider**: Memorizza i rider attivi.
- **Operazione**: Gestisce le operazioni tra client e ristorante.
- **Info_ordine**: Gestisce gli ordini e le consegne.

## Dettagli Implementativi
Le entità utilizzano socket per le operazioni di lettura e scrittura. Il server utilizza l'IO/Multiplexing per la gestione sincrona dell'I/O. Sono state implementate funzioni wrapper per garantire la lettura e scrittura completa delle informazioni.

## Manuale Utente
### Compilazione
Ogni cartella contiene un Makefile. Per compilare i programmi, eseguire il comando `make` nella cartella dell'entità.

### Esecuzione
Per eseguire il codice, avviare prima il Server, poi il Ristorante, il Rider e infine il Cliente. La sequenza consigliata è:
```sh
make
./server
./ristorante
./rider
./cliente
