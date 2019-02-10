//Andrea Aspesi 2018 - Simulatore Macchina di Turing a nastro singolo e non deterministica

//Ottimizzazione:
//-eliminazione stati pozzo in createArrayTransizioni
//-passato agli short int
//-controllo ogni CONTROLLO_LISTA_UGUALE volte se nella mia lista di stati da cui partire non ne ho alcuni da cui son già partito, in tal caso li elimino
//-eliminato transizioni IDENTICHE
//-tengo salvata la lunghezza delle stringhe per non fare la strlen
//-il max num di computazioni diventa un unsigned long
//-ogni ciclo controllo che nella mia lista di nuovi stati da cui partire, non ce ne siano di duplicati
//-elimino le transizioni che mi portano a stati inesistenti e non finali -> no miglioramenti visti
//-executeMovements ora riutilizza la memoria allocata, sovrascrivendo, ove possibile, lo stato precedente

//#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
//#include <crtdbg.h>

#define CONTROLLO_LISTA_UGUALE 1000
#define SET_CHECK_GIRO 5

#define CHAR_POZZI '~'
#define CHAR_SEMPRE_SX '}'
#define CHAR_SEMPRE_DX '{'
#define POSIZIONE_INIZIO_STRINGA_INPUT 0

//Definizioni Struct
typedef struct transizioniS {
	short statoStart, statoEnd;
	char leggo, scrivo, testina, speciale;//speciale mi tiene in considerazione cicli infiniti e pozzi
	struct transizioniS *next;
} transizioni;

typedef struct stringheInputS {
	char* str;
	int len;
	struct stringheInputS *next;
} stringheInput;

typedef struct statoMacchinaS { //Lo uso per far la lista di tutti gli stati in cui si trova la lista in quell'istante
	short numeroStato;
	char* str;
	int len;
	int pos;//posNellaStr
	struct statoMacchinaS *next;
} statoMacchina;

//Prototipi funzioni
int inputFromFile(transizioni **, int **, stringheInput *, unsigned long *);
transizioni **createArrayTransizioni(transizioni *, int[], int);
char executeTM(transizioni **, int[], statoMacchina *, unsigned long);
statoMacchina* executeMovements(transizioni **, statoMacchina *);//l'idea del controllare qui se è uno stato finale, rallenta perchè fa i cicli molte più volte

//variabili globali
int dimArrayStatiAccett;
int foundPozzo;
int foundSameStatus;
int foundInfiniteMov;

int main(int argc, char* argv[]) {
	unsigned long maxC = 0;
	int statoMax;    //stato col numero più grande
	transizioni *listaTransizioni, *listaTransizioniTmp;
	transizioni **arrayTransizioni;
	int *statiAccettazione;
	stringheInput *listaStringheInput, *listaStringheInputTmp;

	statoMacchina *stati;
	char result = ' ';

	//assegnamenti
	statiAccettazione = (int *)malloc(sizeof(int));
	listaStringheInput = (stringheInput *)malloc(sizeof(stringheInput));
	listaTransizioni = (transizioni *)malloc(sizeof(transizioni));

	//Load data from input
	statoMax = inputFromFile(&listaTransizioni, &statiAccettazione, listaStringheInput, &maxC);

	arrayTransizioni = createArrayTransizioni(listaTransizioni, statiAccettazione, statoMax);

	do {
		stati = (statoMacchina *)malloc(sizeof(statoMacchina));
		stati->numeroStato = 0;
		stati->next = NULL;
		stati->pos = POSIZIONE_INIZIO_STRINGA_INPUT;
		stati->str = listaStringheInput->str;
		stati->len = listaStringheInput->len;

		result = executeTM(arrayTransizioni, statiAccettazione, stati, maxC);
		printf("%c\n", result);

		listaStringheInputTmp = listaStringheInput->next;
		free(listaStringheInput);
		listaStringheInput = listaStringheInputTmp;
	} while (listaStringheInput != NULL);

	//_CrtDumpMemoryLeaks();
	return 0;
}

//ritorna il numero dello stato più big
int inputFromFile(transizioni **listaTransizioni, int **statiAccettazione, stringheInput *listaStringheInput, unsigned long *maxC) {
	char *in, *resStr;
	transizioni *x;
	int posInStatiAccettazione = 0;
	stringheInput *lastInput;
	void *ptrPenultimoElem = NULL;
	int tmp;
	int statoMax = 0;

	in = (char *)malloc(100);

	//init input
	lastInput = listaStringheInput;
	lastInput->next = NULL;

	#ifdef _WIN32
	    FILE *io = fopen("inputFile.txt", "r"); //On Windows open a specified file
	#else
	    FILE *io = stdin; //on Linux read from terminal (CAT of the input file)
	#endif

	if (io == NULL)
		printf("Errore");
	if (fscanf(io, "%s", in)) {
		if (strcmp(in, "tr") == 0) {

			x = *listaTransizioni;
			x->next = NULL;
			while (fscanf(io, "%hd %c %c %c %hd", &(x->statoStart), &(x->leggo), &(x->scrivo), &(x->testina), &(x->statoEnd)) == 5) {
				if (statoMax < x->statoStart)
					statoMax = x->statoStart;
				if (statoMax < x->statoEnd)
					statoMax = x->statoEnd;

				x->next = (transizioni *)malloc(sizeof(transizioni));
				ptrPenultimoElem = x;
				x = x->next;
				x->next = NULL;
			}
			free(x);//l'ultimo elemento creato in realtà non esiste
			((transizioni *)ptrPenultimoElem)->next = NULL;


			if (fscanf(io, "%s", in)) {
				if (strcmp(in, "acc") == 0) {
					//ora leggo gli stati di accettazione -> attenzione, io gli ho passato alla funzione un puntatore a puntatore
					int *tmpStati = (int *)malloc(sizeof(int));
					while (fscanf(io, "%d", &tmp) == 1) {
						if (posInStatiAccettazione != 0) {
							tmpStati = (int *)realloc(tmpStati, (posInStatiAccettazione + 1) * sizeof(int));
						}

						tmpStati[posInStatiAccettazione] = tmp;
						posInStatiAccettazione++;
					}
					*statiAccettazione = tmpStati;
					dimArrayStatiAccett = posInStatiAccettazione;

					if (fscanf(io, "%s", in)) {
						if (strcmp(in, "max") == 0) {
							if (fscanf(io, "%ld", maxC)) {
								if (fscanf(io, "%s", in)) {
									if (strcmp(in, "run") == 0) {
										free(in);
										in = (char *)malloc(500000);

										while (fscanf(io, "%s", in) != EOF) {
											resStr = (char *)malloc(strlen(in) + 3);
											strcpy(resStr, in);

											lastInput->str = resStr;
											lastInput->len = (int)strlen(resStr);
											lastInput->next = (stringheInput *)malloc(sizeof(stringheInput));
											ptrPenultimoElem = lastInput;
											lastInput = lastInput->next;
											lastInput->next = NULL;
										}
										free(in);
										free(lastInput);
										((stringheInput *)ptrPenultimoElem)->next = NULL;
									}
								}
							}
						}
					}
				}
			}
		}
	}

	return statoMax;
}

/*Pulizia delle transizioni*/
transizioni **createArrayTransizioni(transizioni *lista, int statiAccettazione[], int statoMax) {
	transizioni **arr, *saveLista, *tmp;
	char *esisteNodo;//dichiaro come char per limitare lo spazio, mi serve per capire se un nodo esiste così che poi io possa eliminare le transizioni che van in un nodo inesistente

	arr = (transizioni **)malloc((statoMax + 1) * sizeof(transizioni *));
	esisteNodo = (char *)malloc(statoMax + 1);

	for (int i = 0; i < statoMax + 1; i++) {
		arr[i] = NULL;
		esisteNodo[i] = '0';
	}

	{//setto l'array dei char che mi dice se uno stato di arrivo esiste
		saveLista = lista;
		while (saveLista != NULL) {
			esisteNodo[saveLista->statoStart] = '1';
			saveLista = saveLista->next;
		}

		for (int i = 0; i < dimArrayStatiAccett; i++)
			esisteNodo[statiAccettazione[i]] = '1';
	}

	//Metto le transizioni nell'array
	while (lista != NULL) {
		saveLista = lista->next;

		if (esisteNodo[lista->statoEnd] == '1') {//aggiungo l'elemento sse questo va in uno stato da cui posso ripartire
			if (arr[lista->statoStart] == NULL) {
				arr[lista->statoStart] = lista;
				arr[lista->statoStart]->next = NULL;
			}
			else {
				tmp = arr[lista->statoStart];
				while (tmp->next != NULL) {
					tmp = tmp->next;
				}
				tmp->next = lista;
				tmp->next->next = NULL;
			}
		}

		lista = saveLista;
	}
	free(esisteNodo);

	//controllo se vi sono due stati uguali e nel caso ne mantengo solo uno
	transizioni *prec = NULL, *tmp2 = NULL;
	saveLista = NULL;
	tmp = NULL;
	for (int i = 0; i < statoMax + 1; i++) {//per ogni stato di partenza
		for (saveLista = arr[i]; saveLista != NULL; saveLista = saveLista->next) {//per ogni stato
			for (tmp = saveLista->next, prec = saveLista; tmp != NULL; tmp = tmp->next, prec = prec->next) {//per tutti gli stati successivi allo stato
				if ((saveLista->leggo == tmp->leggo) && (saveLista->scrivo == tmp->scrivo) && (saveLista->statoEnd == tmp->statoEnd) && (saveLista->testina == tmp->testina)/*&&(start già uguali)*/) {//entro se sono la stessa transizione
																																																	   //quella prec la collego alla mia succ ed elimino la mia
					prec->next = tmp->next;
					free(tmp);
				}
			}
		}
	}

	//Se uno stato è pozzo, gli metto come valore di testina CHAR_POZZI così da poterlo interrompere dopo
	tmp = NULL;
	for (int i = 0; i < statoMax + 1; i++) {
		tmp = arr[i];
		while (tmp != NULL) {
			if (tmp->statoStart == tmp->statoEnd) {
				if ((tmp->leggo == tmp->scrivo) && (tmp->testina == 'S'))
					tmp->testina = CHAR_POZZI;
				else if (tmp->leggo == '_') {
					if (tmp->testina == 'L')
						tmp->speciale = CHAR_SEMPRE_SX;
					else
						tmp->speciale = CHAR_SEMPRE_DX;
				}
			}
			tmp = tmp->next;;
		}
	}

	return arr;
}

char executeTM(transizioni **arrayTransizioni, int statiAccettazione[], statoMacchina *stati, unsigned long maxC) {
	statoMacchina *statiRaggiunti, *nextList, *lastInList, *checkCicli, *nextStato;//giroPrec serve per controllare che non mi sia bloccato in un giro 
	int checkGiro;//checkGiro serve per dire se devo confrontare il giro con giroPrecedente
	unsigned long i;
	int numeroStatoGiroAttuale;

	foundPozzo = 0;
	foundSameStatus = 0;
	foundInfiniteMov = 0;
	checkGiro = 0;
	checkCicli = NULL;

	//per il numero massimo di iterazioni
	for (i = 0; i < maxC; i++) {

		//Se nel ciclo for prima, ho settato checkGiro>0, vuol dire che devo confrontare gli stati attuali con quelli che ho salvato addietro
		//confronto checkCicli con gli stati in lista, se uno stato è anche in checkCicli, lo tolgo, potrei dover settare un flag per mettere dunque U come out
		if (checkGiro > 0 && stati != NULL) {
			checkGiro--;

			//mi muovo a partire dagli stati che ho già fatto girare, per ogniuno di essi, vedo se son stanno per essere ricomputati, nel caso lo elimino
			//se prec==null, devo cambiare il val iniziale di stati
			for (statoMacchina *tmpC = checkCicli; tmpC != NULL; tmpC = tmpC->next) {
				for (statoMacchina *tmpS = stati, *precS = NULL; tmpS != NULL; /*mi devo muovere stando attento alle free*/) {
					if ((tmpS->numeroStato == tmpC->numeroStato) && (tmpS->pos == tmpC->pos) && (strcmp(tmpS->str, tmpC->str) == 0)) {//se ho già computato precedentemente quello stato, lo elimino
						if (precS != NULL) {
							precS->next = tmpS->next;
							free(tmpS->str);
							free(tmpS);
							//mi muovo bene
							tmpS = precS->next;
						}
						else {
							stati = tmpS->next;
							free(tmpS->str);
							free(tmpS);
							tmpS = stati;
						}
						//precS = precS;
					}
					else {
						precS = tmpS;
						tmpS = tmpS->next;
					}
				}
			}
			if (stati == NULL)
				foundSameStatus = 1;

			if (stati == NULL || checkGiro == 0) {
				statoMacchina *tmpC;
				while (checkCicli != NULL) {
					tmpC = checkCicli->next;
					free(checkCicli->str);
					free(checkCicli);
					checkCicli = tmpC;
				}
			}
		}
		else {
			//se son nella situazione di controllo, mi salvo gli stati così che al prossimo giro possa confrontarli con quelli nuovi che "escono" e il contatore di
			//"quante volte" devo fare questo controllo
			if ((i%CONTROLLO_LISTA_UGUALE == 0) && i != 0) {
				//checkCicli = stati; non posso più farlo perchè adesso li riuso, devo clonarli
				statoMacchina *tmpStati = stati, *tmpCheck = NULL;
				checkCicli = NULL;
				while (tmpStati != NULL) {
					if (checkCicli == NULL) {
						tmpCheck = (statoMacchina *)malloc(sizeof(statoMacchina));
						checkCicli = tmpCheck;
					}
					else {
						tmpCheck->next = (statoMacchina *)malloc(sizeof(statoMacchina));
						tmpCheck = tmpCheck->next;
					}

					tmpCheck->len = tmpStati->len;
					tmpCheck->numeroStato = tmpStati->numeroStato;
					tmpCheck->pos = tmpStati->pos;
					tmpCheck->str = (char *)malloc(tmpCheck->len + 1);
					memcpy(tmpCheck->str, tmpStati->str, tmpCheck->len + 1);
					tmpCheck->next = NULL;

					tmpStati = tmpStati->next;
				}

				checkGiro = SET_CHECK_GIRO;
			}
		}

		statiRaggiunti = NULL;
		nextList = NULL;
		lastInList = NULL;
		//Vado in tutti i posti per ogni stato
		while (stati != NULL) {
			nextStato = stati->next;
			numeroStatoGiroAttuale = stati->numeroStato;//salvo qui perchè poi perdo il valore

			statiRaggiunti = executeMovements(arrayTransizioni, stati);//mi ritorna i posti dove sono arrivato

			if (statiRaggiunti == NULL) {//se non son andato da alcuna parte controllo se è lo stato finale e nel caso mi fermo
				for (int j = 0; j < dimArrayStatiAccett; j++)
					if (statiAccettazione[j] == numeroStatoGiroAttuale)
						return '1';
			}
			else {//se tmp non è NULL
				//Accodo alla lista degli stati per il prossimo check, quelli nuovi e poi sposto il puntatore alla fine
				if (nextList != NULL) {
					lastInList->next = statiRaggiunti;
				}
				else {
					nextList = statiRaggiunti;
					lastInList = nextList;
				}
				if (lastInList != NULL)
					while (lastInList->next != NULL)
						lastInList = lastInList->next;
			}

			stati = nextStato;
		}

		//Se non ho finito la lista dei nuovi stati, vado avanti, altrimenti non ha funzionato
		if (nextList != NULL)
			stati = nextList;
		else if (foundPozzo == 0 && foundSameStatus == 0 && foundInfiniteMov == 0)//check here, se ho ad esempio eliminato uno stato che al successivo terminava ed era l'unico, devo tornare 0, non U
			return '0';
		else
			return 'U';

		//controllo se ci sono più stati uguali nella prossima passata, nel caso ne tengo uno solo
		if (stati->next != NULL) {
			for (statoMacchina *tmpC = stati; tmpC!=NULL && tmpC->next != NULL; tmpC = tmpC->next) {
				for (statoMacchina *tmpS = tmpC->next, *precS = tmpC; tmpS != NULL; /*mi devo muovere stando attento alle free*/) {
					if ((tmpS->numeroStato == tmpC->numeroStato) && (tmpS->pos == tmpC->pos) && (strcmp(tmpS->str, tmpC->str) == 0)) {//se ho trovato uno stato identico al mio loked
						precS->next = tmpS->next;
						free(tmpS->str);
						free(tmpS);

						//mi muovo bene
						tmpS = precS->next;
					}
					else {
						precS = tmpS;
						tmpS = tmpS->next;
					}
				}
			}
		}

	}

	//svuoto qui la memoria che non ho usato ora che arrivo al maxC
	while (stati != NULL) {
		nextStato = stati->next;
		free(stati->str);
		free(stati);
		stati = nextStato;
	}

	return 'U';
}

//Gli dico dove si trova e dove può andare, mi ritorna tutti i posti dove va, se non si può muovere torna NULL
//ricevo anche gli stati di accettazione che se mi muovo verso uno di quelli ritorno diretto NULL così che executeTM controlli (nuovamente) e dia 1 nel caso di stato OK
statoMacchina* executeMovements(transizioni **arrayTransizioni, statoMacchina *statoPartenza) {
	statoMacchina *nuoviStati = NULL, *ultimoStato = NULL;
	transizioni *listaTransizioni, *tmpTrans;
	char charLetto = (statoPartenza->str)[statoPartenza->pos];

	//variabili di ottimizzazione
	int len;
	char *copiaStr, *strDiPart;//uso copiaStr per non dover accedere sempre al valore della str referenziato nella lista, strDiPart punta a quella di partenza
	int posStr; //come per copiaStr
	int posPerNuovoChar; //dopo aver spostato i nastri devo andare a scrivere il char
	char dirTestina; //usati nel mov testina
	int riusaStr; //viene settata a 1 se ho più di uno stato a cui andrò da quello attuale, se rimane a 0, non ne ho e posso riusare l'oggetto
	//int finitiMovimenti = 0;

	listaTransizioni = arrayTransizioni[statoPartenza->numeroStato];//la lista delle transizioni che può fare è quella legata allo stato di partenza

	while (listaTransizioni != NULL) { //scorro la lista degli spostamenti e se vedo uno che richiede sul nastro ciò che ho, mi ci muovo verso
		if (listaTransizioni->leggo == charLetto) { //Se entro, sto "andando" in quello stato
			if (listaTransizioni->testina != CHAR_POZZI)  //controllo che non sia uno stato pozzo, nel caso non entro
			{
				//copio nastro			
				len = statoPartenza->len;
				strDiPart = statoPartenza->str;
				posStr = statoPartenza->pos;
				posPerNuovoChar = posStr;
				dirTestina = listaTransizioni->testina;
				copiaStr = NULL; //così finiti i mov controllo, se son ancora a NULL devo inizializzarla, altrimenti ho già fatto

				//Se da uno stato parte una sola transizione, invece di riallocare memoria, aggiorno questo stato, riuso questo nodo
				riusaStr = 0;
				if (listaTransizioni->next != NULL) {
					for (tmpTrans = listaTransizioni->next; riusaStr==0 && tmpTrans != NULL; tmpTrans = tmpTrans->next) //MODIFICARE SE ORDINO LA LISTA CON UN'OTTIMIZZAZIONE
						if (tmpTrans->leggo == charLetto)
							riusaStr++;
				}
				
				//0 <--> devo riusarla ma prima controllo, se mi manda in uno stato finale non devo fare nulla ma tornare NULL così poi executeTM controlla e vede che è finale
				if (riusaStr == 0) {

					{//movimento
						if (dirTestina == 'R') {
							if (strDiPart[posStr + 1] == '\0') { //NON CAMBIARE CON if (posStr == len){ PERCHÈ NON VA!!!
								if (listaTransizioni->speciale == CHAR_SEMPRE_DX) 
									strDiPart = -1;//sto per entrare in un looooop
								else {
									strDiPart = (char *)realloc(strDiPart, len + 2);
									strDiPart[posStr + 1] = '_';
									strDiPart[posStr + 2] = '\0';
									len++;
								}
							}
							posStr++;
						}
						else if (dirTestina == 'L') {
							if (posStr != 0)
								posStr--;
							else {
								if (listaTransizioni->speciale == CHAR_SEMPRE_SX)
									strDiPart = -1;//sto per entrare in un looooop
								else {
									copiaStr = (char *)malloc(len + 2);
									strcpy(copiaStr + 1, strDiPart);
									free(strDiPart);
									strDiPart = copiaStr;
									strDiPart[0] = '_';
									posPerNuovoChar++;//è l'unico caso in cui cambio questo valore e che diventa 1
									len++;
								}
							}
						}
					}//fine mov

					if (strDiPart != -1) {
						statoPartenza->len = len;
						strDiPart[posPerNuovoChar] = listaTransizioni->scrivo;
						statoPartenza->str = strDiPart; //potrei aver ridimensionato la str
						statoPartenza->pos = posStr;
						statoPartenza->numeroStato = listaTransizioni->statoEnd;

						//return statoPartenza; ->devo fare una sorta di: ultimoStato = statoPartenza, copiato dall'altro caso
						if (ultimoStato != NULL) {
							ultimoStato->next = statoPartenza;
							ultimoStato = ultimoStato->next;//FALSO: non mi serve spostare l'ultimo stato, non lo tocco più -> serve non so perchè
						}
						else {
							nuoviStati = statoPartenza;
							ultimoStato = nuoviStati;//per l'if-ternario finale
						}
						ultimoStato->next = NULL;
					}
					else {
						foundInfiniteMov = 1;
						//essendo qui significa che era l'unico e che non va da nessunaparte quindi lo elimino
						free(statoPartenza->str);
						free(statoPartenza);
						//return NULL; non è detto che non abbia prima girato tanti altri casi ok quindi vado al return finale
					}
					goto fineCiclo; //I Don't like GOTOs but this improve performance by 5 to 7 percent
				}
				//else se quindi non è l'ultimo branch
				else {
					//Gestisco lo spostamento della testina
					{
						if (dirTestina == 'R') {
							if (strDiPart[posStr + 1] == '\0') { //NON CAMBIARE CON if (posStr == len){ PERCHÈ NON VA!!!
								if (listaTransizioni->speciale == CHAR_SEMPRE_DX)
									strDiPart = -1;//sto per entrare in un looooop
								else {
									copiaStr = (char *)malloc(len + 2);
									memcpy(copiaStr, strDiPart, len);
									copiaStr[posStr + 1] = '_';
									copiaStr[posStr + 2] = '\0';
									len++;
								}
							}
							posStr++;
						}
						else if (dirTestina == 'L') {
							if (posStr != 0)
								posStr--;
							else {
								if (listaTransizioni->speciale == CHAR_SEMPRE_SX)
									strDiPart = -1;//sto per entrare in un looooop
								else {
									copiaStr = (char *)malloc(len + 2);
									memcpy(copiaStr + 1, strDiPart, len + 1); //devo copiare la stringa lasciando il primo spazio vuoto
									copiaStr[0] = '_';
									posPerNuovoChar++;//è l'unico caso in cui cambio questo valore e che diventa 1
									len++;
								}
							}
						}
					}
					//fine movimento testina

					//se non ho già scritto la nuova stringa nello spostamento della testina, la creo
					if (copiaStr == NULL) {
						copiaStr = (char *)malloc(len + 1);
						memcpy(copiaStr, strDiPart, len + 1);
					}

					//creo il nuovo stato da accodare
					if (copiaStr != -1) {
						//inserisco nel punto dove ci trovavamo prima dello spostamento della testina, il nuovo char
						copiaStr[posPerNuovoChar] = listaTransizioni->scrivo;

						if (ultimoStato != NULL) {
							ultimoStato->next = (statoMacchina *)malloc(sizeof(statoMacchina));
							ultimoStato = ultimoStato->next;
						}
						else {
							nuoviStati = (statoMacchina *)malloc(sizeof(statoMacchina));
							ultimoStato = nuoviStati;
						}
						ultimoStato->next = NULL;

						//salvo il nuovo stato nel nodo creato
						ultimoStato->str = copiaStr;
						ultimoStato->pos = posStr;
						ultimoStato->len = len;
						ultimoStato->numeroStato = listaTransizioni->statoEnd;
					}
					else 
						foundInfiniteMov = 1;
				}
				
			}
			else
				foundPozzo = 1;
		}
		listaTransizioni = listaTransizioni->next;
	}

	free(statoPartenza->str);
	free(statoPartenza);

fineCiclo:
	return ultimoStato != NULL ? nuoviStati : NULL;
}