
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

#include "message.c"


	// grid for probabilities
	int* probaGrid; 

	// grid with anwers
	int* answerGrid;

	int gridSize;


	// Socket client
	int sclient;

	// login
	char* login;



/* Fonction permettant de jouer un coup sur la grille dans le serveur*/
int jouerCoup(int x, int y)
{
	// verifier x et y pas en dehors de la grille


	/* Demande de coup au serveur*/
	printf("--------------PROP--------------\n");
	struct msg propBuf;
	propBuf.code = PROP; // code PROP = 1
	propBuf.data[0] = x; // position x htonl() ?
	propBuf.data[1] = y; // position y
	strcpy(propBuf.login, login);

	int octet_sent = write(sclient, &propBuf, sizeof(propBuf));
	printf("PROP SND TO SERVER : \n octets : %d\n code : %d (exp:1)\n coordX : %d\n coordY : %d\n login : %s\n\n", 
		octet_sent, propBuf.code, propBuf.data[0], propBuf.data[1], propBuf.login);


	/* Récupération de la réponse du serveur*/
	struct msg server_answer;
	int octet_read = read(sclient, &server_answer, sizeof(server_answer));

	printf("PROP RVC FROM SERVER : \n octets : %d\n code : %d(exp:4)\n renards : %d\n",
		octet_read, server_answer.code, ntohl(server_answer.data[0]));

	if(server_answer.code == ERREUR){
		printf("Prop failed!\n");
		exit(1);
	}

	printf("------------END PROP------------\n\n");
	return ntohl(server_answer.data[0]);
}

int endGame()
{
	printf("--------------FIN--------------\n");

	struct msg endBuf;
	endBuf.code = FIN; 
	int octet_sent = write(sclient, &endBuf, sizeof(endBuf));
	printf("FIN SND TO SERVER : \n octets : %d\n code : %d (exp:2)\n\n", 
		octet_sent, endBuf.code);


	struct msg server_answer;
	int octet_read = read(sclient, &server_answer, sizeof(server_answer));
	printf("FIN RVC FROM SERVER : \n octets : %d\n code : %d (exp:6)\n",
		octet_read, server_answer.code);


	printf("-------------END FIN-------------\n");

}

int startPlay()
{

	/* Probabilities grid init*/
	probaGrid = malloc((gridSize * gridSize) * sizeof(int));

	/* Probabilities grid init*/
	answerGrid = malloc((gridSize) * sizeof(int));

	printf("Renards en 1 1 : %d\n", jouerCoup(1,1));


	/* FORCE BRUT 
	for(int i = 0; i < gridSize; i++)
	{
		printf("%d\n", i);
		for(int j = 0; j<gridSize; j++)
		{
			printf("Renards en %d %d : %d\n", i,j, jouerCoup(i,j));

		}
	}
	*/

	endGame();
}


int main(int argc, char const *argv[])
{

	/* Test de la ligne de commande*/
	if (argc != 7) {
		printf("Usage: %s <IP-address> <port number> <grid_size> <nb_de_renard> <user> <seed>\n", argv[0]);
		exit(1);
	}

	gridSize = atoi(argv[3]);
	int nbRenards = atoi(argv[4]);
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


	struct addrinfo *res;
	struct addrinfo criteria;

	/* Criteria*/
	memset(&criteria,0, sizeof(struct addrinfo));
	criteria.ai_family = AF_UNSPEC; /* Allow IPv4 or IPv6 */
	criteria.ai_socktype = SOCK_STREAM; //Socket addresses of any type
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


	/* Initialisation de la grille*/
	printf("--------------INIT--------------\n");
	struct msg initBuf;
	initBuf.code = INIT; // code INIT = 0
	initBuf.data[0] = gridSize; // taille de la grille
	initBuf.data[1] = nbRenards; // nombre de renards
	initBuf.data[2] = seed; // seed de la grille
	strcpy(initBuf.login, login);

	/* Envoi au serveur l'initialisation*/
	int octet_sent = write(sclient, &initBuf, sizeof(initBuf));
	printf("INIT SND TO SERVER : \n octets : %d\n code : %d (exp:0)\n gridSize : %d\n nbRenards : %d\n seed : %d\n login : %s\n\n",
	 octet_sent, initBuf.code, initBuf.data[0], initBuf.data[1],  initBuf.data[2], initBuf.login);

	/* Réponse du serveur sur l'initialisation*/
	struct msg server_answer;
	int octet_read = read(sclient, &server_answer, sizeof(server_answer));
	if(server_answer.code == ERREUR){
		printf("Init failed!\n");
		exit(1);
	}
	printf("INIT RCV FROM SERVER : \n octets : %d\n code : %d (exp:3)\n", 
		octet_read, server_answer.code);
	printf("-------------END INIT-------------\n\n");



	// Starting algorithms to play the game
	//startPlay();




	/* close socket and connection*/
	shçutdown(sclient, SHUT_RDWR);
	close(sclient);

	return EXIT_SUCCESS;
}





void updateProbaGrid()
{

}

