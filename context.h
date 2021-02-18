/*
* SecureWorld file context.h
contiene la estructura en C en la que se carga el contexto escrito en el fichero config.json, y que contiene la configuración de todo el sistema.
Los distintos proyectos (challenges, mirror, app descifradora etc) deben incluir este fichero para compilar pues sus DLL
se invocarán con un parámetro de tipo contexto que se definirá en context.h

Nokia Febrero 2021
*/