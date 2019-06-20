# IvY

## Servidor Azure:

Ip: 40.122.145.252
User: sysadmin@ivy.cl
Pass: sysadmin


## Servidor:
Ip: http://170.239.87.56:8080

User: tenant@thingsboard.org

Pass: tenant

## Acceso SSH:
ssh root@170.239.87.56 -p 22222

Pass Root: Yositengoivy!

## Acceso FTP:
sftp://root@170.239.87.56 -p 22222

Pass Root: Yositengoivy!



## Tareas Urgentes:

1. Cambiar el look and feel para clientes, cambiar logo a:
https://drive.google.com/drive/folders/1o4AAOzCreYCwZJn1CXGXEYsdMtnYo2t6?usp=sharing

--- Problemas actuales
Existen varias formas de cambiar el logo:

A - Una vez instalado postgresql y java, instalar maven.
    clonar el repositorio haciendo bajandolo por https haciendo git clone https://github.com/p-miralles/thingsboard.git (tiene ya el         archivo svg modificado)
   
   hacer build: mvn clean install
   
   hacer build local: mvn clean install -Ddockerfile.skip=false
   
   ERROR JAVA: Error: JAVA_HOME is not defined correctly. We cannot execute /usr/lib/jvm/java-8-openjdk-amd64/jre/bin/java/bin/java
   
   Nota: se siguio el siguiente link: https://thingsboard.io/docs/user-guide/install/building-from-source/
   Nota 2: PARA SALVAR EL PROBLEMA DE JAVA CON NVM, cambiar elpath de instalación como la siguiente nota, hasta antes del bin:
   I got my mistake had incorrectly set JAVA_HOME to point to bin changing 
   Code: JAVA_HOME=/usr/lib/jvm/java-6-sun/bin
   to Code:
   JAVA_HOME=/usr/lib/jvm/java-6-sun
   I had added bin at the last in above.

B - Instalar desde el .deb

    Problema: el .deb no tiene el archivo svg.

C - Hacer build desde el .tar.gz del source de la pagina de thingsboard

Problema 1: no pude descomprimir, reemplazar el archivo y volver a comprimir sin errores.
Problema 2: una vez que tengo el .tar.gz, no estoy seguro de como instalar thingsboard correctamente desde ese archivo.

2. Ojalá cambiar el fondo a blanco

[OK] 3. Arreglar problema de librerias

[OK] 4. Chequear datos llegando a Servidor



    
