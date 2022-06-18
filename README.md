# fat-fs
Implementazione del File System FAT [Progetto SO 2021-2022].

## Struttura del progetto
Il progetto è composto da tre moduli principali:
- la libreria `libfat` contenuta nella cartella `libfat/`.  
  Il suo header pubblico è `fat.h`, mentre l'header `internals.h` è riservato alle funzioni interne alla libreria.  
  Viene compilata in un file oggetto `libfat.a`.

  Contiene tutte le funzioni richieste più le funzioni `file_move` e `file_copy` per spostare e copiare file.

- il programma di test `tester`, interamente contenuto in `tester.c` implementa dei test automatici per tutte le funzioni della libreria libfat. Può essere eseguito come:
    + `./tester` o `./tester all` per testare tutte le funzioni
    + `./tester <nome_funzione>` per testare le singole funzioni della libfat, e vedere anche in forma grafica i risultati dei test (oltre che al punteggio).

- il programma `fat_man` (FAT Manager), interamente contenuto in `fat_man.c` che permette di inizializzare e di utilizzare diversi comandi in stile Linux per usare le funzioni implementate nella libreria libfat.

> il programma `fat_test` (interamente contenuto in `fat_test.c`) è stato usato durante la programmazione della libreria per testare le sue funzioni. Non fa parte della consegna.


## Come compilare
Assicurarsi di usare un sistema Linux e di avere installati `gcc` e `make`.

Per compilare questo progetto basta eseguire `make` e verrà compilata sia la `libfat` che i programmi che dipendono da essa.

## Autori
- Claudio Cicimurri (@Cicim)
- Lorenzo Clazzer (@Claziero)
