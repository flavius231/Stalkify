#include "mongoose.h"
#include <stdio.h>
#include<mysql.h>



// Variabila globala pentru conexiune
MYSQL *con = NULL;

void init_db() {
    con = mysql_init(NULL);
    if (con == NULL) {
        fprintf(stderr, "mysql_init() failed\n");
        return;
    }

    // ATENTIE: Inlocuieste "parola_ta" cu parola setata la instalarea MySQL
    if (mysql_real_connect(con, "localhost", "root", "sUPERMAN@1245", "stalkify_db", 3306, NULL, 0) == NULL) {
        fprintf(stderr, "Eroare la conectare: %s\n", mysql_error(con));
        mysql_close(con);
        con = NULL;
    } else {
        printf("Conexiune la MySQL reusita!\n");
    }
}

static void trateaza_cererea(struct mg_connection *c, int ev, void *ev_data) {
    if (ev == MG_EV_HTTP_MSG) {
        // Servim direct folderul public. Cand cineva acceseaza http://localhost:8080/
        // Mongoose va cauta automat index.html in folderul public.
        struct mg_http_serve_opts opts = {0};
        opts.root_dir = "./public";
        mg_http_serve_dir(c, ev_data, &opts);
    }
}


typedef struct utilizator{
char username[50];
char nume_real[100];
int urmaritori,urmariti;
char parola[50];
char email[100];
int id;
}utilizator;

// Această funcție returnează o structură populată
utilizator mapeaza_mysql_la_utilizator(MYSQL_ROW rand) {
    utilizator u;
    
    // Inițializăm totul cu zero/null pentru siguranță
    memset(&u, 0, sizeof(utilizator));

    // Convertim și copiem datele
    u.id = atoi(rand[0]);
    strncpy(u.username, rand[1], 49);
    strncpy(u.nume_real, rand[2], 99);
    strncpy(u.email, rand[3], 99);
    // Urmaritori/Urmariti pot veni din rand[4], rand[5] dacă îi selectezi în SQL
    
    return u;
}


void cauta_utilizator_dupa_username(char *username_cautat) {
    char query[1024];
    sprintf(query, "SELECT id, username, nume_real, email FROM utilizatori WHERE username = '%s'", username_cautat);

    if (mysql_query(con, query) == 0) {
        MYSQL_RES *rezultat = mysql_store_result(con);
        if (rezultat != NULL) {
            MYSQL_ROW rand = mysql_fetch_row(rezultat);
            if (rand != NULL) {
                // APELĂM FUNCȚIA SEPARATĂ AICI
                utilizator u = mapeaza_mysql_la_utilizator(rand);
                
                printf("Utilizator gasit: %s cu ID: %d\n", u.username, u.id);
            }
            mysql_free_result(rezultat);
        }
    }
}

int main(void) {
  int main(void) {
    printf("DEBUG: Programul a pornit!\n"); // Adaugă asta!
    fflush(stdout); // Asta obligă consola să afișeze mesajul imediat

    init_db();
    
    printf("DEBUG: Conexiunea la DB a fost incercata.\n");
    fflush(stdout);

    // ... restul codului
  cauta_utilizator_dupa_username("havius");
    struct mg_mgr mgr;
    mg_mgr_init(&mgr);
    mg_http_listen(&mgr, "http://0.0.0.0:8080", trateaza_cererea, NULL);
    
    printf("Serverul C a pornit! Acceseaza http://localhost:8080/\n");
    
    for (;;) {
        mg_mgr_poll(&mgr, 1000);
    }
    mg_mgr_free(&mgr);
    return 0;
}

