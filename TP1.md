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
  [ 1] .text             PROGBITS        f0100000 001000 001f81 00  AX  0   0 16
  [ 2] .rodata           PROGBITS        f0101fa0 002fa0 0009d4 00   A  0   0 32
  [ 3] .stab             PROGBITS        f0102974 003974 0048e5 0c   A  4   0  4
  [ 4] .stabstr          STRTAB          f0107259 008259 001e54 00   A  0   0  1
  [ 5] .data             PROGBITS        f010a000 00b000 00a300 00  WA  0   0 4096
  [ 6] .bss              NOBITS          f0114300 015300 000650 00  WA  0   0 32
  [ 7] .comment          PROGBITS        00000000 015300 000034 01  MS  0   0  1
  [ 8] .shstrtab         STRTAB          00000000 0160a4 00004c 00      0   0  1
  [ 9] .symtab           SYMTAB          00000000 015334 0008c0 10     10  70  4
  [10] .strtab           STRTAB          00000000 015bf4 0004b0 00      0   0  1


La sección .bss comienza en 0xf0114300 y tiene un tamaño de 0x000650. La suma entre estos valores corresponde a la dirección en donde termina el segmento: 0xf0114950. Ésta será la dirección a la que apunta end. 
Como el valor obtenido no es múltiplo de 4096, lo que haremos es encontrar el menor valor posible, que sea mayor a 0xf0114950 y múltiplo de 4096. El valor buscado es 0xf0115000, que será la primera dirección de memoria que devolverá boot_alloc.

b. 
$ make gdbgdb -q -s obj/kern/kernel -ex 'target remote 127.0.0.1:26000' -n -x .gdbinit
Leyendo símbolos desde obj/kern/kernel...hecho.
Remote debugging using 127.0.0.1:26000
0x0000fff0 in ?? ()
(gdb) b boot_alloc
Punto de interrupción 1 at 0xf0100ae6: file kern/pmap.c, line 86.
(gdb) c
Continuando.
Se asume que la arquitectura objetivo es i386
=> 0xf0100ae6 <boot_alloc>:	mov    %eax,%edx

Breakpoint 1, boot_alloc (n=<unknown type>) at kern/pmap.c:86
86	{
(gdb) print nextfree
$1 = 0x0
(gdb) print (char *) &end
$7 = 0xf0114950 "\022"
(gdb) b 97
Punto de interrupción 2 at 0xf0100af1: file kern/pmap.c, line 97.
(gdb) c
Continuando.
=> 0xf0100af1 <boot_alloc+11>:	mov    $0xf011594f,%eax

Breakpoint 2, boot_alloc (n=<unknown type>) at kern/pmap.c:97
97			nextfree = ROUNDUP((char *) end, PGSIZE);
(gdb) next
=> 0xf0100b00 <boot_alloc+26>:	mov    0xf0114538,%eax
105		result = nextfree;
(gdb) print nextfree
$4 = 0xf0115000 ""



page_alloc
----------
Definición de page2kva en pmap.h:

static inline void*

page2kva(struct PageInfo *pp){
	
	return KADDR(page2pa(pp));
}

Y 

static inline void*
_kaddr(const char *file, int line, physaddr_t pa){
	if (PGNUM(pa) >= npages)
		_panic(file, line, "KADDR called with invalid pa %08lx", pa);
	return (void *)(pa + KERNBASE);
}

page2kva hace un llamado a kaddr que, internamente devuelve pa+kernbase -> con pa, el resultado del llamado a page2pa 
VERIFICAAAAAAAAAAAAAR

