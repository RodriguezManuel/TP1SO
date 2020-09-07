-------------Sistemas Operativos - ITBA-------------

Trabajo Practico Nro. 1 - Inter Process Comunication
            Segundo Cuatrimestre 2020

            Grupo Nº3
            Integrantes del grupo:
            Rodriguez, Manuel Joaquín - 60258
            Arca, Gonzalo - 60303
            Parma, Manuel - 60425

----------------------------------------------------


--------------Dependencias del proyecto-------------

-gcc (probado en versión 4.9.2)
-make (probado en versión 4.0)
-minisat

----------------------------------------------------


--------------------Instrucciones-------------------

1. Se debe ejecutar el comando:
    $> make all

2. Se debe ejecutar el programa, para esto existen 
   dos caminos posibles:

   -> Método 1

    Se escribe por línea de comando:
     $> ./master archivosCNF/* | ./vista

     donde archivosCNF es una carpeta en la que 
     se encuentran los analizables de formato
     CNF (es necesario que tengan permisos de 
     lectura para el correcto funcionamiento del
     programa). Notamos que se pueden enviar del
     mismo modo varias carpetas separadas
     por espacios, o directamente los archivos
     a parsear.

    -> Método 2
    
     Se ejecuta el master y luego el vista por 
     separado. Primero, se debe indicar en línea 
     de comando:
      $> ./master archivosCNF/*
     
     Luego, en caso de desearlo, correr el vista 
     indicando como parámetro la cantidad de
     archivos a analizar (que se encuentra en 
     la terminal donde corre master pues este la
     imprime en STDOUT) de la siguiente forma:
      $> ./vista cantArchivos

----------------------------------------------------
