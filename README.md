# fat-fs
Implementazione del File System FAT [Progetto SO 2021-2022].

## Struttura del progetto
Il progetto è composto da tre moduli principali:
- la libreria `libfat` contenuta nella cartella `libfat/`.  
  Il suo header pubblico è `fat.h`, mentre l'header `internals.h` è riservato alle funzioni interne alla libreria.  
  Viene compilata in un file oggetto `libfat.a`.

  Contiene tutte le funzioni richieste più le funzioni `file_move` e `file_copy` per spostare e copiare file.

- il programma di test `tester`, interamente contenuto in `tester.c` implementa dei test automatici per tutte le funzioni della libreria libfat. Può essere eseguito come:
    + `./tester` o `./tester all` per testare tutte le funzioni.
    + `./tester <nome_funzione>` per testare le singole funzioni della libfat, e vedere anche in forma grafica i risultati dei test (oltre che al punteggio).

- il programma `fat_man` (FAT Manager), interamente contenuto in `fat_man.c` che permette di inizializzare e di utilizzare diversi comandi in stile Linux per usare le funzioni implementate nella libreria libfat.

> Il programma `fat_test` (interamente contenuto in `fat_test.c`) è stato usato durante la programmazione della libreria per testare le sue funzioni. Non fa parte della consegna.


## Come compilare
Assicurarsi di usare un sistema Linux e di avere installati `gcc` e `make`.

Per compilare questo progetto basta eseguire `make` e verrà compilata sia la `libfat` che i programmi che dipendono da essa.

## Uso del manager
Per inizializzare il file system usare `./fat_man -i` (verrà fornita una guida su come passare gli altri parametri).

Eseguendo `./fat_man -s <file>` una volta inizializzato il file system nel file `file` sarà possibile eseguire i seguenti comandi:
- `cd <dir>`: apre la cartella `dir` (se esiste). Se `dir` non viene passato si intende la cartella root `/`.
- `ls [-l|-a|--all|-la|-al] <dir>`: stampa il contenuto della cartella `dir`.
- `mkdir <dir>`: crea la cartella `dir`.
- `rmdir <dir>`: elimina la cartella `dir` e tutto il suo contenuto ricorsivamente.
- `touch <file>`: crea il file vuoto `file`.
- `rm <file>`: elimina il file `file`.
- `cat <file>`: legge il contenuto del file `file` e lo stampa a schermo.
- `write <file>`: scrive l'input letto da tastiera nel file `file` (se non esiste il file può essere creato).
- `append <file>`: scrive l'input letto da tastiera alla fine del file `file` (se non esiste il file può essere creato).
- `ec <file_ext> <file>`: copia il contenuto del file esterno `file_ext` nel file `file`.
- `repeat <file> <char> <n>`: appende alla fine del file `file` il carattere `char` per `n` volte.
- `mv <file|dir> <file|dir>`: sposta (o rinomina) il file o la cartella (il file o la cartella di destinazione non devono esistere già con lo stesso nome).
- `cp <file|dir> <file|dir>`: copia (o duplica) il file o la cartella (il file o la cartella di destinazione non devono esistere già con lo stesso nome).
- `size <dir>`: stampa la dimensione della cartella `dir` in Bytes realmente occupati e il numero di blocchi (e relativi Bytes di peso) effettivamente occupati su disco. Se il parametro `dir` non è presente si intende la cartella corrente.
- `free`: stampa il numero di blocchi e numero di Bytes liberi e totali all'interno del file system.

> Nota: con `dir` e `file` si intendono i percorsi verso la cartella o il file, siano essi assoluti oppure relativi.

## Autori
- Claudio Cicimurri (@Cicim)
- Lorenzo Clazzer (@Claziero)
