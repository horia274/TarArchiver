// Ignat Andrei-Horia 314CA

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

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

int check(char *com, int *cod) {
// verific daca comanda este corect introdusa si daca da,
// o indentific cu valoarea variabilei cod
  int nr;
  // numarul de argumente ale comenzii
  *cod = -1; nr = 0;
  char *comcopy;

  comcopy = strdup(com);
  char *p = strtok(comcopy, " ");
  while (p != NULL) {
    if (strcmp(p, "exit") == 0)
      *cod = 0;
    if (strcmp(p, "create") == 0)
      *cod = 1;
    if (strcmp(p, "list") == 0)
      *cod = 2;
    if (strcmp(p, "extract") == 0)
      *cod = 3;
    nr++;
    p = strtok(NULL, " ");
  }
  free(comcopy);
  if (*cod == -1 || (*cod == 0 && nr != 1) || (*cod == 1 && nr != 3)
  || (*cod == 2 && nr != 2) || (*cod == 3 && nr != 3))
    return 0;
  return 1;
}

void completestr(char *s, char *t, int dim) {
  // pun '\0'-uri la stringurile din header
  memset(s, 0, dim*sizeof(char));
  strcpy(s, t);
}

int decimal(int n) {
  // transform un octal in decimal (pentru extract)
  int n10 = 0, p = 1;

  while (n != 0) {
    n10 += (n % 10) * p;
    p *= 8;
    n /= 10;
  }
  return n10;
}

int permission(char *perm) {
  // calculez permisiunile in octal
  int permu = 0, permg = 0, permo = 0, perms;

  if (perm[0] == 'r')
    permu += 4;
  if (perm[1] == 'w')
    permu += 2;
  if (perm[2] == 'x')
    permu++;
  if (perm[3] == 'r')
    permg += 4;
  if (perm[4] == 'w')
    permg += 2;
  if (perm[5] == 'x')
    permg++;
  if (perm[6] == 'r')
    permo += 4;
  if (perm[7] == 'w')
    permo += 2;
  if (perm[8] == 'x')
    permo++;
  perms = permu*100 + permg*10 + permo;
  return perms;
}

long timedec(char *day, char *clock) {
  // calculez timpul in decimal
  struct tm time = {0};
  char *q;

  q = strtok(day, " :-");
  time.tm_year = atoi(q)-1900;
  q = strtok(NULL, " :-");
  time.tm_mon = atoi(q)-1;
  q = strtok(NULL, " :-");
  time.tm_mday = atoi(q);
  q = strtok(clock, " :-");
  time.tm_hour = atoi(q);
  q = strtok(NULL, " :-");
  time.tm_min = atoi(q);
  q = strtok(NULL, " :-");
  time.tm_sec = atoi(q);

  return (long)mktime(&time);
}

int sum(union record x) {
  // fac suma toturor datelor din header pentru chksum
  int i, s = 0;

  for (i = 0; i < 100; i++) {
    s += x.header.name[i];
    s += x.header.linkname[i];
  }

  for (i = 0; i < 32; i++) {
    s += x.header.uname[i];
    s += x.header.gname[i];
  }

  for (i = 0; i < 12; i++) {
    s += x.header.size[i];
    s += x.header.mtime[i];
  }

  for (i = 0; i < 8; i++) {
    s += x.header.mode[i];
    s += x.header.uid[i];
    s += x.header.gid[i];
    s += x.header.magic[i];
    s += x.header.devmajor[i];
    s += x.header.devminor[i];
  }

  s += x.header.typeflag;
  s += (8*32);
  
  return s;
}

void display_header(union record x, FILE *bin) {
  // scriu in arhiva pe care o creez datele din header
  char buff[168];
  memset(buff, 0, 167*sizeof(char));

  fwrite(x.header.name, sizeof(char), 100, bin);
  fwrite(x.header.mode, sizeof(char), 8, bin);
  fwrite(x.header.uid, sizeof(char), 8, bin);
  fwrite(x.header.gid, sizeof(char), 8, bin);
  fwrite(x.header.size, sizeof(char), 12, bin);
  fwrite(x.header.mtime, sizeof(char), 12, bin);
  fwrite(x.header.chksum, sizeof(char), 8, bin);
  fwrite(&x.header.typeflag, sizeof(char), 1, bin);
  fwrite(x.header.linkname, sizeof(char), 100, bin);
  fwrite(x.header.magic, sizeof(char), 8, bin);
  fwrite(x.header.uname, sizeof(char), 32, bin);
  fwrite(x.header.gname, sizeof(char), 32, bin);
  fwrite(x.header.devmajor, sizeof(char), 8, bin);
  fwrite(x.header.devminor, sizeof(char), 8, bin);
  fwrite(buff, sizeof(char), 167, bin);
}

char* path(char *nume_director, char *nume_fisier) {
  // creez calea pentru un fisier pe care
  // trebuie sa il deschid dintr-un anumit director
  char *s;

  s = (char*)malloc((strlen(nume_fisier)+strlen(nume_director)+3)*sizeof(char));
  strcpy(s, "./");
  strcat(s, nume_director);
  strcat(s, nume_fisier);
  
  return s;
}

void create(char *name_arch, char *name_dir) {
  // creez arhiva
  FILE *bin, *f, *f1, *f2;
  union record x;
  char *path1, *path2, *pathf;

  path1 = path(name_dir, "files.txt");
  path2 = path(name_dir, "usermap.txt");

  // deschid files.txt
  f1 = fopen(path1, "r");
  // deschid arhiva
  if ((bin = fopen(name_arch, "wb")) == NULL) {
    printf("> Failed!\n");
    return;
  }

  char buff[512], line[100], aux[4];
  char *p, *perm, *user, *group, *size, *date, *clock, *namef;

  // citesc primul caracter din fiecare linie
  // pana cand ajung la capatul fisierului
  while (getc(f1) != EOF) {
    fgets(buff, 100, f1);
    buff[strlen(buff)-1] = '\0';

    // separ datele cu strtok in pointer-ul p
    p = strtok(buff, " ");

    // retin informatia din p intr-o variabila sugestiva
    // si completez header-ul
    perm = strdup(p);
    strcpy(x.header.mode, "0000");
    sprintf(aux, "%d", permission(perm));
    strcat(x.header.mode, aux);
    p = strtok(NULL, " ");
    p = strtok(NULL, " "); user = strdup(p);
    completestr(x.header.uname, user, 32);
    p = strtok(NULL, " "); group = strdup(p);
    completestr(x.header.gname, group, 32);
    p = strtok(NULL, " "); size = strdup(p);
    sprintf(x.header.size, "%011o", atoi(size));
    p = strtok(NULL, " "); date = strdup(p);
    p = strtok(NULL, " "); clock = strdup(p);
    p = strtok(NULL, " ");
    p = strtok(NULL, " "); namef = strdup(p);
    completestr(x.header.name, namef, 100);
    completestr(x.header.linkname, namef, 100);
    sprintf(x.header.mtime, "%lo", timedec(date, clock));

    // deschid usermap.txt citind la fel ca mai sus  si completez
    // fiecare linie citita cu primul caracter citit deja
    f2 = fopen(path2, "r");
    char c = getc(f2);

    while (c != EOF) {
      fgets(line, 100, f2);
      line[strlen(line)-1] = '\0';
      memset(buff, 0, strlen(buff)*sizeof(char));
      buff[0] = c;
      strcat(buff, line);

      // separ argumentele ca sa obtin uid si gid
      // pentru user-ul respectv
      p = strtok(buff, ":");
      if (strcmp(p, user) == 0) {
        p = strtok(NULL, ":");
        p = strtok(NULL, ":");
        sprintf(x.header.uid, "%07o", atoi(p));
        p = strtok(NULL, ":");
        sprintf(x.header.gid, "%07o", atoi(p));
      }
      c = getc(f2);
    }

    fclose(f2);
    x.header.typeflag = '0';
    strcpy(x.header.magic, "GNUtar ");
    memset(x.header.devmajor, 0, 8*sizeof(char));
    memset(x.header.devminor, 0, 8*sizeof(char));
    sprintf(x.header.chksum, "%06o", sum(x));
    x.header.chksum[7] = ' ';
    
    // afisez in arhiva continutul header-ului
    display_header(x, bin);
    // deschid fisierul pentru a completa arhiva cu datele din el
    pathf = path(name_dir, namef);
    f = fopen(pathf, "r");
    
    while (!feof(f)) {
      memset(buff, 0, 512*sizeof(char));
      fread(buff, sizeof(char), 512, f);
      fwrite(buff, sizeof(char), 512, bin);
    }

    fclose(f);
    free(pathf); free(perm); free(user); free(group);
    free(size); free(date); free(clock); free(namef);
  }

  // completez cu cele 512 '\0'-uri de la finalul arhivei
  memset(buff, 0, 512*sizeof(char));
  fwrite(buff, sizeof(char), 512, bin);
  printf("> Done!\n");
  fclose(f1); fclose(bin);
  free(path1); free(path2);
}

void list(char *nume_arhiva) {
  FILE *bin;

  // deschid arhiva
  if ((bin = fopen(nume_arhiva, "rb")) == NULL) {
    printf("> File not found!\n");
    return;
  }

  char namef[100], size[12];
  int blocks, sizedec, no_char = 1;

  // citesc din ea pana cand exista fisiere
  while (!feof(bin) && no_char > 0) {
    fread(namef, sizeof(char), 100, bin);
    no_char = strlen(namef);
    
    if (strlen(namef) > 0)
      printf("> %s\n", namef);
    
    // ma mut in arhiva ca sa ajung la size
    fseek(bin, 24, SEEK_CUR);
    fread(size, sizeof(char), 12, bin);
    sizedec = decimal(atoi(size));
    
    // ma mut in arhiva ca sa ajung la continut
    fseek(bin, 376, SEEK_CUR);
    
    // calculez cate blocuri de 512 caractere trebuie sa sar
    // pentru a ajunge la alt fisier
    if (sizedec % 512 == 0 && sizedec != 0)
      blocks = sizedec / 512;
    else
      blocks = sizedec / 512 + 1;
    fseek(bin, 512 * blocks, SEEK_CUR);
  }
  fclose(bin);
}

void extract(char *nume_fisier, char *nume_arhiva) {
  FILE *bin, *extbin;
  
  if ((bin = fopen(nume_arhiva, "rb")) == NULL) {
    printf("> File not found!\n");
    return;
  }

  char namef[100], size[12], buff[512], *path;
  int blocks, sizedec, i, no_char = 1, ok = 0;
  
  // daca ok=1 am gasit fisierul care trebuie extras
  while (!feof(bin) && no_char > 0 && ok == 0) {
    // ma plimb prin arhiva la fel ca la list
    fread(namef, sizeof(char), 100, bin);
    no_char = strlen(namef);
    
    if (strcmp(namef, nume_fisier) == 0)
      ok = 1;
    
    fseek(bin, 24, SEEK_CUR);
    fread(size, sizeof(char), 12, bin);
    sizedec = decimal(atoi(size));
    fseek(bin, 376, SEEK_CUR);
    
    if (sizedec % 512 == 0 && sizedec != 0)
      blocks = sizedec / 512;
    else
      blocks = sizedec / 512 + 1;

    if (ok == 0) {
      fseek(bin, 512 * blocks, SEEK_CUR);
    } else {
      // deschid fisierul extras
      path = (char*)malloc((strlen(nume_fisier) + 11) * sizeof(char));
      strcpy(path, "extracted_");
      strcat(path, nume_fisier);
      extbin = fopen(path, "wb");
      
      // citesc datele din fisier si le bag in celalalt
      for (i = 0; i < blocks-1; i++) {
        memset(buff, 0, 512*sizeof(char));
        fread(buff, sizeof(char), 512, bin);
        fwrite(buff, sizeof(char), 512, extbin);
      }

      memset(buff, 0, (sizedec % 512)*sizeof(char));
      fread(buff, sizeof(char), sizedec % 512, bin);
      fwrite(buff, sizeof(char), sizedec % 512, extbin);
      free(path);
      fclose(extbin);
    }
  }

  if (ok == 0)
    printf("> File not found!\n");
  else
    printf("> File extracted!\n");
  fclose(bin);
}

int main() {
  char com[512], *arch, *dir, *file;
  int cod;

  // citesc comenzi din terminal si in functie de
  // comanda apelez functiile respective
  fgets(com, 512, stdin);
  com[strlen(com)-1] = '\0';

  while (strcmp(com, "exit") != 0) {
    if (check(com, &cod) == 0) {
      printf("> Wrong command!\n");
    } else {
      if (cod == 1) {
        arch = strtok(com, " ");
        arch = strtok(NULL, " ");
        dir = strtok(NULL, " ");
        create(arch, dir);
      }
      if (cod == 2) {
        arch = strtok(com, " ");
        arch = strtok(NULL, " ");
        list(arch);
      }
      if (cod == 3) {
        file = strtok(com, " ");
        file = strtok(NULL, " ");
        arch = strtok(NULL, " ");
        extract(file, arch);
      }
    }
    fgets(com, 512, stdin);
    com[strlen(com)-1]='\0';
  }
  
  return 0;
}
