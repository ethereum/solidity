README.md
El lenguaje de programación orientado a contratos de solidez
Chat de matriz Chat de Gitter Foro de Solidez Twitter Seguir Mastodon Seguir

Puede hablar con nosotros en Gitter y Matrix, enviarnos un tweet en Twitter o crear un nuevo tema en el foro de Solidity. ¡Preguntas, comentarios y sugerencias son bienvenidos!

Solidity es un lenguaje de alto nivel, orientado a contratos y de tipo estático para implementar contratos inteligentes en la plataforma Ethereum.

Para obtener una buena descripción general y un buen punto de partida, consulte el portal oficial de idiomas de Solidity .

Tabla de contenido
Fondo
Construir e instalar
Ejemplo
Documentación
Desarrollo
Mantenedores
Licencia
Seguridad
Fondo
Solidity es un lenguaje de programación con llaves de tipo estático diseñado para desarrollar contratos inteligentes que se ejecutan en la máquina virtual Ethereum. Los contratos inteligentes son programas que se ejecutan dentro de una red peer-to-peer donde nadie tiene autoridad especial sobre la ejecución y, por lo tanto, permiten implementar tokens de valor, propiedad, votación y otros tipos de lógica.

Al implementar contratos, debe utilizar la última versión publicada de Solidity. Esto se debe a que con regularidad se introducen cambios importantes, así como nuevas funciones y correcciones de errores. Actualmente utilizamos un número de versión 0.x para indicar este rápido ritmo de cambio .

Construir e instalar
Las instrucciones sobre cómo construir e instalar el compilador de Solidity se pueden encontrar en la documentación de Solidity .

Ejemplo
Un programa "Hello World" en Solidity es aún menos útil que en otros idiomas, pero aún así:

// SPDX-License-Identifier: MIT 
pragma solidity > = 0 . 6 . 0  < 0 . 9 . 0 ;

contrato HelloWorld {
     función helloWorld () devoluciones puras externas  ( memoria de cadena ) {
         return "¡Hola, mundo!"   ;
    }
}
Para comenzar con Solidity, puede usar Remix , que es un IDE basado en navegador. A continuación, se muestran algunos contratos de ejemplo:

Votación
Subasta a ciegas
Compra remota segura
Canal de micropagos
Documentación
La documentación de Solidity se encuentra alojada en Read the docs .

Desarrollo
La solidez aún está en desarrollo. ¡Las contribuciones son siempre bienvenidas! Siga la Guía para desarrolladores si desea ayudar.

Puede encontrar nuestras funciones actuales y las prioridades de errores para los próximos lanzamientos en la sección de proyectos .

Mantenedores
@axic
@chriseth
Licencia
Solidity tiene la licencia GNU General Public License v3.0 .

Algunos códigos de terceros tienen sus propios términos de licencia .

Seguridad
La política de seguridad se puede encontrar aquí .
