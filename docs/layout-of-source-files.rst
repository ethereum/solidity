**************************************
Structure d'un fichier source Solidity
**************************************

Les fichiers sources peuvent contenir un nombre arbitraire de :ref:`définitions de contrats<contract_structure>`, directives d'import_ et :ref:`directives pragma<pragma>` et définitions de 
:ref:`structs<structs>` et d':ref:`enums<enums>`, :ref:`fonctions<functions>`, :ref:`errors<errors>`
et ddéfinitions de :ref:`variables constantes<constants>`.

.. index:: ! license, spdx

SPDX License Identifier
=======================

Trust in smart contracts can be better established if their source code
is available. Since making source code available always touches on legal problems
with regards to copyright, the Solidity compiler encourages the use
of machine-readable `SPDX license identifiers <https://spdx.org>`_.
Every source file should start with a comment indicating its license:

``// SPDX-License-Identifier: MIT``

The compiler does not validate that the license is part of the
`list allowed by SPDX <https://spdx.org/licenses/>`_, but
it does include the supplied string in the :ref:`bytecode metadata <metadata>`.

If you do not want to specify a license or if the source code is
not open-source, please use the special value ``UNLICENSED``.
Note that ``UNLICENSED`` (no usage allowed, not present in SPDX license list)
is different from ``UNLICENSE`` (grants all rights to everyone).
Solidity follows `the npm recommendation <https://docs.npmjs.com/cli/v7/configuring-npm/package-json#license>`_.

Supplying this comment of course does not free you from other
obligations related to licensing like having to mention
a specific license header in each source file or the
original copyright holder.

The comment is recognized by the compiler anywhere in the file at the
file level, but it is recommended to put it at the top of the file.

More information about how to use SPDX license identifiers
can be found at the `SPDX website <https://spdx.org/ids-how>`_.


.. index:: ! pragma

.. _pragma:

Pragmas
=======

Le mot-clé ``pragma``` peut être utilisé pour activer certaines fonctions ou vérifications du compilateur. Une directive pragma est toujours locale à un fichier source, donc vous devez donc ajouter pragma à tous vos fichiers si vous voulez l'activer dans tout votre projet.
Si vous :ref:`importez<import>` un autre fichier, le pragma de ce fichier ne s'appliquera *pas* automatiquement au fichier à importer.

.. index:: ! pragma, version

.. _version_pragma:

Version Pragma
--------------

Les fichiers sources peuvent (et devraient) être annotés avec un ``version pragma`` pour refuser d'être compilés avec de futures versions de compilateurs qui pourraient introduire des changements incompatibles. Nous essayons de limiter ces changements au strict minimum, et en particulier
introduire des changements d'une manière telle que les changements de sémantique nécessiteront également des changements de syntaxe, mais ce n'est bien sûr pas toujours possible. Pour cette raison, c'est toujours une bonne idée de lire le fichier des modifications ("changelog") au moins pour les versions qui contiennent des changements de rupture, ces versions auront toujours des versions de la forme ``0.x.0`` ou ``x.0.0``.

Le ``version pragma`` est utilisée comme suit::

  pragma solidity ^0.4.0;

Un tel fichier source ne compilera pas avec un compilateur antérieur à la version 0.4.0 et ne fonctionnera pas non plus sur un compilateur à partir de la version 0.5.0 (cette deuxième condition est ajoutée en utilisant ``^``). L'idée derrière cela est la supposition qu'il n'y aura pas de changements de rupture jusqu'à la version ``0.5.0``, donc nous pouvons toujours être sûrs que notre code compilera la façon dont nous l'avons prévu. Nous ne précisons pas la version exacte de correctif du compilateur, de sorte que les versions corrigées sont toujours possibles.

Il est possible de spécifier des règles beaucoup plus complexes pour la version du compilateur, la syntaxe suit celle utilisée par `npm <https://docs.npmjs.com/cli/v6/using-npm/semver>`_.

.. note::
 L'utilisation de ``version pragma`` ne changera pas la version du compilateur.
 Il n'activera ou désactivera pas non plus les fonctions du compilateur. Il demandera simplement au compilateur de vérifier si sa version correspond à celle requise par le pragma. S'il ne correspond pas, le compilateur affichera une erreur.

ABI Coder Pragma
----------------

By using ``pragma abicoder v1`` or ``pragma abicoder v2`` you can
select between the two implementations of the ABI encoder and decoder.

The new ABI coder (v2) is able to encode and decode arbitrarily nested
arrays and structs. It might produce less optimal code and has not
received as much testing as the old encoder, but is considered
non-experimental as of Solidity 0.6.0. You still have to explicitly
activate it using ``pragma abicoder v2;``. Since it will be
activated by default starting from Solidity 0.8.0, there is the option to select
the old coder using ``pragma abicoder v1;``.

The set of types supported by the new encoder is a strict superset of
the ones supported by the old one. Contracts that use it can interact with ones
that do not without limitations. The reverse is possible only as long as the
non-``abicoder v2`` contract does not try to make calls that would require
decoding types only supported by the new encoder. The compiler can detect this
and will issue an error. Simply enabling ``abicoder v2`` for your contract is
enough to make the error go away.

.. note::
  This pragma applies to all the code defined in the file where it is activated,
  regardless of where that code ends up eventually. This means that a contract
  whose source file is selected to compile with ABI coder v1
  can still contain code that uses the new encoder
  by inheriting it from another contract. This is allowed if the new types are only
  used internally and not in external function signatures.

.. note::
  Up to Solidity 0.7.4, it was possible to select the ABI coder v2
  by using ``pragma experimental ABIEncoderV2``, but it was not possible
  to explicitly select coder v1 because it was the default.

.. index:: ! pragma, experimental

.. _experimental_pragma:

Pragma Expérimental
-------------------

Le deuxième pragma est le ``experimental pragma``. Il peut être utilisé pour activer des fonctions du compilateur ou de la langue qui ne sont pas encore activées par défaut.
Les pragmas expérimentaux suivants sont actuellement pris en charge :


ABIEncoderV2
~~~~~~~~~~~~

Because the ABI coder v2 is not considered experimental anymore,
it can be selected via ``pragma abicoder v2`` (please see above)
since Solidity 0.7.4.

.. _smt_checker:

SMTChecker
~~~~~~~~~~

Ce composant doit être activé lors de la compilation du compilateur et n'est par conséquent pas forcément présent dans tous les binaires Solidity.
Les :ref:`instructions de compilation<smt_solvers_build>` expliquent comment activer cette option.
Elle est activée pour les versions PPA d'Ubuntu dans la plupart des versions, mais pas pour solc-js, les images Docker, les binaires Windows ni les binaires Linux pré-compilés. It can be activated for solc-js via the
`smtCallback <https://github.com/ethereum/solc-js#example-usage-with-smtsolver-callback>`_ if you have an SMT solver
installed locally and run solc-js via node (not via the browser).

Si vous utilisez ``pragma experimental SMTChecker;``, vous aurez des :ref:`avertissements de sécuristé<formal_verification>` supplémentaires qui sont obtenus en interrogeant un solveur SMT.
Le composant ne prend pas encore en charge toutes les fonctionnalités du langage Solidity et émet probablement de nombreux avertissements. Dans le cas où il signale des caractéristiques non prises en charge, l'analyse peut ne pas être cohérente.

.. index:: source file, ! import, module, source unit

.. _import:

Importation d'autres fichiers sources
=====================================

Syntaxe et sémantique
---------------------

Solidity supporte les instructions d'importation qui sont très similaires à celles disponibles en JavaScript (à partir de ES6), bien que Solidity ne connaisse pas le concept de `default export <https://developer.mozilla.org/en-US/docs/web/javascript/reference/statements/export#Description>`_.

Au niveau global, vous pouvez utiliser les instructions d'importation sous la forme suivante :

.. code-block:: solidity

    import "filename";

The ``filename`` part is called an *import path*.
Cette instruction importe tous les symboles globaux de "filename" (et les symboles qui y sont importés) dans le champ d'application global actuel (différent de celui de ES6 mais rétrocompatible pour Solidity).
Cette syntaxe simple n'est pas recommandée car elle pollue l'espace de noms d'une manière imprévisible: Si vous ajoutez de nouveaux éléments de niveau supérieur dans "filename", ils apparaîtront automatiquement dans tous les fichiers qui importent ainsi à partir de "nom de fichier". Il est préférable d'importer explicitement des symboles spécifiques.

L'exemple suivant crée un nouveau symbole global ``symbolName`` dont les membres sont tous les symboles globaux de ``"filename"``.


.. code-block:: solidity

    import * as symbolName from "filename";

which results in all global symbols being available in the format ``symbolName.symbol``.

A variant of this syntax that is not part of ES6, but possibly useful is:

.. code-block:: solidity

  import "filename" as symbolName;

which is equivalent to ``import * as symbolName from "filename";``.

En cas de collision de noms, vous pouvez également renommer les symboles lors de l'importation.
Ce code crée de nouveaux symboles globaux ``alias`` et ``symbole2`` qui font référence à ``symbole1`` et ``symbole2`` de ``"nom de fichier"``, respectivement.

.. code-block:: solidity

    import {symbol1 as alias, symbol2} from "filename";

.. index:: virtual filesystem, source unit name, import; path, filesystem path, import callback, Remix IDE

Import Paths
------------

In order to be able to support reproducible builds on all platforms, the Solidity compiler has to
abstract away the details of the filesystem where source files are stored.
For this reason import paths do not refer directly to files in the host filesystem.
Instead the compiler maintains an internal database (*virtual filesystem* or *VFS* for short) where
each source unit is assigned a unique *source unit name* which is an opaque and unstructured identifier.
The import path specified in an import statement is translated into a source unit name and used to
find the corresponding source unit in this database.

Using the :ref:`Standard JSON <compiler-api>` API it is possible to directly provide the names and
content of all the source files as a part of the compiler input.
In this case source unit names are truly arbitrary.
If, however, you want the compiler to automatically find and load source code into the VFS, your
source unit names need to be structured in a way that makes it possible for an :ref:`import callback
<import-callback>` to locate them.
When using the command-line compiler the default import callback supports only loading source code
from the host filesystem, which means that your source unit names must be paths.
Some environments provide custom callbacks that are more versatile.
For example the `Remix IDE <https://remix.ethereum.org/>`_ provides one that
lets you `import files from HTTP, IPFS and Swarm URLs or refer directly to packages in NPM registry
<https://remix-ide.readthedocs.io/en/latest/import.html>`_.

For a complete description of the virtual filesystem and the path resolution logic used by the
compiler see :ref:`Path Resolution <path-resolution>`.

.. index:: ! comment, natspec

Commentaires
============

Les commentaires sur une seule ligne (``//``) et les commentaires sur plusieurs lignes (``/*...*/``) sont possibles.

.. code-block:: solidity

  // Ceci est un commentaire sur une ligne.

  /*
  Ceci est un commentaire
  multi-lignes.
  */

.. note::
 Un commentaire d'une seule ligne est terminé par tout terminateur de ligne unicode (LF, VF, FF, CR, NEL, LS ou PS) en codage utf8. Le terminateur fait toujours partie du code source après le commentaire, donc si ce n'est pas un symbole ascii (que sont NEL, LS et PS), il conduira à une erreur de parsing.

De plus, il existe un autre type de commentaire appelé commentaire natspec, détaillé dans :ref:`style guide<natspec>`. Ils sont écrits avec une triple barre oblique (``///``) ou un double bloc d'astérisque (``/**... */``) et ils doivent être utilisés directement au-dessus des déclarations ou instructions de fonction.
