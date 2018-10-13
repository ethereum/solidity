**************************************
Structure d'un fichier source Solidity
**************************************

Les fichiers sources peuvent contenir un nombre arbitraire de :ref:`définitions de contrats<contract_structure>`, directives d'import_ et :ref:`directives pragma<pragma>`.

.. index:: ! pragma

.. _pragma:

Pragmas
=======

Le mot-clé ``pragma``` peut être utilisé pour activer certaines fonctions ou vérifications du compilateur. Une directive pragma est toujours locale à un fichier source, vous devez donc ajouter pragma à tous vos fichiers si vous voulez l'activer dans tout votre projet. Si vous :ref:`importez<import>` un autre fichier, le pragma de ce fichier ne s'appliquera pas automatiquement au fichier à importer.

.. index:: ! pragma, version

.. _version_pragma:

Version Pragma
--------------

Les fichiers sources peuvent (et devraient) être annotés avec un ``version pragma`` pour refuser d'être compilés avec de futures versions de compilateurs qui pourraient introduire des changements incompatibles. Nous essayons de limiter ces changements au strict minimum, et en particulier
introduire des changements d'une manière telle que les changements de sémantique nécessiteront également des changements de syntaxe, mais ce n'est bien sûr pas toujours possible. Pour cette raison, c'est toujours une bonne idée de lire le fichier des modifications ("changelog") au moins pour les versions qui contiennent des changements de rupture, ces versions auront toujours des versions de la forme ``0.x.0`` ou ``x.0.0``.

Le ``version pragma`` est utilisée comme suit::

  pragma solidity ^0.4.0;

Un tel fichier source ne compilera pas avec un compilateur antérieur à la version 0.4.0 et ne fonctionnera pas non plus sur un compilateur à partir de la version 0.5.0 (cette deuxième condition est ajoutée en utilisant ``^``). L'idée derrière cela est la supposition qu'il n'y aura pas de changements de rupture jusqu'à la version ``0.5.0``, donc nous pouvons toujours être sûrs que notre code compilera la façon dont nous l'avons prévu. Nous ne précisons pas la version exacte de correctif du compilateur, de sorte que les versions corrigées sont toujours possibles.

Il est possible de spécifier des règles beaucoup plus complexes pour la version du compilateur, la syntaxe suit celle utilisée par `npm <https://docs.npmjs.com/misc/semver>`_.

.. note::
 L'utilisation de ``version pragma`` ne changera pas la version du compilateur.
 Il n'activera ou désactivera pas non plus les fonctions du compilateur. Il demandera simplement au compilateur de vérifier si sa version correspond à celle requise par le pragma. S'il ne correspond pas, le compilateur affichera une erreur.

.. index:: ! pragma, experimental

.. _experimental_pragma:

Pragma Expérimental
-------------------

Le deuxième pragma est le ``experimental pragma``. Il peut être utilisé pour activer des fonctions du compilateur ou de la langue qui ne sont pas encore activées par défaut.
Les pragmas expérimentaux suivants sont actuellement pris en charge :


ABIEncoderV2
~~~~~~~~~~~~

Le nouvel encodeur ABI est capable d'encoder et de décoder arbitrairement des tableaux et des structures imbriqués. Il produit un code moins optimal (l'optimiseur pour cette partie du code est encore en développement) et n'a pas reçu autant de tests que l'ancien codeur. Vous pouvez l'activer en utilisant ``pragma experimental ABIEncoderV2;``.

.. _smt_checker:

SMTChecker
~~~~~~~~~~

Ce composant doit être activé lors de la compilation du compilateur et n'est par conséquent pas forcément présent dans tous les binaires Solidity.
Les :ref:`instructions de compilation<smt_solvers_build>` expliquent comment activer cette option.
Elle est activée pour les versions PPA d'Ubuntu dans la plupart des versions, mais pas pour solc-js, les images Docker, les binaires Windows ni les binaires Linux pré-compilés.

Si vous utilisez ``pragma experimental SMTChecker;``, vous aurez des avertissements de sécurité supplémentaires qui sont obtenus en interrogeant un solveur SMT.
Le composant ne prend pas encore en charge toutes les fonctionnalités du langage Solidity et émet probablement de nombreux avertissements. Dans le cas où il signale des caractéristiques non prises en charge, l'analyse peut ne pas être cohérente.

.. index:: source file, ! import

.. _import:

Importing other Source Files
============================

Syntax and Semantics
--------------------

Importation d'autres fichiers sources
=====================================

Syntaxe et sémantique
---------------------

Solidity supporte les instructions d'importation qui sont très similaires à celles disponibles en JavaScript (à partir de ES6), bien que Solidity ne connaisse pas le concept de "default export".

Au niveau global, vous pouvez utiliser les instructions d'importation sous la forme suivante :

::

  import "filename";

Cette instruction importe tous les symboles globaux de "nom de fichier" (et les symboles qui y sont importés) dans le champ d'application global actuel (différent de celui de ES6 mais rétrocompatible pour Solidity).
Cette syntaxe simple n'est pas recommandée car elle pollue l'espace de nommage d'une manière imprévisible: Si vous ajoutez de nouveaux éléments de niveau supérieur dans "nom de fichier", ils apparaîtront automatiquement dans tous les fichiers qui importent ainsi à partir de "nom de fichier". Il est préférable d'importer explicitement des symboles spécifiques.

L'exemple suivant crée un nouveau symbole global ``symbolName`` dont les membres sont tous les symboles globaux de ``"filename"``.

::

  import * as symbolName from "filename";

En cas de collision de noms, vous pouvez également renommer les symboles lors de l'importation.
Ce code crée de nouveaux symboles globaux ``alias`` et ``symbole2`` qui font référence à ``symbole1`` et ``symbole2`` de ``"nom de fichier"``, respectivement.

::

  import {symbol1 as alias, symbol2} from "filename";



Une autre syntaxe ne fait pas partie de ES6, mais probablement pratique :

::

  import "filename" as symbolName;

qui équivaut à ``import * as symbolName from "filename";``.

Chemins
-------

Ci-dessus, ``nom-de-fichier`` est toujours traité comme un chemin avec ``/`` comme séparateur de répertoire, ``.`` comme le répertoire courant et ``..`` comme le répertoire parent. Lorsque ``.``ou ``..`` est suivi d'un caractère autre que ``/``, il n'est pas considéré comme le répertoire courant ou parent.
Tous les noms de chemins sont traités comme des chemins absolus à moins qu'ils ne commencent par le répertoire courant ``.`` ou le répertoire parent ``..``.

Pour importer un fichier ``x`` du même répertoire que le fichier courant, utilisez ``import "./x" as x;``.
Si vous utilisez ``import "x" as x;`` à la place, un fichier différent pourrait être référencé (d'un plus global "include directory").

Il repose sur le compilateur (voir ci-dessous) de résoudre les chemins.
En général, la hiérarchie des répertoires n'a pas besoin de pointer strictement sur votre système de fichiers local, elle peut aussi pointer vers les ressources en ipfs, http ou git par exemple.

.. note::
     Utilisez toujours des importations relatives comme ``import "./filename.sol";``et évitez d'utiliser ``..`` dans les spécificateurs de chemins. Dans ce dernier cas, il est probablement préférable d'utiliser des chemins globaux et de configurer les remappages comme expliqué ci-dessous.

Utilisation dans les compilateurs
---------------------------------

Lorsque vous invoquez le compilateur, vous pouvez spécifier comment découvrir le premier élément d'un chemin, ainsi que les remappages de préfixes de chemins. Par exemple, vous pouvez configurer un remappage de sorte que tout ce qui est importé du répertoire virtuel ``github.com/ethereum/dapp-bin/library`` soit réellement lu depuis votre répertoire local ``/usr/local/dapp-bin/library``.
Si plusieurs remappages s'appliquent, celui avec la clé la plus longue est essayé en premier.
Un préfixe vide n'est pas autorisé. Les remappages peuvent dépendre d'un contexte, ce qui vous permet de configurer des paquets à importer, par exemple différentes versions d'une bibliothèque du même nom.

**solc** :

Pour solc (le compilateur de ligne de commande), vous fournissez ces chemins d'accès sous la forme d'arguments ``context:prefix=target``, où les parties ``context:``et ``=target`` sont optionnelles (``prefix`` est la valeur par défaut de ``target`` dans ce cas
). Toutes les valeurs de remappage qui sont des fichiers réguliers sont compilées (y compris leurs dépendances).

Ce mécanisme est rétrocompatible (tant qu'aucun nom de fichier ne contient ``=`` ou ``:```) et ne constitue donc pas un changement de rupture. Tous les fichiers dans ou sous le répertoire ``context`` qui importent un fichier commençant par ``prefix`` sont redirigés en remplaçant ``prefix`` par ``target``.

Par exemple, si vous clonez ``github.com/ethereum/dapp-bin/`` localement vers ``/usr/local/dapp-bin``, vous pouvez utiliser ce qui suit dans votre fichier source :

::

  import "github.com/ethereum/dapp-bin/library/iterable_mapping.sol" as it_mapping;

Puis lancer le compilateur:

.. code-block:: bash

  solc github.com/ethereum/dapp-bin/=/usr/local/dapp-bin/ source.sol

Comme exemple plus complexe, supposons que vous utilisiez un module qui utilise une ancienne version de dapp-bin que vous avez extraite vers ``/usr/local/dapp-bin_old``, alors vous pouvez exécuter :

.. code-block:: bash

  solc module1:github.com/ethereum/dapp-bin/=/usr/local/dapp-bin/ \
       module2:github.com/ethereum/dapp-bin/=/usr/local/dapp-bin_old/ \
       source.sol

Cela signifie que toutes les importations du ``module2`` pointent vers l'ancienne version mais les importations du ``module1`` pointent vers la nouvelle version.

.. note::

  ``solc``vous permet seulement d'inclure des fichiers de certains répertoires. Ils doivent être dans le répertoire (ou sous-répertoire) d'un des fichiers sources explicitement spécifiés ou dans le répertoire (ou sous-répertoire) d'une cible de remappage. Si vous voulez autoriser les includes absolus directs, ajoutez le remapping ``/=//``.

S'il y a plusieurs remappages qui mènent à un fichier valide, le remappage avec le préfixe commun le plus long est choisi.

**Remix**:

`Remix <https://remix.ethereum.org/>`_ fournit un remappage automatique pour GitHub et récupère automatiquement le fichier en ligne. Vous pouvez importer le mappage itérable comme ci-dessus, par exemple:

::
  import "github.com/ethereum/dapp-bin/library/iterable_mapping.sol" as it_mapping;

Remix may add other source code providers in the future.

.. index:: ! comment, natspec

Commentaires
============

Les commentaires sur une seule ligne (``//``) et les commentaires sur plusieurs lignes (``/*...*/``) sont possibles.

::

  // Ceci est un commentaire sur une ligne.

  /*
  Ceci est un commentaire
  multi-lignes.
  */

.. note::
 Un commentaire d'une seule ligne est terminé par tout terminateur de ligne unicode (LF, VF, FF, CR, NEL, LS ou PS) en codage utf8. Le terminateur fait toujours partie du code source après le commentaire, donc si ce n'est pas un symbole ascii (que sont NEL, LS et PS), il conduira à une erreur d'analyse.

De plus, il existe un autre type de commentaire appelé commentaire natspec, pour lequel la documentation n'est pas encore écrite. Ils sont écrits avec une triple barre oblique (``///``) ou un double bloc d'astérisque (``/**... */``) et ils doivent être utilisés directement au-dessus des déclarations ou instructions de fonction.
Vous pouvez utiliser les balises de style `Doxygen <https://en.wikipedia.org/wiki/Doxygen>`_ à l'intérieur de ces commentaires pour documenter les fonctions, annoter les conditions de vérification, et fournir un **texte de confirmation** qui est montré aux utilisateurs lorsqu'ils tentent d'appeler une fonction.

Dans l'exemple suivant, nous documentons le titre du contrat, l'explication des deux paramètres d'entrée et les deux valeurs retournées.

::

    pragma solidity >=0.4.0 <0.6.0;

    /** @title Shape calculator. */
    contract ShapeCalculator {
        /** @dev Calculates a rectangle's surface and perimeter.
          * @param w Width of the rectangle.
          * @param h Height of the rectangle.
          * @return s The calculated surface.
          * @return p The calculated perimeter.
          */
        function rectangle(uint w, uint h) public pure returns (uint s, uint p) {
            s = w * h;
            p = 2 * (w + h);
        }
    }
