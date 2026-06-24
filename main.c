#include "mongoose.h"
#include <stdio.h>
#include <string.h> // Adaugat pentru memset, strncpy
#include <stdlib.h> // Adaugat pentru atoi
#include <mysql.h>

// Variabila globala pentru conexiune
MYSQL *con = NULL;

void init_db() {
    con = mysql_init(NULL);
    if (con == NULL) {
        fprintf(stderr, "Eroare: mysql_init() failed\n");
        return;
    }

    // Încercăm să setăm clientul să nu ceară SSL
    // client_flag = 0 înseamnă că nu activăm opțiuni speciale, 
    // dar vom forța conexiunea în mysql_real_connect
    
    if (mysql_real_connect(con, "localhost", "root", "sUPERMAN@1245", "stalkify_db", 3306, NULL, 0) == NULL) {
        
        // Dacă a eșuat din cauza SSL, putem încerca să oprim SSL-ul explicit dacă librăria ne permite
        // Dar mai întâi, să vedem dacă eroarea persistă.
        fprintf(stderr, "Eroare la conectare MySQL: %s\n", mysql_error(con));
        mysql_close(con);
        con = NULL;
    } else {
        printf("DEBUG: Conexiune la MySQL reusita!\n");
    }
}

typedef struct utilizator {
    char username[50];
    char nume_real[100];
    int urmaritori, urmariti;
    char parola[50];
    char email[100];
    int id;
} utilizator;

utilizator mapeaza_mysql_la_utilizator(MYSQL_ROW rand) {
    utilizator u;
    memset(&u, 0, sizeof(utilizator)); // Curatam structura

    // Daca randul e valid, convertim datele
    if (rand != NULL) {
        u.id = atoi(rand[0]);
        strncpy(u.username, rand[1] ? rand[1] : "", 49);
        strncpy(u.nume_real, rand[2] ? rand[2] : "", 99);
        strncpy(u.email, rand[3] ? rand[3] : "", 99);
    }
    return u;
}

void cauta_utilizator_dupa_username(char *username_cautat) {
    // PROTECTIE CRITICA: Daca nu avem conexiune, iesim din functie
    if (con == NULL) {
        fprintf(stderr, "Eroare: Baza de date nu este conectata. Cautarea pentru '%s' a fost anulata.\n", username_cautat);
        return; 
    }

    char query[1024];
    sprintf(query, "SELECT id, username, nume_real, email FROM utilizatori WHERE username = '%s'", username_cautat);

    if (mysql_query(con, query) == 0) {
        MYSQL_RES *rezultat = mysql_store_result(con);
        if (rezultat != NULL) {
            MYSQL_ROW rand = mysql_fetch_row(rezultat);
            if (rand != NULL) {
                utilizator u = mapeaza_mysql_la_utilizator(rand);
                printf("SUCCES! Utilizator gasit: %s cu ID: %d, Email: %s\n", u.username, u.id, u.email);
            } else {
                printf("Utilizatorul '%s' nu exista in baza de date.\n", username_cautat);
            }
            mysql_free_result(rezultat);
        }
    } else {
        fprintf(stderr, "Eroare la executia interogarii: %s\n", mysql_error(con));
    }
}

static void trateaza_cererea(struct mg_connection *c, int ev, void *ev_data) {
    if (ev == MG_EV_HTTP_MSG) {
        struct mg_http_message *hm = (struct mg_http_message *) ev_data;

        // Folosim ".buf" deoarece versiunile noi de Mongoose au redenumit asa variabila de text
        if (hm->uri.len == 7 && strncmp(hm->uri.buf, "/search", 7) == 0) {
            char username[100] = "";
            
            // Extragem variabila username din URL (?username=havius)
            mg_http_get_var(&hm->query, "username", username, sizeof(username));
            printf("DEBUG: Am primit cerere de cautare pentru: %s\n", username);

            // Returnam browserului un mesaj de confirmare
            char raspuns[500];
            snprintf(raspuns, sizeof(raspuns), "Serverul C a prins comanda! Ai cautat userul: %s. Urmeaza conectarea la MySQL.", username);
            
            mg_http_reply(c, 200, "Content-Type: text/plain\r\n", "%s", raspuns);
        } 
        // Orice alta cerere (ex: cand intri pe pagina principala)
        else {
            struct mg_http_serve_opts opts = {0};
            opts.root_dir = "D:\\Stalkify\\public"; 
            mg_http_serve_dir(c, hm, &opts);
        }
    }
}
int main(void) {
    printf("DEBUG: Programul a pornit!\n"); 
    fflush(stdout); 

    init_db();
    
    // Testam cautarea in baza de date
    cauta_utilizator_dupa_username("havius");

    // Pornim serverul
    struct mg_mgr mgr;
    mg_mgr_init(&mgr);
    mg_http_listen(&mgr, "http://0.0.0.0:8080", trateaza_cererea, NULL);
    
    printf("DEBUG: Serverul C a pornit! Acceseaza http://localhost:8080/\n");
    fflush(stdout);
    
    for (;;) {
        mg_mgr_poll(&mgr, 1000);
    }
    
    mg_mgr_free(&mgr);
    
    // Inchidem conexiunea curat la iesire
    if (con != NULL) {
        mysql_close(con);
    }
    
    return 0;
}
