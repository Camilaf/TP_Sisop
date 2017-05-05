TP2: Procesos de usuario
========================

env_alloc
---------
1. Porción de código a considerar para la generación del id del proceso:

	// Generate an env_id for this environment.
	generation = (e->env_id + (1 << ENVGENSHIFT)) & ~(NENV - 1);
	if (generation <= 0)  // Don't create a negative env_id.
		generation = 1 << ENVGENSHIFT;
	e->env_id = generation | (e - envs);

Donde: 

	ENVGENSHIFT = 12
	NENV = 1024
	e->env_id = 0 (siempre se inicializa en 0)
En consecuencia

	(1 << 12) = 4096 = 0x1000
	~(NENV - 1) = ~(0x3ff) = 0xfffffc00

Se desarrolla a continuación el cálculo de e->env_id para los primeros cinco procesos.

- Primer proceso:

    0x00000000 + ( 0x1000 & 0xfffffc00) hace que generation = 0x00001000 
    
Como generation es mayor a 0, no entro al if.
Luego, e-envs = dirección proceso - dirección lista_de_procesos (Como ambos son punteros a struct Env, la resta en cada caso dará el número de proceso). Aquí, e-envs = 0.

Entonces el env_id(1) = 0x00001000 | 0x0 

    env_id(1) = 0x00001000

- Segundo proceso

e->env_id = 0x00000000

    	0x00000000 + (0x1000 & ~0x3ff) = 0x00001000 = generation

e - envs = 0x00000001
	
Hacemos el "or" entre e-envs y generation, obteniendo

    	env_id(2) = 0x00001001
	
- Tercer proceso

e->env_id = 0x00000000

    	0x00000000 + (0x1000 & ~0x3ff) = 0x00001000 = generation

e - envs = 0x00000010
	
Hacemos el "or" entre e-envs y generation, obteniendo

    	env_id(3) = 0x00001010

- Cuarto proceso

e->env_id = 0x00000000

    	0x00000000 + (0x1000 & ~0x3ff) = 0x00001000 = generation

e - envs = 0x00000011
	
Hacemos el "or" entre e-envs y generation, obteniendo

    	env_id(4) = 0x00001011
	
- Quinto proceso

e->env_id = 0x00000000

    	0x00000000 + (0x1000 & ~0x3ff) = 0x00001000 = generation

e - envs = 0x00000100
	
Hacemos el "or" entre e-envs y generation, obteniendo

    	env_id(5) = 0x00001100

2. En principio, siguiendo la idea del punto 1a, concluimos que: 
	
	env_id(ejecución_1) = 0x00001000 | 0x00000276 = 0x00001276

Al liberar el proceso, como todos están en ejecución (no hay otro espacio), la siguiente ejecución, va a tomar el slot que quedó disponible, manteniendo el offset con respecot a envs, ya que en env_free se lo pone primero en env_free_list, pero no se pone a env_id nuevamente en 0.
Siguiendo con esta idea:

- Segunda ejecución

Calculamos generation:
	
	generation = ( 0x00001276 + 0x1000 ) & ~(0x000003ff) = 0x00002276 & 0x11111C00 = 0x00002000
	
En consecuencia:
	
	e->env_id(ejecución_2) = 0x00002276
	
- Tercera ejecución


Calculamos generation:
	
	generation = ( 0x00002276 + 0x1000 ) & ~(0x000003ff) = 0x00003276 & 0x11111C00 = 0x00003000
	
En consecuencia:
	
	e->env_id(ejecución_3) = 0x00003276
	
- Cuarta ejecución

Calculamos generation:
	
	generation = ( 0x00003276 + 0x1000 ) & ~(0x000003ff) = 0x00004276 & 0x11111C00 = 0x00004000
	
En consecuencia:
	
	e->env_id(ejecución_4) = 0x00004276
	
- Quinta ejecución

Calculamos generation:
	
	generation = ( 0x00004276 + 0x1000 ) & ~(0x000003ff) = 0x00005276 & 0x11111C00 = 0x00005000
	
En consecuencia:
	
	e->env_id(ejecución_5) = 0x00005276
	


env_init_percpu
---------------

a. La función lgdt carga en el GDTR (Global Descriptor Table Register) los valores ubicados en el operando. Como el registro es de 48 bits, escribe 6 bytes en el mismo.

En env.c, la función env_init_percpu llama a lgdt(&gdt_pd).
Donde gdt_pd es un struct Pseudodesc gdt_pd, cuyo tamaño es de 6 bytes.

b. Esos bytes representan el tamaño y ubicación de la Global Descriptor Table (GDT): los 16 bits menos significativos contienen el tamaño (límite de tabla), mientras que en los 32 bits más significativos se encuentra la ubicación de la GDT en memoria (dirección base).

Para verificar lo mencionado buscamos la definición de los valores que se cargan, y del struct: 
struct Pseudodesc gdt_pd = { sizeof(gdt) - 1, (unsigned long) gdt };
struct Pseudodesc {
	uint16_t pd_lim;		// Limit
	uint32_t pd_base;		// Base address
}

c. La GDT es una tabla en memoria que define los segmentos de memoria del procesador, como cada hilo se ejecuta en paralelo se tendrán dos procesadores (como mínimo). Cada uno con su conjunto de registros, y específicamente, su GDTR. Por lo cual, cada uno puede apuntar a una tabla GDT distinta almacenada en memoria. 

Cada procesador necesita su propio TSS (Task State Segment) descriptor, donde TSS es una estructura de datos especial que contiene información sobre una tarea. Ésto ocurre ya que al cambiar a un nivel de mayor privilegio, carga un nuevo stack pointer para ese nivel a partir del TSS: al hacer un system call, la CPU obtiene el valor de SS0 (stack segment selector para CPL=0) y ESP0 (valor nuevo de ESP para CPL=0) del TSS, permitiendo que el kernel utilice un stack diferente al del programa de usuario. Se podría tener una única tabla GDT con varios TSS descriptors, o bien una GDT para cada procesador.


env_pop_tf
----------
1. ¿Qué hay en (%esp) tras el primer movl de la función?
El primer movl de la función (movl %0,%%esp) lo que hace es almacenar el primer argumento (correspondiente a tf), que es el puntero al struct Trapframe del proceso, en el registro %esp. Por lo cual, luego de ejecutar la instruc, %esp apunta al inicio del struct Trapframe asociado con el proceso. Específicamente, en (%esp) se encuentra el struct PushRegs tf_regs.

2. ¿Qué hay en (%esp) justo antes de la instrucción iret? ¿Y en 8(%esp)?
La función usa distintas instrucciones para restaurar los registros a partir del Trapframe: popal recupera los general registers, luego popl recupera %es y %ds. La instrucción addl "esquiva" los campos trapno y errcode. 

Lo siguiente en el struct es tf_eip, por lo cual %esp apuntará al valor del instruction pointer, siendo (%esp) igual a la dirección de la siguiente instrucción a ser ejecutada. Si a %esp le sumamos 8 bytes, vamos a pasar por la dirección de tf_cs (que se encuentra a 4 bytes) para llegar a la dirección donde se encuentra tf_eflags. Entonces 8(%esp) corresponderá al valor de %eflags.

3. ¿Cómo puede determinar la CPU si hay un cambio de ring (nivel de privilegio)?
Para cada nivel de privilegio (0 y 3) existe una pila de ejecución propia de ese nivel. Para la pila de privilegio 3, se guardan SS y ESP, los cuales automáticamente se salvan al llamar un proceso con mayor privilegio. En el caso de la pila de nivel 0, el puntero de esta se guarda en el TSS.
Este manejo es invisible al proceso, excepto cuando hay una excepción por intentar acceder con derechos que no se tienen en ese proceso.

La CPU puede determinar el cambio de ring mediante los valores RPL (Requested Privilege Level) y CPL (Current Privilege Level). Por ejemplo, para realizar una syscall se controla que CPL en %cs sea menor o igual a DPL (Descriptor Privilege Level), y los registros %ss y %esp se salvan únicamente cuando el valor de privilege level del selector es menor a CPL.

gdb_hello
---------

1. 
$ make gdb
gdb -q -s obj/kern/kernel -ex 'target remote 127.0.0.1:26000' -n -x .gdbinit
Leyendo símbolos desde obj/kern/kernel...hecho.
Remote debugging using 127.0.0.1:26000
0x0000fff0 in ?? ()
(gdb) b env_pop_tf
Punto de interrupción 1 at 0xf0102da9: file kern/env.c, line 476.
(gdb) c
Continuando.
Se asume que la arquitectura objetivo es i386
=> 0xf0102da9 <env_pop_tf>:	push   %ebp

Breakpoint 1, env_pop_tf (tf=0xf01b7000) at kern/env.c:476
476	{

2.
(qemu) info registers
EAX=003bc000 EBX=f01b7000 ECX=f03bc000 EDX=0000020f
ESI=00010094 EDI=00000000 EBP=f0118fd8 ESP=f0118fbc
EIP=f0102da9 EFL=00000092 [--S-A--] CPL=0 II=0 A20=1 SMM=0 HLT=0
ES =0010 00000000 ffffffff 00cf9300 DPL=0 DS   [-WA]
CS =0008 00000000 ffffffff 00cf9a00 DPL=0 CS32 [-R-]

3.
(gdb) p tf
$1 = (struct Trapframe *) 0xf01b7000

4.
(gdb) x/17x tf
0xf01b7000:	0x00000000	0x00000000	0x00000000	0x00000000
0xf01b7010:	0x00000000	0x00000000	0x00000000	0x00000000
0xf01b7020:	0x00000023	0x00000023	0x00000000	0x00000000
0xf01b7030:	0x00800020	0x0000001b	0x00000000	0xeebfe000
0xf01b7040:	0x00000023

5.
(gdb) disas
Dump of assembler code for function env_pop_tf:
=> 0xf0102da9 <+0>:	push   %ebp
   0xf0102daa <+1>:	mov    %esp,%ebp
   0xf0102dac <+3>:	sub    $0xc,%esp
   0xf0102daf <+6>:	mov    0x8(%ebp),%esp
   0xf0102db2 <+9>:	popa   
   0xf0102db3 <+10>:	pop    %es
   0xf0102db4 <+11>:	pop    %ds
   0xf0102db5 <+12>:	add    $0x8,%esp
   0xf0102db8 <+15>:	iret   
   0xf0102db9 <+16>:	push   $0xf0105373
   0xf0102dbe <+21>:	push   $0x1e6
   0xf0102dc3 <+26>:	push   $0xf010532e
   0xf0102dc8 <+31>:	call   0xf01000a9 <_panic>
End of assembler dump.
(gdb) si 4
=> 0xf0102db2 <env_pop_tf+9>:	popa   
0xf0102db2	477		asm volatile("\tmovl %0,%%esp\n"

6.
(gdb) x/17x $sp
0xf01b7000:	0x00000000	0x00000000	0x00000000	0x00000000
0xf01b7010:	0x00000000	0x00000000	0x00000000	0x00000000
0xf01b7020:	0x00000023	0x00000023	0x00000000	0x00000000
0xf01b7030:	0x00800020	0x0000001b	0x00000000	0xeebfe000
0xf01b7040:	0x00000023

7.


8.
(gdb) si 4
=> 0xf0102db8 <env_pop_tf+15>:	iret   
0xf0102db8	477		asm volatile("\tmovl %0,%%esp\n"

(qemu) info registers
EAX=00000000 EBX=00000000 ECX=00000000 EDX=00000000
ESI=00000000 EDI=00000000 EBP=00000000 ESP=f01b7030
EIP=f0102db8 EFL=00000096 [--S-AP-] CPL=0 II=0 A20=1 SMM=0 HLT=0
ES =0023 00000000 ffffffff 00cff300 DPL=3 DS   [-WA]
CS =0008 00000000 ffffffff 00cf9a00 DPL=0 CS32 [-R-]

9.
(gdb) si
=> 0x800020:	cmp    $0xeebfe000,%esp
0x00800020 in ?? ()
(gdb) p $pc 
$2 = (void (*)()) 0x800020
(gdb) symbol-file obj/user/hello
¿Cargar una tabla de símbolos nueva desde «obj/user/hello»? (y or n) y
Leyendo símbolos desde obj/user/hello...hecho.
Error in re-setting breakpoint 1: Función «env_pop_tf» no definida.
(gdb) p $pc
$3 = (void (*)()) 0x800020 <_start>

10.
(gdb) tbreak syscall
Punto de interrupción temporal 2 at 0x800a04: file lib/syscall.c, line 23.
(gdb) c
Continuando.
=> 0x800a04 <syscall+17>:	mov    0x8(%ebp),%ecx

Temporary breakpoint 2, syscall (num=0, check=-289415544, a1=<unknown type>, 
    a2=<unknown type>, a3=<unknown type>, a4=<unknown type>, a5=<unknown type>)
    at lib/syscall.c:23
23		asm volatile("int %1\n"
(gdb) disas
Dump of assembler code for function syscall:
   0x008009f3 <+0>:	push   %ebp
   0x008009f4 <+1>:	mov    %esp,%ebp
   0x008009f6 <+3>:	push   %edi
   0x008009f7 <+4>:	push   %esi
   0x008009f8 <+5>:	push   %ebx
   0x008009f9 <+6>:	sub    $0x1c,%esp
   0x008009fc <+9>:	mov    %eax,-0x20(%ebp)
   0x008009ff <+12>:	mov    %edx,-0x1c(%ebp)
   0x00800a02 <+15>:	mov    %ecx,%edx
=> 0x00800a04 <+17>:	mov    0x8(%ebp),%ecx
   0x00800a07 <+20>:	mov    0xc(%ebp),%ebx
   0x00800a0a <+23>:	mov    0x10(%ebp),%edi
   0x00800a0d <+26>:	mov    0x14(%ebp),%esi
   0x00800a10 <+29>:	int    $0x30
   0x00800a12 <+31>:	cmpl   $0x0,-0x1c(%ebp)
   0x00800a16 <+35>:	je     0x800a35 <syscall+66>
   0x00800a18 <+37>:	test   %eax,%eax
   0x00800a1a <+39>:	jle    0x800a35 <syscall+66>
   0x00800a1c <+41>:	mov    -0x20(%ebp),%edx
   0x00800a1f <+44>:	sub    $0xc,%esp
   0x00800a22 <+47>:	push   %eax
   0x00800a23 <+48>:	push   %edx
---Type <return> to continue, or q <return> to quit---
   0x00800a24 <+49>:	push   $0x800fd4
   0x00800a29 <+54>:	push   $0x23
   0x00800a2b <+56>:	push   $0x800ff1
   0x00800a30 <+61>:	call   0x800acd <_panic>
   0x00800a35 <+66>:	lea    -0xc(%ebp),%esp
   0x00800a38 <+69>:	pop    %ebx
   0x00800a39 <+70>:	pop    %esi
   0x00800a3a <+71>:	pop    %edi
   0x00800a3b <+72>:	pop    %ebp
   0x00800a3c <+73>:	ret    
End of assembler dump.
(gdb) si 4
=> 0x800a10 <syscall+29>:	int    $0x30
0x00800a10	23		asm volatile("int %1\n"
(gdb) si
aviso: A handler for the OS ABI "GNU/Linux" is not built into this configuration
of GDB.  Attempting to continue with the default i8086 settings.

Se asume que la arquitectura objetivo es i8086
[f000:e05b]    0xfe05b:	cmpl   $0x0,%cs:0x70c8
0x0000e05b in ?? ()
(gdb) 

[ Agregando -no-reboot me   queda: 
(gdb) si 4
=> 0x800a10 <syscall+29>:	int    $0x30
0x00800a10	23		asm volatile("int %1\n"
(gdb) stepi
Conexión remota cerrada ] Cual iria?


(qemu) info registers
EAX=00000000 EBX=00000000 ECX=00000000 EDX=00000000
ESI=00000000 EDI=00000000 EBP=00000000 ESP=eebfe000
EIP=00800020 EFL=00000002 [-------] CPL=3 II=0 A20=1 SMM=0 HLT=0
ES =0023 00000000 ffffffff 00cff300 DPL=3 DS   [-WA]
CS =001b 00000000 ffffffff 00cffa00 DPL=3 CS32 [-R-]
