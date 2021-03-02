# TarArchiver

## Scop

Aceasta aplicatie implementata in **C** are ca scop simularea unui tool de arhivare tar.
Ea primeste diferite comenzi asa cum sunt specificate in [enunt](https://acs.curs.pub.ro/2019/pluginfile.php/50565/mod_folder/content/0/Programare_2019___Tema_3.pdf?forcedownload=1) si executa operatiile de
**create**, **list** si **extract**. Nu este folosit niciun algoritm de compresie, doar
se respecta standardul de arhiva tar. Interesant este faptul ca daca folosesc utilitarul *tar*
disponibil pe *linux*, arhivei create cu acest program ii poate fi extras continutul si la fel,
unei arhive create cu utilitarul *tar*, ii poate fi listata continutul folosind implementarea
data.

## Detalii despre implementare

### Cerinta 0 - archiver

Functia **check** returneaza valoarea 0 daca comanda introdusa este gresita si 1 daca
este corecta. In variabila *cod* retin un numar care va fi specific comenzii respective
* -1 comanda gresita;
* 0 exit;
* 1 create;
* 2 list;
* 3 extract.


### Cerinta 1 - create

Am folosit functiile **completestr** (completeaza stringurile din header cu '\0' la final),
**permission**, **timedec**, **sum**, **display_header**, **path** (creeaza calea pentru
fiecare fisier care trebuie deschis dintr-un anumit director), pe care le-am inclus in
functia **create**.
	
Deschid arhiva pe care vreau sa o creez si incep sa citesc din fisierul *files.txt*, linie
cu linie. Citesc la inceput un caracter (care va fi '-' cat timp nu se termina fisierul).
Folosesc **strtok** ca sa separ informatiile din *files.txt*.

Cand termin de separat informatiile, deschid fisierul *usermap.txt* si caut user-ul pe care
l-am obtinut din *files.txt* pentru a calcula **uid** si **gid**. Apoi introduc in fisierul
binar (arhiva) datele din header, completez cu 0-uri astfel incat sa aiba dimensiunea de *512*,
folosind functia **display_header**. Deschid fisierul respectiv, (pentru care am completat header-ul)
si citesc datele din el in blocuri de *512*, pe care le bag apoi in arhiva mea.

Pentru **mode** folosesc functia **permision** care verifica caracter cu caracter stringul dat
ca parametru si calculeaza permisiunile in octal.

Pentru **mtime**, folosesc functia **timedec** care calculeaza timpul in decimal. Ma folosesc
de structura **struct tm** si completez variabilele specifice ei (*time.tm_year* etc) cu datele
din cele doua stringuri date ca parametru (*day 2019-11-12; clock 17:55:15.882089310*). La final,
**mktime** returneaza timpul in secunde.

Pentru **chksum** am folosit functia **sum**, care aduna codurile ascii ale fiecarei variabila din
header, la care se adauga si 8 valori ale caracterului ' ', caci la inceput **chksum** este format
din spatii.

Restul informatiilor din header le calculez in functia create, pe care o apelez in **main**, daca
codul comenzii introduse este 1.


### Cerinta 2 - list

Deschid fisierul binar (arhiva) si citesc pana cand nu mai sunt fisiere in arhiva (gasesc zona de
'\0'), adica pana cand dau de un fisier care nu are nume (strlen(nume_fisier)==0). In variabilele
*namef* si *size* retin numele si size-ul fiecarui fisier. Citesc numele fiecarui fisier si il
afisez in terminal, apoi ma mut cu *24* pozitii (asa este standardizat) in arhiva ca sa ajung la *size*.
Dupa ce citesc *size-ul* ma mut *376* pozitii ca sa ajung la continutul fisierului.

Calculez cate blocuri de *512* caractere sunt in continutul fiecarui fisier cu ajutorul lui *size*,
pe care il am in octal si il transform in decimal folosind functia **decimal**, dupa care ma mut in
arhiva cu *512 * numar_blocuri* ca sa ajung la header-ul fisierului urmator din arhiva.
	

### Cerinta 3 - extract

Deschid fisierul binar (arhiva) si creez fisierul extras. Folosesc aproximativ aceleasi lucruri ca in
list, doar ca citesc si pana cand dau de numele fisierului pe care vreau sa il extrag (ok=1, am gasit
fisierul, ok ramane 0, fisierul nu exista).

Citesc la fel numele fisierului si size-ul ca sa pot sa ma mut in arhiva cu fseek. Daca ok=0, nu am
gasit fisierul, merg mai departe, daca ok=1, deschid fisierul extras si il completez cu datele din arhiva.

