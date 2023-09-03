Esta libreria ha sido adaptada a microcontroladores de la serie STM como parte del desarrollo 
del Trabajo Fin de Grado.

Para incluirla en nuestro proyecto debemos copiar el fichero APDS9960.h en la carpeta: Core/inc
y el fichero APDS9960.cpp en la carpeta: Core/src

Para crear un objeto del tipo APDS9960 basta con el siguiente codigo, una vez creado el objeto 
podemos trabajar con todas las funciones incluidas en la libreria.
 -> APDS9960 apds = APDS9960(); //Objeto APDS9960