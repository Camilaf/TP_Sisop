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
PGSHIFT = ?  (NO ENCONTRÉ LA DEFINICIÓN DEL VALOR)

Al puntero del cual queremos hallar la página física (pp) le resta el inicio del arreglo de physical pages porque este inicio estàn en igual valor de dirección virtual y física -> indica la base.
Obtenemos el valor de página física y shiftea para para ¿¿¿¿¿ir al inicio de la pàgina hallada?????

boot_alloc_pos
--------------

...


page_alloc
----------

...


