#ifndef _MESSAGE_H
#define _MESSAGE_H

/* code requete */

#define INIT 0
#define PROP 1
#define FIN  2

/* code reponse */

#define INIT_ACK 3
#define PROP_ACK 4
#define GAGNE    5
#define FIN_ACK  6
#define ERREUR   7

#define LOGIN_MAX_SIZE 12

#define DEBUG 0

struct msg {
    char code;
    int data[3];
    char login[LOGIN_MAX_SIZE];
    int octets;
};

#endif