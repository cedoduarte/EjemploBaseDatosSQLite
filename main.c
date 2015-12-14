#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sqlite3.h"

#define CREATE_TABLE_PERSONA \
    "CREATE TABLE IF NOT EXISTS persona " \
    "(id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL" \
    ", nombre TEXT NOT NULL" \
    ", edad INTEGER NOT NULL)"

typedef struct Persona Persona;

struct Persona
{
    char nombre[500];
    int edad;
    int id;
    Persona *siguiente;
};

void muestraLista(const Persona *lista)
{
    if (lista == NULL) {
        printf("[Lista vacia]\n");
        return;
    }
    Persona *ix = lista;
    while (ix != NULL) {
        printf("ID: %d;Nombre: %s;Edad: %d\n", ix->id, ix->nombre, ix->edad);
        ix = ix->siguiente;
    }
}

Persona *personaNueva(const Persona *persona)
{
    Persona *p = (Persona *) malloc(sizeof(Persona));
    strcpy(p->nombre, persona->nombre);
    p->edad = persona->edad;
    p->id = persona->id;
    p->siguiente = NULL;
    return p;
}

void agregaPersona(Persona **lista, const Persona *persona)
{
    if (lista == NULL) {
        return;
    }
    if (*lista == NULL) {
        *lista = personaNueva(persona);
        return;
    }
    Persona *ix = *lista;
    while (ix->siguiente != NULL) {
        ix = ix->siguiente;
    }
    ix->siguiente = personaNueva(persona);
}

int gestionaError(sqlite3 *db)
{
    fprintf(stderr, "Error: %s\n", sqlite3_errmsg(db));
    return sqlite3_errcode(db);
}

void introduceDatos(const Persona *lista, sqlite3 *db)
{
    if (lista == NULL) {
        return;
    }
    Persona *ix = lista;
    char sql[100];
    while (ix != NULL) {
        sprintf(sql, "INSERT INTO persona (nombre, edad) VALUES ('%s', %d)",
                ix->nombre, ix->edad);
        if (sqlite3_exec(db, sql, NULL, NULL, NULL) != SQLITE_OK) {
            gestionaError(db);
            return;
        }
        ix = ix->siguiente;
    }
}

int callback(void *ptr, int numeroDeColumnas, char **valoresCeldas,
             char **nombresDeColumnas)
{
    (void) ptr;
    int ix;
    for (ix = 0; ix < numeroDeColumnas; ++ix) {
        printf("%s: %s\n", nombresDeColumnas[ix], valoresCeldas[ix]);
    }
    printf("\n");
    return 0;
}

void leeBaseDatos(sqlite3 *db)
{
    sqlite3_exec(db, "SELECT * FROM persona", callback, NULL, NULL);
}

int main(void)
{
    FILE *file = NULL;
    Persona persona;
    Persona *lista = NULL;
    sqlite3 *db = NULL;
    const char *filenameDatabase = "C:/users/cedo/desktop/MiBaseDatos.db";
    const char *filenameLista = "C:/users/cedo/desktop/datos.txt";

    persona.id = -1;

    // abre base de datos
    if (sqlite3_open(filenameDatabase, &db) != SQLITE_OK) {
        return gestionaError(db);
    }

    // configura base de datos
    if (sqlite3_exec(db, CREATE_TABLE_PERSONA, NULL, NULL, NULL)
            != SQLITE_OK) {
        return gestionaError(db);
    }

    // lee lista
    file = fopen(filenameLista, "r");
    if (file != NULL) {
        while (!feof(file)) {
            fscanf(file, "%s%d", persona.nombre, &persona.edad);
            agregaPersona(&lista, &persona);
        }
        fclose(file);
    } else {
        fprintf(stderr, "Error de archivo\n");
    }

    muestraLista(lista);

    // introduce datos a la base de datos
    introduceDatos(lista, db);

    // lectura de base de datos
    leeBaseDatos(db);

    // cerramos sqlite
    sqlite3_close(db);

    return 0;
}
