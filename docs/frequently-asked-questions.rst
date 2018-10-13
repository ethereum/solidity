##########################
FAQ - Questions Fréquentes
##########################

Cette liste a été compilée par `fivedogit <mailto:fivedogit@gmail.com>`_.


******************
Questions Basiques
******************

Qu'est-ce que le "payload" d'une transaction ?
==============================================

C'est la charge utile (binaire) envoyée avec la requête.


Créer un contrat qui peut être "tué" et retourner ses fonds
===========================================================

D'abord, un avertissement : Tuer les contrats semble être une bonne idée, parce que "nettoyer"
est toujours bon, mais comme on l'a vu plus haut, il ne nettoie pas vraiment. En outre,
si l'Ether est envoyé à des contrats tués, l'Ether sera perdu à jamais.

Si vous souhaitez désactiver vos contrats, il est préférable de les **désactiver** en changeant un état interne qui empêche le lancement de ses fonctions. Cela rendra impossible
l'utilisation du contrat et l'éther envoyé au contrat sera remboursé automatiquement.

Répondons maintenant à la question : Dans un constructeur, ``msg.sender`` est le nom de l'expéditeur
créateur. Stocker-le. Puis ``selfdestruct(createur);`` pour tuer et rendre les fonds.

`example <https://github.com/fivedogit/solidity-baby-steps/blob/master/contracts/05_greeter.sol>`_

Notez que si vous faites un ``import "mortal"`` en haut e vos contrats et déclarez
``contract SomeContract is mortal { ...`` et compilez par un compilateur qui l'inclus
(comme `Remix <https://remix.ethereum.org/>`_), alors
la fonction ``kill()`` est gérée pour vous. Une fois qu'un contrat est "mortal", vous pouvez
``contractname.kill.sendTransaction({from:eth.coinbase})``, pile poil comme dans l'exemple.

Peut-on retourner une array ou ``string`` dans un appel de fonctions en Solidity ?
==================================================================================

Oui, voir `array_receiver_and_returner.sol <https://github.com/fivedogit/solidity-baby-steps/blob/master/contracts/60_array_receiver_and_returner.sol>`_.

Est-il possible d'initialiser une array à la déclaration tel que: ``string[] myarray = ["a", "b"];`` ?
======================================================================================================

Oui. Cependant il devrait être noté que ça ne marche actuellement qu'avec les tableaux  de taille
statique. Vous pouvez même générer un tableau à la volée dans la ligne de retour.

Exemple::

    pragma solidity >=0.4.16 <0.6.0;

    contract C {
        function f() public pure returns (uint8[5] memory) {
            string[4] memory adaArr = ["This", "is", "an", "array"];
            adaArr[0] = "That";
            return [1, 2, 3, 4, 5];
        }
    }

Un contrat peut-il retourner une ``struct`` ?
=============================================

Oui, mais seulement dans un appel de fonction ``internal`` ou si ``pragma experimental "ABIEncoderV2";`` est utilisé.

Si je retourne un ``enum``, je ne reçois que les ``integer`` avec web3.js. Comment avoir les noms associés ?
=========================================================================================================

Les Enums ne sont pas encore supportés par l'ABI, juste par Solidity.
Vous devrez faire la correspondance vous même, mais nous fournirons probablement des outils plus tard.

Les variables d'état peuvent-elles être initialisées à la déclaration ?
=======================================================================

Oui, possible pour tous les types (même les structs). Cependant, là encore, un tableau devra être de taille statique pour ce faire.

Examples::

    pragma solidity >=0.4.0 <0.6.0;

    contract C {
        struct S {
            uint a;
            uint b;
        }

        S public x = S(1, 2);
        string name = "Ada";
        string[4] adaArr = ["This", "is", "an", "array"];
    }

    contract D {
        C c = new C();
    }

Comment marchent les ``struct`` ?
=================================

Regardez `struct_and_for_loop_tester.sol <https://github.com/fivedogit/solidity-baby-steps/blob/master/contracts/65_struct_and_for_loop_tester.sol>`_.

Comment marche la boucle for ?
==============================

Très similaire au JS, comme dans l'exemple ci-dessous:

``for (uint i = 0; i < a.length; i ++) { a[i] = i; }``

Regardez `struct_and_for_loop_tester.sol <https://github.com/fivedogit/solidity-baby-steps/blob/master/contracts/65_struct_and_for_loop_tester.sol>`_.

Quelles sont les exemples de fonctions de manipulation de ``string`` (``substring``, ``indexOf``, ``charAt``, etc) ?
====================================================================================================================

Il existe quelques fonctions de manipulation de strings comme `stringUtils.sol <https://github.com/ethereum/dapp-bin/blob/master/library/stringUtils.sol>`_
qui seront étendues dans le futur. En addition, Arachnid a écrit `solidity-stringutils <https://github.com/Arachnid/solidity-stringutils>`_.

Pour l'instant, si vous voulez modifier une string (même seulement pour connaitre a taille),
vous devriez y convertir en ``bytes`` (représentation octale) d'abord::

    pragma solidity >=0.4.0 <0.6.0;

    contract C {
        string s;

        function append(byte c) public {
            bytes(s).push(c);
        }

        function set(uint i, byte c) public {
            bytes(s)[i] = c;
        }
    }


Puis-je concaténer 2 strings ?
==============================

Oui, vous pouvez utiliser ``abi.encodePacked``::

    pragma solidity >=0.4.0 <0.6.0;

    library ConcatHelper {
        function concat(bytes memory a, bytes memory b)
                internal pure returns (bytes memory) {
            return abi.encodePacked(a, b);
        }
    }


Pourquoi la foncttion bas-niveau ``.call()`` est moins recommendable que d'instancier un contrat dans une variable (``ContractB b;``) puis d'exécuter ses fonctions (``b.doSomething();``)?
==============================================================================================================================================================================================

Si vous utilisez des fonctions, le compilateur vous dira si les types ou vos arguments ne correspondent pas, si la fonction n'existe pas ou n'est pas visible et il encodera les arguments pour vous.

Regardez `ping.sol <https://github.com/fivedogit/solidity-baby-steps/blob/master/contracts/45_ping.sol>`_ et
`pong.sol <https://github.com/fivedogit/solidity-baby-steps/blob/master/contracts/45_pong.sol>`_.

En retournant par exemple un ``uint``, est-il possible de retourner ``undefined`` , "null" ou une valeur similaire ?
====================================================================================================================

Cela n'est pas possible, car tous les types utilisent toute la plage de valeurs binaires possibles.

Vous avez la possibilité de ``throw`` en cas d'erreur, ce qui annulera également l'ensemble de la transaction et pourrait être une bonne idée si vous avez rencontré une situation inattendue.

Si vous ne voulez pas annuler, vous pouvez retourner une seconde valeur::

    pragma solidity >0.4.23 <0.6.0;

    contract C {
        uint[] counters;

        function getCounter(uint index)
            public
            view
            returns (uint counter, bool error) {
                if (index >= counters.length)
                    return (0, true);
                else
                    return (counters[index], false);
        }

        function checkCounter(uint index) public view {
            (uint counter, bool error) = getCounter(index);
            if (error) {
                // Gère l'erreur
            } else {
                // Fait quelque chose avec counter.
                require(counter > 7, "Invalid counter value");
            }
        }
    }


Les commentaires sont-ils déployés avec le contrat et/ou augmentent t'ils le coût du déploiement (gas) ?
========================================================================================================

Non, tout ce qui n'est pas nécessaire à l'exécution est retiré à la compilation.
Ça inclut, entre autres, les commentaires, noms de variables et noms de types.

Que se passe t'il si j'envoie des Ether lors de l'appel de fonction à un contrat ?
==================================================================================

Le montant s'ajoute à la ``balance`` du contrat, tout comme l'envoi d'Ether à la création.
Vous ne pouvez envoyer une transaction comprenant de l'Ether qu'à une fonction ayant le modifieur ``payable``,
sinon une exception interromp l'exécution.

Est-il possible d'avoir un reçu de transaction pour une transaction contrat à contrat ?
=======================================================================================

Non, un appel de fonction d'un contrat à un autre ne crée pas sa propre transaction, vous devez regarder dans la transaction initiatrice. C'est aussi la raison pour laquelle plusieurs explorateurs de blocs n'affichent pas correctement l'Ether envoyé entre les contrats.


******************
Questions Avancées
******************

Comment obtenir un nombre aléatoire dans un contrat ? (implémenter un contrat de jeu de hasard automatisé)
==========================================================================================================

Obtenir de l'aléatoire correctement est souvent la partie cruciale d'un projet de crypto et la plupart des échecs résultent de mauvais générateurs de nombres aléatoires.

Si vous ne voulez pas qu'il soit sûr, vous construisez quelque chose de similaire au `coin flipper <https://github.com/fivedogit/solidity-baby-steps/blob/master/contracts/35_coin_flipper.sol>`_
mais sinon, utilisez plutôt un contrat qui fournit un l'aléatoire, comme le `RANDAO <https://github.com/randao/randao>`_.

Obtenir la valeur de retour d'une fonction non constante d'un autre contrat
===========================================================================

Le point principal est que le contrat appelant doit connaître la fonction qu'il a l'intention d'appeler.

Regardez `ping.sol <https://github.com/fivedogit/solidity-baby-steps/blob/master/contracts/45_ping.sol>`_
et `pong.sol <https://github.com/fivedogit/solidity-baby-steps/blob/master/contracts/45_pong.sol>`_.

Comment créer des tableaux à 2 dimensions ?
===========================================

Regardez `2D_array.sol <https://github.com/fivedogit/solidity-baby-steps/blob/master/contracts/55_2D_array.sol>`_.

Notez que remplir un carré 10x10 de ``uint8`` + création de contrat a pris plus de ``800,000`` gas
au moment d'écrire ces lignes. 17x17 aura pris "2 000 000 000" de gas. La limite étant fixée à
3,14 millions.... eh bien, il y a un plafond assez bas pour ce que vous pouvez créer correctement
maintenant.

Notez que simplement "créer" le tableau est gratuit, les coûts sont dans son remplissage.

Note2 : L'optimisation de l'accès au stockage peut réduire considérablement les coûts du gas, car
32 valeurs ``uint8`` peuvent être stockées dans un seul emplacement. Le problème est que ces optimisations
ne fonctionnent mal avec les boucles et ont également un problème avec la vérification des limites (bound-checking).
Vous obtiendrez de bien meilleurs résultats de ce côté là dans le futur, normalement.

Qu'arrive t'il à un mapping de ``struct``s quand il est copié dans une ``struct``?
==================================================================================

C'est une question très intéressante. Supposons que nous ayons un environnement de contrat configuré comme tel::

    struct User {
        mapping(string => string) comments;
    }

    function somefunction public {
       User user1;
       user1.comments["Hello"] = "World";
       User user2 = user1;
    }

Dans ce cas, le mappage de la structure copiée dans ``user2`` est ignoré car il n'y a pas de "liste des clés mappées".
Il n'est donc pas possible de savoir quelles valeurs doivent être copiées.

Comment initialiser un contrat avec un montant spécifique de wei ?
==================================================================

Actuellement, l'approche est un peu sale, mais il n'y a pas grand-chose à faire pour l'améliorer.
Dans le cas d'un ``contract A`` appelant une nouvelle instance du ``contract B``, les parenthèses doivent être utilisées autour du ``new B`` parce que ``B.value`` renvoie à un membre de ``B`` appelé ``value``.
Vous devrez vous assurer que les deux contrats sont conscients l'un de l'autre et que "contract B" a un constructor ``payable``.
Dans cet exemple::

    pragma solidity >0.4.99 <0.6.0;

    contract B {
        constructor() public payable {}
    }

    contract A {
        B child;

        function test() public {
            child = (new B).value(10)(); // construit un nouveau B avec 10 wei
        }
    }

Une fonction de contrat peut-elle prendre en entrée un tableau à 2 dimensions ?
===============================================================================

Si vous voulez passer des tableaux bidimensionnels entre des fonctions non internes, vous avez très probablement besoin d'utiliser ``pragma experimental "ABIEncoderV2";``.

Quelle est la relation entre ``bytes32`` et ``string`` ? Comment se fait-il que ``bytes32 somevar = "stringliteral";`` fonctionne et que signifie la valeur hexadécimale de 32 octets stockée ?
========================================================================================================================================================================

Le type ``bytes32`` peut contenir 32 octets (bruts). Dans l'affectation ``bytes32 somevar = "stringliteral";``, le texte de la ``string`` est interprété dans sa forme d'octets bruts et si vous consultez ``somevar`` et voyez une valeur hexa sur 32 octets, c'est juste ``"stringliteral`` en hexa.

Le type "bytes" est similaire, mais peut changer sa longueur.

Enfin, ``string`` est fondamentalement identique à ``bytes`` seulement qu'il est supposé contenir l'encodage UTF-8 d'une chaîne de caractères valide. Puisque ``string`` stocke les données en encodage UTF-8, il est assez coûteux de calculer le nombre de caractères dans la chaîne (l'encodage de certains caractères prennant plus d'un octet). Pour cette raison, ``string s ; s.length`` n'est pas encore supporté ni même l'accès par index ``s[2]``. Mais si vous voulez accéder à l'encodage d'octets de bas niveau de la chaîne, vous pouvez utiliser ``bytes(s).length`` et ``bytes(s)[2]`` ce qui aura pour résultat le nombre d'octets dans le codage UTF-8 de la chaîne (pas le nombre de caractères) et le second octet (pas forcément caractère) de la chaîne encodée UTF-8, respectivement.


Un contrat peut-il passer un tableau (taille statique) ou une chaîne de caractères ou encore un ``bytes`` (taille dynamique) à un autre contrat ?
=====================================================================================================

Bien sûr. Veillez à ce que si vous franchissez la limite mémoire / stockage, des copies indépendantes soient créées.::

    pragma solidity >=0.4.16 <0.6.0;

    contract C {
        uint[20] x;

        function f() public {
            g(x);
            h(x);
        }

        function g(uint[20] memory y) internal pure {
            y[2] = 3;
        }

        function h(uint[20] storage y) internal {
            y[3] = 4;
        }
    }

L'appel à ``g(x)``n'aura pas d'effet sur ``x`` car il doit créer une copie indépendante de la valeur de stockage en mémoire.
Par contre, ``h(x)`` modifie ``x`` avec succès parce que seule une référence et non une copie est transmise.

Parfois, quand j'essaie de changer la longueur d'un tableau avec par exemple ``arrayname.length = 7;``, j'obtiens une erreur de compilation ``Value must be an lvalue``. Pourquoi ?
======================================================================================================================================================================================

Vous pouvez redimensionner un tableau dynamique en storage (c'est-à-dire un tableau déclaré au niveau du contrat) avec ``arrayname.length = <une nouvelle longueur>;``. Si vous obtenez l'erreur "lvalue", vous faites probablement l'une des deux choses suivantes.

1. Vous essayez peut-être de redimensionner un tableau en "memory", ou

2. Vous essayez peut-être de redimensionner un tableau non dynamique.

::

    pragma solidity >=0.4.18 <0.6.0;

    // Ceci ne compile pas
    contract C {
        int8[] dynamicStorageArray;
        int8[5] fixedStorageArray;

        function f() public {
            int8[] memory memArr;        // Cas 1
            memArr.length++;             // illégal

            int8[5] storage storageArr = fixedStorageArray;   // Cas 2
            storageArr.length++;                             // illégal

            int8[] storage storageArr2 = dynamicStorageArray;
            storageArr2.length++;                     // légal


        }
    }

.. note::
    En Solidity, les dimensions des tableaux sont déclarées à l'envers par rapport à la façon dont vous pourriez être habitué à les déclarer en C ou Java, mais elles sont accessibles comme en C ou en Java.
    Par exemple, ``int8[][5] somearray;`` sont 5 tableaux dynamiques de ``int8``.
    La raison en est que ``T[5]`` est toujours un tableau de 5 ``T``, peu importe si ``T`` lui-même est un tableau ou non (ce n'est pas le cas en C ou Java).

Est-il possible de retourner un tableau de chaînes de caractères (``string[]``) à partir d'une fonction Solidity ?
==================================================================================================================

Uniquement lorsque ``pragma experimental "ABIEncoderV2";`` est utilisé.

Que fait l'étrange vérification suivante dans le contrat Custom Token ?
=======================================================================

::

    require((balanceOf[_to] + _valeur) >= balanceOf[_to]) ;

Les entiers dans Solidity (et la plupart des autres langages de programmation bas-niveau) sont limités à une certaine plage.
Pour ``uint256``, il s'agit de ``0`` jusqu'à ``2**256 - 1``. Si le résultat d'une opération quelconque sur ces nombres ne correspond pas à cette plage, il est tronqué. Ces troncatures peuvent avoir de `graves conséquences <https://en.bitcoin.it/wiki/Value_overflow_incident>`_, donc un code comme celui ci est nécessaire pour éviter certaines attaques.


Pourquoi les conversions explicites entre les ``bytes`` de taille fixe et les types ``int`` échouent-elles ?
============================================================================================================

Depuis la version 0.5.0, les conversions explicites entre les tableaux d'octets de taille fixe et les entiers ne sont autorisées que si les deux types ont la même taille. Cela permet d'éviter les comportements inattendus lors de la troncation ou du bourrage.
De telles conversions sont encore possibles, mais des conversions intermédiaires explicites sont nécessaires pour rendre visible la convention de troncature et de bourrage souhaitée. Voir :ref:`types-conversion-elementary-types` pour une explication complète et des exemples.


Pourquoi les nombres littéraux (dans une ``string``) ne peuvent-ils pas être convertis en types ``bytes`` de taille fixe ?
==========================================================================================================================

Depuis la version 0.5.0, seuls les nombres hexadécimaux peuvent être convertis en bytes de taille fixe et uniquement si le nombre de chiffres hexadécimaux correspond à la taille du type. Voir :ref:`types-conversion-litterals` pour une explication complète et des exemples.



Autres questions ?
==================

Si vous avez d'autres questions ou si vous ne trouvez pas la réponse à la votre ici, n'hésitez pas à nous contacter, en anglais, sur `gitter <https://gitter.im/ethereum/solidity>`_ ou à nous faire parvenir un `problème <https://github.com/ethereum/solidity/issues>`_ sur github.