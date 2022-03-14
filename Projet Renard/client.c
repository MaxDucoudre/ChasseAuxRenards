
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>

#include <sys/types.h>

#include <netdb.h>
#include <string.h>  
#include <assert.h>  

#include <sys/stat.h>
#include <fcntl.h>
#include <arpa/inet.h>

#include <signal.h> 

#include "message.c"








	/* Pour gérer write & les read au serveur*/
	int sclient;
	char* login;

	/* On formations sur la partie en cours */
	int gridSize;
	int nb_coup;
	int renard_vivants;

	/* Calculs pour le bot qui vas jouer */
	int* probaGrid; 
	int* answerGrid;


/* Ferme la connection proprement*/
void endProg()
{
	shutdown(sclient, SHUT_RDWR);
	close(sclient);
	printf("\nFin du programme! \n");
	exit(0);
}

/* Permet d'envoyer un message au serveur via la structure msg*/
int writeInServer(struct msg message)
{
	int octet_sent = 0;
	octet_sent += write(sclient, &message.code,    sizeof(message.code));
	octet_sent += write(sclient, &message.data[0], sizeof(message.data[0]));
	octet_sent += write(sclient, &message.data[1], sizeof(message.data[1]));
	octet_sent += write(sclient, &message.data[1], sizeof(message.data[2]));
	octet_sent += write(sclient, &message.login,   sizeof(message.login));
	message.octets = octet_sent;

	if(octet_sent != 25)
	{
		printf("Nombre d'octets envoyés mauvais!");
		//endGame();
	}
	return octet_sent;

}
/* Permet de lire un message envoyé par le serveur (à condition d'en avoir écrit un avant) */
struct msg readInServer()
{
	int octet_read = 0;
	struct msg message;

	octet_read += read(sclient, &message.code,    sizeof(message.code));
	octet_read += read(sclient, &message.data[0], sizeof(message.data[0]));
	octet_read += read(sclient, &message.data[1], sizeof(message.data[1]));
	octet_read += read(sclient, &message.data[1], sizeof(message.data[2]));
	octet_read += read(sclient, &message.login,   sizeof(message.login));
	message.octets = octet_read;

	return message;
}

/* Encois un message de fin au serveur */
int endGame()
{
	if(DEBUG) printf("\n--------------FIN--------------\n");
	struct msg endBuf;
	endBuf.code = FIN; 
	int octet_sent = write(sclient, &endBuf, sizeof(endBuf));
	if(DEBUG) printf("FIN SND TO SERVER : \n octets : %d\n code : %d (exp:2)\n\n", 
		octet_sent, endBuf.code);


	struct msg server_answer = readInServer();
	if(DEBUG) printf("FIN RVC FROM SERVER : \n octets : %d\n code : %d (exp:6)\n",
		server_answer.octets, server_answer.code);


	if(DEBUG) printf("-------------END FIN-------------\n");

	endProg();
}

/* Fonction permettant de jouer un coup sur la grille dans le serveur*/
int jouerCoup(int x, int y)
{
	// verifier x et y ne sont pas en dehors de la grille
	if(x > gridSize || y > gridSize || x < 0 || y < 0)
	{
		printf("Coords out of grid!\n");
		endGame();
	}

	/* Demande de coup au serveur*/
	if(DEBUG) printf("--------------PROP--------------\n");
	struct msg propBuf;
	propBuf.code = PROP; // code PROP = 1
	propBuf.data[0] = htonl(x); // position x 
	propBuf.data[1] = htonl(y); // position y
	propBuf.data[2] = 0; // position y

	strcpy(propBuf.login, login);

	int octet_sent = writeInServer(propBuf);
	if(DEBUG) printf("PROP SND TO SERVER : \n octets : %d\n code : %d (exp:1)\n coordX : %d\n coordY : %d\n login : %s\n\n", 
		octet_sent, propBuf.code, ntohl(propBuf.data[0]),  ntohl(propBuf.data[1]), propBuf.login);


	/* Récupération de la réponse du serveur*/
	struct msg server_answer = readInServer();
	//int octet_read = read(sclient, &server_answer, sizeof(server_answer));

	if(DEBUG) printf("PROP RVC FROM SERVER : \n octets : %d\n code : %d(exp:4 or 5)\n renards : %d\n",
		server_answer.octets, server_answer.code, ntohl(server_answer.data[0]));


	if(server_answer.code == ERREUR){
		printf("Prop failed!\n");
		endGame();
	}

	if(server_answer.code == GAGNE)
	{
		printf("\nYou Win!\n\n");

		if(DEBUG) printf("------------END PROP------------\n\n");
		endGame();
	}

	nb_coup++;
	if(DEBUG) printf("------------END PROP------------\n\n");


	/* Si un renard a été touché, on renvois quand même le nombre de renards reliés au point x y*/
	if(server_answer.data[0] == -1)
	{
		renard_vivants--;
		return jouerCoup(x,y);
	}


	return ntohl(server_answer.data[0]);
}

/* Début de la partie, le bot vas jouer*/
int startPlay()
{
	printf("\n renards vivants : %d", renard_vivants);
	printf("\n coups joués: %d", nb_coup);

	/* Probabilities grid init*/
	probaGrid = malloc((gridSize * gridSize) * sizeof(int));

	/* Probabilities grid init*/
	answerGrid = malloc((gridSize) * sizeof(int));

	printf("\n renards en 1 1 : %d\n", jouerCoup(1,1));

	printf("\n renards en 1 2 : %d\n", jouerCoup(1,2));

	printf("\n renards en 1 3 : %d\n", jouerCoup(1,3));


	printf("\n renards en 1 4 : %d\n", jouerCoup(1,4));

	printf("\n renards en 1 5 : %d\n", jouerCoup(1,5));



	// FORCE BRUT 
	/*
	for(int i = 0; i < gridSize; i++)
	{
		for(int j = 0; j<gridSize; j++)
		{
			jouerCoup(i,j);
		}
	}
	*/
}


int main(int argc, char const *argv[])
{
	/* Test de la ligne de commande*/
	if (argc != 7) {
		printf("Usage: %s <IP-address> <port number> <grid_size> <nb_de_renard> <user> <seed>\n", argv[0]);
		exit(1);
	}

	/* Conversion des arguments en entier */
	gridSize = atoi(argv[3]);
	renard_vivants = atoi(argv[4]);
	int seed = atoi(argv[6]);

	/* Récupération du login*/
	login = malloc(sizeof(char) * LOGIN_MAX_SIZE);
	for(int i = 0; i< LOGIN_MAX_SIZE; i++)
	{
		if(i < sizeof(argv[5]) / sizeof(argv[5][0]))
		{
			login[i] = argv[5][i];
		} else
		{
			login[i] = '\0';
		}
	}

	/* Connection au serveur */
	struct addrinfo *res;
	struct addrinfo criteria;

	/* Critère pour se connecter */
	memset(&criteria,0, sizeof(struct addrinfo));
	criteria.ai_family = AF_UNSPEC; /* Allow IPv4 or IPv6 */
	criteria.ai_socktype = SOCK_STREAM; // adresse type connecté
	criteria.ai_protocol = 0; /* Any protocol */

	/* test variable for connection*/
	int testConnect = -1;

	/* addrinfo */	
	getaddrinfo(argv[1], argv[2], &criteria ,&res);
	while(res != NULL){
	/* socket */
		sclient = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
		testConnect = connect(sclient, res->ai_addr, res->ai_addrlen);

		if(testConnect >= 0)
		{
			break;
		} 
		res = res->ai_next; // itération dans la liste chaîné
	}

	if(testConnect < 0)
	{
		printf("Impossible to connect!");
		exit(1);
	} 
		assert(testConnect >= 0);

	assert(sclient >= 0);

	signal(SIGINT, endProg); // ferme propropement la connection et le socket en cas de SIGINT

	/* Initialisation de la grille*/
	if(DEBUG) printf("--------------INIT--------------\n");
	struct msg initBuf;
	initBuf.code = INIT; // code INIT = 0
	initBuf.data[0] = htonl(gridSize); // taille de la grille
	initBuf.data[1] = htonl(renard_vivants); // nombre de renards
	initBuf.data[2] = htonl(seed); // seed de la grille
	strcpy(initBuf.login, login);

	/* Envoi au serveur l'initialisation*/
	int octet_sent = writeInServer(initBuf);
	if(DEBUG) printf("INIT SND TO SERVER : \n octets envoyés : %d\n code : %d (exp:0)\n gridSize : %d\n nbRenards : %d\n seed : %d\n login : %s\n\n",
	 octet_sent, initBuf.code, ntohl(initBuf.data[0]), ntohl(initBuf.data[1]),  ntohl(initBuf.data[2]), initBuf.login);

	/* Réponse du serveur sur l'initialisation*/
	struct msg server_answer = readInServer();
	if(server_answer.code == ERREUR){
		printf("Init failed!\n");
		endGame();
	}

	if(DEBUG) printf("INIT RCV FROM SERVER : \n octets reçus : %d\n code : %d (exp:3)\n", 
		server_answer.octets, server_answer.code);
	if(DEBUG) printf("-------------END INIT-------------\n\n");

	// Début du déroulé de la partie
	nb_coup = 0;
	startPlay();
}