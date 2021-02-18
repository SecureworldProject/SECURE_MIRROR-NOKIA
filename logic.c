/*
* SecureWorld file logic.c
contiene la lógica común logic(ctx, operación) a cualquier operación.
SOLO una lógica que implementa la operativa cargada en loadContext() para todas las operaciones. 
invoca a parentControl(), keymaker(), cypher() y decypher().

Nokia Febrero 2021
*/