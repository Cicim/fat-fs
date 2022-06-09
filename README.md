# fat-fs
Implementazione del File System FAT [Progetto SO 2021-2022].

## Struttura del progetto
Il progetto è strutturato come segue:
- La cartella principale contiene il file "driver" `fat_man.c` ed il Makefile associato all'intero progetto.
- La cartella `/libfat` contiene le implementazioni delle funzioni del FS. In particolare:
    - `internals.h` contiene le strutture e le definizioni delle funzioni non accessibili all'esterno della libreria.
    - `fat.h` contiene le strutture e le definizioni delle funzionalità del FS, ognuna implementata nel file associato.

## Autori
- Claudio Cicimurri (@Cicim)
- Lorenzo Clazzer (@Claziero)
