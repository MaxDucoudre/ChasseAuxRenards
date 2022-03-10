
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
	struct msg propBuf;
	propBuf.code = htonl(PROP); // code PROP = 1
	propBuf.data[0] = htonl(x);
	propBuf.data[1] = htonl(y);
	propBuf.data[2] = 0;
	//propBuf.login = login;
	strcpy(propBuf.login, login);


	int octet_sent = write(sclient, &propBuf, sizeof(propBuf));


	/* Récupération de la réponse*/
	struct msg server_answer;
	int octet_read = read(sclient, &server_answer, sizeof(server_answer));

	if(ntohs(server_answer.code) == ERREUR){
		printf("Prop failed!\n");
		//exit(1);
	}


	return ntohl(server_answer.data[0]);


}


int startPlay()
{

	/* Probabilities grid init*/
	probaGrid = malloc((gridSize * gridSize) * sizeof(int));

	/* Probabilities grid init*/
	answerGrid = malloc((gridSize) * sizeof(int));

	printf("%d\n", gridSize);

	for(int i = 0; i < gridSize; i++)
	{
		printf("%d\n", i);
		for(int j = 0; j<gridSize; j++)
		{
			printf("Renards en %d %d : %d\n", i,j, jouerCoup(i,j));

		}
	}
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

	assert(testConnect >= 0); 
	assert(sclient >= 0);


	/* Initialisation de la grille*/
	struct msg initBuf;

	initBuf.code = INIT; // code INIT = 0
	initBuf.data[0] = gridSize;
	initBuf.data[1] = nbRenards;
	initBuf.data[2] = seed;
	strcpy(initBuf.login, login);

	/* Envoi au serveur l'initialisation*/
	int octet_sent = write(sclient, &initBuf, sizeof(initBuf));

	printf(
		"INIT : \n
		octets : %d\n
		code : %d
		gridSize : %d\n
		nbRenards : %d\n
		seed : %d\n
		login : %s\n
		");


	/* Réponse du serveur sur l'initialisation*/
	struct msg server_answer;
	int octet_read = read(sclient, &server_answer, sizeof(server_answer));
	if(server_answer.code == ERREUR){
		printf("Init failed!\n");
		exit(1);
	}
	printf("Init rcv : %d (%d)\n", octet_read, server_answer.code);



	// Starting algorithms to play the game
	startPlay();

	/* close socket and connection*/
	shutdown(sclient, SHUT_RDWR);
	close(sclient);

	return EXIT_SUCCESS;
}



void updateProbaGrid()
{

}

