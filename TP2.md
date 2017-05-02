TP2: Procesos de usuario
========================

env_alloc
---------
1)

Para el primer proceso:

Con e->env_id obtenes un 0
(1 << 12) es 4096 (0x1000)
~(NENV - 1) es -1023 (NENV = 1024)

Combinando todo:

    0+ ( 4096 & - (1023) ) hace que generation = 4096 = 0x1000

Como generation es mayor a 0, no entro al if.
Luego, e-envs = proceso - lista_de_procesos (lo que apuntan).
Aquí, e-envs = 0.

Entonces el env_id = 4096 para el primer proceso 

    env_id(1) = 0x1000

Segundo proceso

e->env_id = 0x1000  (CHEQUEAR SI VA A DAR EL ANTERIOR O UN 0. NO ENTIENDO BIEN).

    0x1000 + 0x1000 & - (0x3ff) = 8192 = 0x2000 = generation

e - envs = ??

    env_id(2) = ?

2)


env_init_percpu
---------------

...


env_pop_tf
----------

...


gdb_hello
---------

...
