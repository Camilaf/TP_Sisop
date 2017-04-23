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

...


page_alloc
----------

...


