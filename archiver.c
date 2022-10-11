#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#define NMAX 512
#define head temporar.cap.header

union record {
    char charptr[512];
    struct header {
        char name[100];
        char mode[8];
        char uid[8];
        char gid[8];
        char size[12];
        char mtime[12];
        char chksum[8];
        char typeflag;
        char linkname[100];
        char magic[8];
        char uname[32];
        char gname[32];
        char devmajor[8];
        char devminor[8];
    } header;
};

typedef struct files {
    union record cap;
}file;

// Functia returneaza permisiunile in octal
int getPerm(char perm[NMAX]) {
    int nr = 0, u = 0, g = 0, o = 0;

    if (perm[1] == 'r') {
        u += 4;
    }
    if (perm[2] == 'w') {
        u += 2;
    }
    if (perm[3] == 'x') {
        u += 1;
    }
    if (perm[4] == 'r') {
        g += 4;
    }
    if (perm[5] == 'w') {
        g += 2;
    }
    if (perm[6] == 'x') {
        g += 1;
    }
    if (perm[7] == 'r') {
        o += 4;
    }
    if (perm[8] == 'w') {
        o += 2;
    }
    if (perm[9] == 'x') {
        o += 1;
    }

    nr = u * 100 + g * 10 + o;
    return nr;
}

// Pune in stringul mode numarul
void makeMode(char *q, char mode[8]) {
    memset(mode, '0', 8);
    int nr = getPerm(q);
    mode[7] = '\0';
    mode[6] = nr % 10 + '0';
    mode[5] = nr / 10 % 10 + '0';
    mode[4] = nr / 100 + '0';
}

// Calculeaza secundele de la 1 Ianuarie 1970 00:00 cu ajutorul functiei mktime
void makeTime(char *q, char time[12]) {
    struct tm inform;

    q = strtok(NULL, " ");
    inform.tm_year = (q[0] - '0') * 1000 + (q[1] - '0') * 100;
    inform.tm_year += (q[2] - '0') * 10 + (q[3] - '0') - 1900;
    inform.tm_mon = (q[5] - '0') * 10 + (q[6] - '0') - 1;
    inform.tm_mday = (q[8] - '0') * 10 + (q[9] - '0');

    q = strtok(NULL, " ");
    inform.tm_hour = (q[0] - '0') * 10 + (q[1] - '0');
    inform.tm_min = (q[3] - '0') * 10 + (q[4] - '0');
    inform.tm_sec = (q[6] - '0') * 10 + (q[7] - '0');
    inform.tm_isdst = -1;

    int timp = mktime(&inform);
    sprintf(time, "%011o", timp);
    time[11] = '\0';
}

// Extrage userid si groupid
void uidgid(char uname[32], char uid[8], char gid[8]) {
    FILE *lista1;
    lista1 = fopen("usermap.txt", "rt");

    int ok = 1;
    char rand[NMAX];

    // Parcurge fisierul usermap.txt
    while (fgets(rand, NMAX, lista1) && ok == 1) {
        char un[NMAX];
        strcpy(un, rand);
        char *p = strtok(un, ":");

        // Verifica numele
        if (strcmp(p, uname) == 0) {
            ok = 0;
            p = strtok(NULL, ":");

            // Extrage userid
            p = strtok(NULL, ":");
            sprintf(uid, "%07o", atoi(p));
            uid[7] = '\0';

            // Extrage groupid
            p = strtok(NULL, ":");
            sprintf(gid, "%07o", atoi(p));
            gid[7] = '\0';
        }
    }

    fclose(lista1);
}

void create(char narhiva[NMAX], char ndirector[NMAX]) {
    char rand[NMAX], nfisier[2 * NMAX], continut[NMAX];
    int i;
    long marime = 0, nrblocuri;

    // Deschidem fisierul files.txt unde gasim informatiile
    FILE *lista;
    lista = fopen("files.txt", "rt");
    if (lista == NULL) {
        return;
    }

    // Deschidem arhiva
    FILE *archive;
    archive = fopen(narhiva, "wb");
    if (archive == NULL) {
        return;
    }

    // Parcurgem files.txt si formam arhiva ceruta
    while (fgets(rand, NMAX, lista) != NULL) {
        file temporar;
        for (int i = 0 ; i < 512 ; i++) {
            temporar.cap.charptr[i] = '\0';
        }

        // Umplem campurile din header cu informatiile necesare
        char *p = strtok(rand, " ");

        makeMode(p, temporar.cap.header.mode);

        p = strtok(NULL, " ");

        p = strtok(NULL, " ");
        memset(temporar.cap.header.uname, '\0', 32);
        strcpy(temporar.cap.header.uname, p);
        p = strtok(NULL, " ");
        memset(temporar.cap.header.gname, '\0', 32);
        strcpy(temporar.cap.header.gname, p);

        p = strtok(NULL, " ");
        marime = atoi(p);
        sprintf(temporar.cap.header.size, "%011o", atoi(p));
        temporar.cap.header.size[11] = '\0';

        makeTime(p, temporar.cap.header.mtime);

        p = strtok(NULL, " ");

        p = strtok(NULL, " ");
        memset(temporar.cap.header.name, '\0', 100);
        strcpy(temporar.cap.header.name, p);
        temporar.cap.header.name[strlen(p) - 1] = '\0';

        strcpy(nfisier, ndirector);
        strcat(nfisier, temporar.cap.header.name);

        memset(temporar.cap.header.linkname, '\0', 100);
        strcpy(temporar.cap.header.linkname, p);
        temporar.cap.header.linkname[strlen(p) - 1] = '\0';

        temporar.cap.header.typeflag = '0';

        memset(temporar.cap.header.devmajor, '0', 8);
        temporar.cap.header.devmajor[7] = '\0';
        memset(temporar.cap.header.devminor, '0', 8);
        temporar.cap.header.devminor[7] = '\0';
        strcpy(temporar.cap.header.magic, "GNUtar ");

        uidgid(head.uname, head.uid, head.gid);

        memset(temporar.cap.header.chksum, ' ', 8);
        int sum = 0;
        for (int i = 0 ; i < 512 ; i++) {
            sum += temporar.cap.charptr[i];
        }
        temporar.cap.header.chksum[7] = ' ';
        sprintf(temporar.cap.header.chksum, "%06o", sum);

        // Scriem headerul in arhiva
        fwrite(&(temporar.cap.header), sizeof(union record), 1, archive);

        FILE *fisier;
        fisier = fopen(nfisier, "rb");

        if (fisier == NULL) {
            return;
        }

        // Citim din fisier si scriem in arhiva
        nrblocuri = marime / 512;
        for (i = 0 ; i < nrblocuri ; i++) {
            fread(continut, sizeof(char), 512, fisier);
            fwrite(continut, sizeof(char), 512, archive);
        }

        int x = fread(continut, sizeof(char), NMAX, fisier);
        fwrite(continut, sizeof(char), x, archive);

        char c = '\0';
        for (i = x ; i < 512 ; i++) {
            fwrite(&c, sizeof(char), 1, archive);
        }

        fclose(fisier);
    }

    fclose(lista);
    fclose(archive);
}

// Transforma un numar din baza 8 in baza 10
int octZec(int nrOct) {
    int nrZec = 0;
    int k = 0;
    while (nrOct != 0) {
        int put = 1, nr = k;
        while (nr > 0) {
            put *= 8;
            nr--;
        }
        nrZec += (nrOct % 10) * put;
        nrOct /= 10;
        k++;
    }
    k = 1;
    return nrZec;
}

void list(char narhiva[NMAX]) {
    // Verificam daca este arhiva
    if (strstr(narhiva, ".tar") != NULL) {
        FILE *fisier;
        fisier = fopen(narhiva, "rb");

        // Verificam existenta arhivei
        if (fisier == NULL) {
            printf("> File not found!\n");
            return;
        } else {
            int size;
            int k, sizefile = 0;
            char continut[NMAX], aux[12];

            // Obtinem marimea arhivei
            fseek(fisier, 0, SEEK_END);
            size = ftell(fisier);
            fseek(fisier, 0, SEEK_SET);

            // Parcurgem arhiva
            while (ftell(fisier) < size) {
                // Extragem headerul si il parcurgem
                fread(continut, sizeof(char), NMAX, fisier);

                // Extragem numele
                char nume[100];
                strcpy(nume, continut);
                printf("> %s\n", nume);

                // Extragem marimea fisierului
                k = 0;
                for (int i = 124 ; i < 136 ; i++) {
                    aux[k] = continut[i];
                    k++;
                }
                aux[k] = '\0';
                sizefile = atoi(aux);
                sizefile = octZec(sizefile);

                // Verificam daca marimea este multiplu de 512
                if (sizefile % 512 != 0 || sizefile == 0) {
                    sizefile = (sizefile / 512 + 1) * 512;
                }

                // Mutam pozitia in fisier cu noua marime pozitii
                fseek(fisier, sizefile, SEEK_CUR);
            }
        }

        fclose(fisier);
     } else {
        printf("> Wrong command!\n");
     }
}

void extract(char narhiva[NMAX], char ndirector[NMAX]) {
    FILE *fisier;
    fisier = fopen(narhiva, "rb");

    int size = ftell(fisier);
    int k, sizefile = 0;
    char continut[NMAX], aux[12];
    int ok = 0;

    // Obtinem marimea arhivei
    fseek(fisier, 0, SEEK_END);
    size = ftell(fisier);
    fseek(fisier, 0, SEEK_SET);

    // Parcurgem arhiva si verificam daca s-a gasit fisierul
    while (ftell(fisier) < size && ok == 0) {
        // Extragem headerul si il parcurgem
        fread(continut, sizeof(char), NMAX, fisier);

        // Extragem numele
        char nume[100];
        strcpy(nume, continut);
        k = 0;
        for (int i = 124 ; i < 136 ; i++) {
            aux[k] = continut[i];
            k++;
        }

        // Extragem marimea fisierului
        aux[k] = '\0';
        sizefile = atoi(aux);
        sizefile = octZec(sizefile);

        // Calculam cu cat trebuie mutata pozitia in arhiva
        int jumpsize = sizefile;
        if (sizefile % 512 != 0  || sizefile == 0) {
            jumpsize = (sizefile / 512 + 1) * 512;
        }

        // Verificam daca s-a gasit fisierul
        if (strcmp(continut, ndirector) == 0) {
            ok = 1;
            char fisierfinal[NMAX];

            // Formam fisierul
            strcpy(fisierfinal, "extracted_");
            strcat(fisierfinal, ndirector);
            FILE *fisier1;
            fisier1 = fopen(fisierfinal, "wb");

            // Formam fisierul
            if (sizefile != 0) {
                char inform[NMAX];
                int count = sizefile / 512;
                int x;
                while (count > 0) {
                    x = fread(inform, sizeof(char), NMAX, fisier);
                    fwrite(inform, sizeof(char), x, fisier1);
                    count--;
                }
                if (sizefile % 512 != 0) {
                    jumpsize -= sizefile;
                    x = fread(inform, sizeof(char), NMAX - jumpsize, fisier);
                    fwrite(inform, sizeof(char), x, fisier1);
                }
             }

            fclose(fisier1);
            printf("> File extracted!\n");
        } else {
            fseek(fisier, jumpsize, SEEK_CUR);
        }
    }

    fclose(fisier);
    if (ok == 0) {
        printf("> File not found!\n");
    }
}

int main() {
    char comanda[NMAX], *p;

    // Citim comanda
    while (fgets(comanda, NMAX, stdin)) {
        char aux[NMAX];
        int nrcuv = 0;
        comanda[strlen(comanda) - 1] = '\0';

        // Numaram cuvintele
        strcpy(aux, comanda);
        p = strtok(aux, " ");
        while (p) {
            nrcuv++;
            p = strtok(NULL, " ");
        }

        // Verificam numarul de cuvinte
        char narhiva[NMAX], ndirector[NMAX];
        if (nrcuv > 0) {
            p = strtok(comanda, " ");

            // Verificam primul cuvant
            if (strcmp(p, "exit") == 0) {
                return 0;
            } else if (strcmp(p, "create") == 0 && nrcuv == 3) {
                p = strtok(NULL, " ");
                strcpy(narhiva, p);

                // Verificam daca este arhiva
                if (strstr(narhiva, ".tar") != NULL) {
                    p = strtok(NULL, " ");
                    strcpy(ndirector, p);
                    int lendir = strlen(ndirector);

                    // Verificam daca este director
                    if (ndirector[lendir - 1] == '/') {
                        // Apelam functia create
                        create(narhiva, ndirector);
                        printf("> Done!\n");
                    } else {
                        printf("> Wrong command!\n");
                    }

                } else {
                    printf("> Wrong command!\n");
                }

            } else if (strcmp(p, "list") == 0 && nrcuv == 2) {
                p = strtok(NULL, " ");
                strcpy(narhiva, p);

                // Apelam functia list
                list(narhiva);
            } else if (strcmp(p, "extract") == 0 && nrcuv == 3) {
                p = strtok(NULL, " ");
                strcpy(ndirector, p);
                p = strtok(NULL, " ");
                strcpy(narhiva, p);

                // Verificam daca este arhiva
                if (strstr(narhiva, ".tar") != NULL) {
                    // Apelam functia list
                    extract(narhiva, ndirector);
                } else {
                    printf("> Wrong command!\n");
                }

            } else {
                printf("> Wrong command!\n");
            }

        } else {
            printf("> Wrong command!\n");
        }
    }
    return 0;
}
