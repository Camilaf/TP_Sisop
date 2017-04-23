TP1: Memoria virtual en JOS
===========================

page2pa
-------
Definición de la función:

static inline physaddr_t
page2pa(struct PageInfo *pp)
{
	return (pp - pages) << PGSHIFT;
}

Donde:
pp apunta a la información de la página (el struct)
pages guarda el estado de cada página física
PGSHIFT = 12 (definición en mmu.h)

Lo que hace la función es, a partir de un puntero a PageInfo, obtener la dirección física de la página a la que se refiere.
Al puntero del cual queremos hallar la página física (pp) le resta el inicio del arreglo de physical pages.
Obtenemos el número de la página física y shiftea para obtener la dirección de la página física. Hacer el shift de 12 bits es equivalente a multiplicar el número de página por 4096, que corresponde al tamaño de cada página en bytes.

boot_alloc_pos
--------------
a. Como al inicializar nextfree lo que hace la función es un ROUNDUP entre end y PGSIZE, en donde end apunta al final del segmento .bss del kernel, podemos calcular la dirección en la cual termina ese segmento mediante el comando:

$ readelf -S kernel

Encabezados de Sección:
  [Nr] Nombre            Tipo            Direc    Desp   Tam    ES Opt En Inf Al
  [ 0]                   NULL            00000000 000000 000000 00      0   0  0
  [ 1] .text             PROGBITS        f0100000 001000 001b51 00  AX  0   0 16
  [ 2] .rodata           PROGBITS        f0101b60 002b60 0007f0 00   A  0   0 32
  [ 3] .stab             PROGBITS        f0102350 003350 004291 0c   A  4   0  4
  [ 4] .stabstr          STRTAB          f01065e1 0075e1 001cb5 00   A  0   0  1
  [ 5] .data             PROGBITS        f0109000 00a000 00a300 00  WA  0   0 4096
  [ 6] .bss              NOBITS          f0113300 014300 000650 00  WA  0   0 32
  [ 7] .comment          PROGBITS        00000000 014300 000034 01  MS  0   0  1
  [ 8] .shstrtab         STRTAB          00000000 014fe0 00004c 00      0   0  1
  [ 9] .symtab           SYMTAB          00000000 014334 000840 10     10  62  4
  [10] .strtab           STRTAB          00000000 014b74 00046c 00      0   0  1


La sección .bss comienza en 0xf0113300 y tiene un tamaño de 0x000650. La suma entre estos valores corresponde a la dirección en donde termina el segmento: 0xf0113950. 
Como el valor obtenido no es múltiplo de 4096, lo que haremos es encontrar el menor valor posible, que sea mayor a 0xf0113950 y múltiplo de 4096. El valor buscado es 0xf0114000, que será la primera direción de memoria que devolverá boot_alloc.

b)

page_alloc
----------

...


