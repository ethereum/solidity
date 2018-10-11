###############################
Introduction aux Smart Contracts
###############################

.. _simple-smart-contract:

************************
Un Smart Contract simple
************************

Commençons par un exemple de base qui fixe la valeur d'une variable et l'expose pour l'accès par d'autres contrats. C'est très bien si vous ne comprenez pas tout maintenant, nous entrerons plus en détail plus tard.

Storage
=======

::

    pragma solidity >=0.4.0 <0.6.0;

    contract SimpleStorage {
        uint storedData;

        function set(uint x) public {
            storedData = x;
        }

        function get() public view returns (uint) {
            return storedData;
        }
    }

La première ligne indique simplement que le code source est écrit pour Solidity version 0.4.0 ou tout ce qui est plus récent qui ne casse pas la fonctionnalité (jusqu'à la version 0.6.0, mais non comprise). Il s'agit de s'assurer que le contrat n'est pas compilable avec une nouvelle version du compilateur (de rupture), où il pourrait se comporter différemment.
Les pré-cités pragmas sont des instructions courantes pour les compilateurs sur la façon de traiter le code source (par exemple `pragma once <https://fr.wikipedia.org/wiki/Pragma_once>`_).

Un contrat au sens de Solidity est un ensemble de code (ses *fonctions*) et les données (son *état*) qui résident à une adresse spécifique sur la blockchain Ethereum. La ligne ``uint storedData;`` déclare une variable d'état appelée ``storedData``` de type ``uint`` (*u*nsigned *int*eger de *256* bits). Vous pouvez le considérer comme une case mémoire dans une base de données qui peut être interrogée et modifiée en appelant les fonctions du code qui gèrent la base de données. Dans le cas d'Ethereum, c'est toujours le contrat propriétaire. Et dans ce cas, les fonctions ``set`` et ``get`` peuvent être utilisées pour modifier
ou récupérer la valeur de la variable.

Pour accéder à une variable d'état, vous n'avez pas besoin du préfixe ``this.`` d'autres langues.

Ce contrat ne fait pas encore grand-chose en dehors de (en raison de l'infrastructure construite par Ethereum) permettre à n'importe qui de stocker un numéro unique qui est accessible par n'importe qui dans le monde sans un moyen (faisable) pour vous empêcher de publier ce numéro. Bien sûr, n'importe qui peut simplement appeler ``set`` à nouveau avec une valeur différente.
et écraser votre numéro, mais le numéro sera toujours stocké dans l'historique de la blockchain. Plus tard, nous verrons comment vous pouvez imposer des restrictions d'accès pour que vous seul puissiez modifier le numéro.

.. note::
    Tous les identifiants (noms de contrat, noms de fonctions et noms de variables) sont limités au jeu de caractères ASCII. Il est possible de stocker des données encodées en UTF-8 dans des variables de type string.

.. warning::
    Soyez prudent lorsque vous utilisez du texte Unicode, car des caractères d'apparence similaire (ou même identique) peuvent avoir des codages unicode différents et seront donc codés sous la forme d'un tableau d'octets différent.

.. index:: ! subcurrency

Exemple de sous-monnaie
=======================

Le contrat suivant mettra en œuvre la forme la plus simple d'un contrat de
cryptomonnaie. Il est possible de générer des pièces à partir de rien, mais
seule la personne qui a créé le contrat sera en mesure de le faire (il est facile de mettre en œuvre un schéma d'émission différent).
De plus, n'importe qui peut s'envoyer des pièces sans avoir besoin de s'enregistrer avec un nom d'utilisateur et un mot de passe - tout ce dont vous avez besoin est une paire de clés Ethereum.


::

    pragma solidity >0.4.99 <0.6.0;

    contract Coin {
        // Le mot-clé "public" rend ces variables
        // facilement accessible de l'exterieur.
        address public minter;
        mapping (address => uint) public balances;

        // Les Events authowisent les clients légers à réagir
        // aux changements efficacement.
        event Sent(address from, address to, uint amount);

        // C'est le constructor, code qui n'est exécuté
        // qu'à la création du contrat.
        constructor() public {
            minter = msg.sender;
        }

        function mint(address receiver, uint amount) public {
            require(msg.sender == minter);
            require(amount < 1e60);
            balances[receiver] += amount;
        }

        function send(address receiver, uint amount) public {
            require(amount <= balances[msg.sender], "Insufficient balance.");
            balances[msg.sender] -= amount;
            balances[receiver] += amount;
            emit Sent(msg.sender, receiver, amount);
        }
    }

Ce contrat introduit quelques nouveaux concepts, passons-les en revue un à un.
La ligne ``address public minter;`` déclare une variable d'état de type adress qui est accessible au public. Le type ``adress`` est une valeur de 160 bits qui ne permet aucune opération arithmétique. Il convient pour le stockage des adresses de contrats ou de paires de clés appartenant à des tiers. Le mot-clé "public" génère automatiquement une fonction qui permet d'accéder à la valeur courante de la variable d'état de l'extérieur du contrat.
Sans ce mot-clé, les autres contrats n'ont aucun moyen d'accéder à la variable.
Le code de la fonction générée par le compilateur est à peu près équivalent à ce qui suit (ignorez ``external'' et ``view`` pour l'instant)::

    function minter() external view returns (address) { return minter; }

Bien sûr, l'ajout d'une fonction exactement comme celle-là ne fonctionnera pas parce que nous aurions une fonction et une variable d'état avec le même nom, mais vous avez l'idée - le compilateur réalisera cela pour vous.

.. index:: mapping

La ligne suivante, ``mapping (" adress => uint ") public balances;`` 
crée également une variable d'état publique, mais c'est un type de données plus complexe.
Le type fait correspondre les adresses aux entiers non signés.
Les mappings peuvent être vus comme des `tables de hachage <https://en.wikipedia.org/wiki/Hash_table>`_ qui sont
virtuellement initialisées de sorte que toutes les clés possibles existent dès le début et sont mappées à un fichier
dont la représentation octale n'est que de zéros. Cette analogie ne va pas
trop loin, car il n'est pas non plus possible d'obtenir une liste de toutes les clés d'un mapping, ni une liste de toutes les valeurs. Il faut donc garder à l'esprit (ou bien
mieux, gardez une liste ou utilisez un type de données plus avancé) ce que vous avez ajouté à la cartographie ou l'utiliser dans un contexte où cela n'est pas nécessaire.
La :ref:`fonction getter<fonctiongetter-fonctions>` créé par le mot-clé ``public`` est un peu plus complexe dans ce cas. Ça ressemble grossièrement à ça::

    function balances(address _account) external view returns (uint) {
        return balances[_account];
    }

Comme vous pouvez le voir, vous pouvez utiliser cette fonction pour interroger facilement le solde d'un seul compte.

.. index:: event

La ligne ``event Sent(address from, address to, uint amount);`` déclare un bien-nommé "event" qui est émis dans la dernière ligne de la fonction ``send``. Les interfaces utilisateur (ainsi que les applications serveur bien sûr) peuvent écouter les événements qui sont émis sur la blockchain sans trop de frais. Dès qu'elle est émise, l'auditeur reçoit également le message
des arguments "from", "to" et "amount", ce qui facilite le suivi des transactions. Pour écouter cet événement, vous devriez utiliser le code JavaScript suivant (qui suppose que ``Coin` est un objet de contrat créé via web3.js ou un module similaire)::

    Coin.Sent().watch({}, '', function(error, result) {
        if (!error) {
            console.log("Coin transfer: " + result.args.amount +
                " coins were sent from " + result.args.from +
                " to " + result.args.to + ".");
            console.log("Balances now:\n" +
                "Sender: " + Coin.balances.call(result.args.from) +
                "Receiver: " + Coin.balances.call(result.args.to));
        }
    })

Notez comment la fonction ``balances`` générée automatiquement est appelée depuis l'interface utilisateur.

.. index:: coin

Le constructor est une fonction spéciale qui est exécutée pendant la création du contrat et ne peut pas être appelée ultérieurement. Il stocke de façon permanente l'adresse de la personne qui crée le contrat : ``msg`` (avec ``tx`` et ``block``) est une variable globale spéciale qui contient certaines propriétés qui permettent d'accéder à la blockchain. ``msg.sender`` est toujours l'adresse d'où vient l'appel de la fonction courante (externe).

Enfin, les fonctions qui finiront avec le contrat et qui peuvent être appelées par les utilisateurs et les contrats sont "mint" et "send".
Si "mint" est appelé par quelqu'un d'autre que le compte qui a créé le contrat, rien ne se passera. Ceci est assuré par la fonction spéciale ``require`` qui fait que tous les changements sont annulés si son argument est évalué à faux.
Le deuxième appel à ``require`` permet de s'assurer qu'il n'y aura pas trop de pièces, ce qui pourrait causer des erreurs de débordement de buffer plus tard.

D'un autre côté, ``send`` peut être utilisé par n'importe qui (qui a déjà certaines de ces pièces) pour envoyer des pièces à n'importe qui d'autre. Si vous n'avez pas assez de pièces à envoyer, l'appel ``require`` échouera et fournira également à l'utilisateur un message d'erreur approprié.

.. note::
    Si vous utilisez ce contrat pour envoyer des pièces à une adresse, vous ne verrez rien lorsque vous regarderez cette adresse sur un explorateur de chaîne de blocs, parce que le fait que vous avez envoyé des pièces et les soldes modifiés sont seulement stockés dans le stockage de données de ce contrat de pièces particulier. Par l'utilisation d'événements, il est relativement facile de créer un "explorateur de chaîne" qui suit les transactions et les soldes de votre nouvelle pièce, mais vous devez inspecter l'adresse du contrat de pièces et non les adresses des propriétaires des pièces.

.. _blockchain-basics:

*****************
Blockchain Basics
*****************

Les blockchains en tant que concept ne sont pas trop difficiles à comprendre pour les programmeurs. La raison en est que
la plupart des complications (mining, `hashing <https://en.wikipedia.org/wiki/Cryptographic_hash_function>`_, `elliptic-curve cryptography <https://en.wikipedia.org/wiki/Elliptic_curve_cryptography>`_, `réseaux pair-à-pair <https://en.wikipedia.org/wiki/Peer-to-peer>`_, etc.)
sont juste là pour fournir un certain nombre de fonctionnalités et de promesses pour la plate-forme. Une fois que vous prenez ces fonctions pour aquises, vous n'avez pas à vous soucier de la technologie sous-jacente - ou devez-vous savoir comment fonctionne le cloud AWS d'Amazon en interne afin de l'utiliser ?

.. index:: transaction

Transactions
============

Une blockchain est une base de données transactionnelle partagée à l'échelle mondiale.
Cela signifie que tout le monde peut lire les entrées de la base de données simplement en participant au réseau.
Si vous voulez modifier quelque chose dans la base de données, vous devez créer une transaction qui doit être acceptée par tous les autres.
Le mot transaction implique que la modification que vous voulez effectuer (en supposant que vous voulez modifier deux valeurs en même temps) n'est pas effectuée du tout ou est complètement appliquée. De plus, pendant que votre transaction est appliquée à la base de données, aucune autre transaction ne peut la modifier.

Par exemple, imaginez un tableau qui énumère les soldes de tous les comptes dans une devise électronique. Si un transfert d'un compte à un autre est demandé, la nature transactionnelle de la base de données garantit que si le montant est soustrait d'un compte, il est toujours ajouté à l'autre compte. Si, pour quelque raison que ce soit, il n'est pas possible d'ajouter le montant au compte cible, le compte source n'est pas non plus modifié.

De plus, une transaction est toujours signée cryptographiquement par l'expéditeur (créateur).
Il est donc facile de garder l'accès à des modifications spécifiques de la base de données. Dans l'exemple de la monnaie électronique, un simple contrôle permet de s'assurer que seule la personne qui détient les clés du compte peut transférer de l'argent à partir de celui-ci.

.. index:: ! block

Blocs
=====

Un obstacle majeur à surmonter est ce que l'on appelle (en termes Bitcoin) une " attaque de double dépense " :
Que se passe-t-il si deux transactions existent dans le réseau et que toutes deux veulent vider un compte ?
Une seule des transactions peut être valide, généralement celle qui est acceptée en premier.
Le problème est que "premier" n'est pas un terme objectif dans un réseau pair-à-pair.

La réponse abstraite à cette question est que vous n'avez pas à vous en soucier. Un ordre des transactions accepté dans le monde entier sera sélectionné pour vous, résolvant ainsi le conflit. Les transactions seront regroupées dans ce que l'on appelle un "bloc", puis elles seront exécutées et réparties entre tous les nœuds participants.
Si deux transactions se contredisent, celle qui finit deuxième sera rejetée et ne fera pas partie du bloc.

Ces blocs forment une séquence linéaire dans le temps et c'est de là que vient le mot "blockchain". Des blocs sont ajoutés à la chaîne à des intervalles assez réguliers - pour Ethereum, c'est à peu près toutes les 17 secondes.

Dans le cadre du mécanisme de sélection d'ordre (qu'on appelle "mining"), il peut arriver que des blocs soient retournés de temps à autre, mais seulement au "sommet" de la chaîne. Plus il y a de blocs ajoutés au-dessus d'un bloc particulier, moins il y a de chances que ce bloc soit retourné. Il se peut donc que vos transactions soient annulées et même supprimées de la blockchain, mais plus vous attendez, moins il est probable qu'elles le soient.

.. note::
    Il n'est pas garanti que les transactions seront incluses dans le bloc suivant ou dans tout bloc futur spécifique, puisque ce n'est pas à l'auteur d'une transaction, mais aux mineurs de déterminer dans quel bloc la transaction est incluse.

    Si vous voulez programmer des appels futurs de votre contrat, vous pouvez utiliser le service 'alarm clock <http://www.ethereum-alarm-clock.com/>`_ ou un service oracle similaire.

.. _the-ethereum-virtual-machine:

.. index:: !evm, ! ethereum virtual machine

*****************************
La Machine Virtuelle Ethereum
*****************************

Définition
==========

La Machine Virtuelle Ethereum ou EVM est l'environnement d'exécution des contrats intelligents dans Ethereum. Il n'est pas seulement cloisonné, il est aussi complètement isolé, ce qui signifie que le code fonctionnant à l'intérieur de l'EVM n'a pas accès au réseau, au système de fichiers ou à d'autres processus.
Les Smart Contracts ont même un accès limité à d'autres Smart Contracts.

.. index:: ! account, address, storage, balance

Comptes
=======

Il y a deux types de comptes dans Ethereum qui partagent le même espace d'adresses : **Comptes externes** qui sont contrôlés par des paires de clés public-privé (c'est-à-dire des humains) et **comptes contrats** qui sont contrôlés par le code stocké avec le compte.

L'adresse d'un compte externe est déterminée à partir de la clé publique tandis que l'adresse d'un contrat est déterminée au moment de la création du contrat (elle est dérivée de l'adresse du créateur et du nombre de transactions envoyées à partir de cette adresse, ce qu'on appelle le "nonce").

Indépendamment du fait que le compte stocke ou non du code, les deux types sont traités de la même manière par l'EVM.

Chaque compte dispose d'une base de données persistante de clés-valeurs qui associe des mots de 256 bits à des mots de 256 bits appelée **storage**.

De plus, chaque compte a une **balance** en Ether (dans "Wei" pour être exact, `1 ether` est `10**18 wei`) qui peut être modifié en envoyant des transactions qui incluent des Ether.

.. index:: ! transaction

Transactions
============

Une transaction est un message envoyé d'un compte à un autre (qui peut être identique ou vide, voir ci-dessous).
Il peut inclure des données binaires (ce qu'on appelle charge utile ou "payload") et de l'éther.

Si le compte cible contient du code, ce code est exécuté et le payload est fourni comme données d'entrée.

Si le compte cible n'est pas défini (la transaction n'a pas de destinataire ou le destinataire est défini sur ``null``), la transaction crée un **nouveau contrat**.
Comme nous l'avons déjà mentionné, l'adresse de ce contrat n'est pas l'adresse zéro, mais une adresse dérivée de l'adresse de l'expéditeur et de
son nombre de transactions envoyées (le "nonce"). Le payload d'une telle transaction de création de contrat est considérée comme étant du bytecode EVM et exécuté. Les données de sortie de cette exécution sont stockées en permanence comme code du contrat.
Cela signifie que pour créer un contrat, vous n'envoyez pas le code réel du contrat, mais en fait un code qui retourne ce code lorsqu'il est exécuté.

.. note::
  Pendant la création d'un contrat, son code est toujours vide.
  Pour cette raison, vous ne devez pas rappeler le contrat en cours de construction tant que son constructeur n'a pas terminé son exécution.

.. index:: ! gas, ! gas price

Gas
===

Lors de la création, chaque transaction est facturée une certaine quantité de **gas**, dont le but est de limiter la quantité de travail nécessaire à l'exécution de la transaction et de payer pour cette exécution en même temps. Pendant que l'EVM exécute la commande
le gaz est progressivement épuisé selon des règles spécifiques.

Le **gas price** (prix du gas) est une valeur fixée par le créateur de la transaction, qui doit payer ``gas_price * gas`` à l'avance à partir du compte émetteur. S'il reste du gaz après l'exécution, il est remboursé au créateur de la même manière.

Si le gaz est épuisé à n'importe quel moment (c'est-à-dire qu'il serait négatif), une exception "à court de gas" est déclenchée, qui annule toutes les modifications apportées à l'état dans la trame d'appel en cours.

.. index:: ! storage, ! memory, ! stack

Storage, Memory et la Stack
===========================

La machine virtuelle Ethereum dispose de trois zones où elle peut stocker les données, stockage ("storage"), la mémoire ("memory") et la pile ("stack"), qui sont expliquées dans les paragraphes suivants.

Chaque compte possède une zone de données appelée **storage**, qui est persistante entre les appels de fonction et les transactions.
Storage est un stockage de valeur clé qui mappe les mots de 256 bits en 256 bits.
Il n'est pas possible d'énumérer storage à partir d'un contrat et il est comparativement coûteux à lire, et encore plus à modifier le storage.
Un contrat ne peut ni lire ni écrire dans un storage autre que le sien.

La deuxième zone de données est appelée **memory**, dont un contrat obtient une instance fraîchement rapprochée pour chaque appel de message. La mémoire est linéaire et peut être adressée au niveau de l'octet, mais les lectures sont limitées à une largeur de 256 bits, tandis que les écritures peuvent être de 8 bits ou de 256 bits. La mémoire est augmentée d'un mot (256 bits), lors de l'accès (en lecture ou en écriture) à un mot de mémoire qui n'a pas été touché auparavant (c.-à-d. tout décalage dans un mot). Au moment de l'agrandissement, le coût en gaz doit être payé. La mémoire est d'autant plus coûteuse qu'elle s'agrandit (le coût grandit de façon quadratique).

L'EVM n'est pas une machine à registre mais une machine à pile, donc tous les calculs sont effectués sur une zone de données appelée la **stack**. Elle a une taille maximale de 1024 éléments et contient des mots de 256 bits. L'accès à la stack est
limitée à l'extrémité supérieure de la façon suivante :
Il est possible de copier l'un des 16 éléments les plus hauts au sommet de la stack ou d'inverser
l'élément le plus en haut avec l'un des 16 éléments en dessous.
Toutes les autres opérations prennent les deux éléments les plus hauts (ou un, ou plus, selon l'opération) de la stack et poussent le résultat sur la stack.
Bien sûr, il est possible de déplacer les éléments de la pile vers le stockage ou la mémoire afin d'obtenir un accès plus profond à la stack,
mais il n'est pas possible d'accéder à des éléments arbitraires plus profondément dans la stack sans d'abord en enlever le haut.

.. index:: ! instruction

Jeu d'Instructions
==================

Le jeu d'instructions de l'EVM est maintenu au minimum afin d'éviter
des impl'ementations incorrectes ou incohérentes qui pourraient causer des problèmes de consensus.
Toutes les instructions fonctionnent sur le type de données de base, les mots de 256 bits ou sur des tranches de mémoire
(ou d'autres tableaux d'octets).
Les opérations arithmétiques, binaires, logiques et de comparaison habituelles sont présentes.
Des sauts conditionnels et inconditionnels sont possibles. En outre,
les contrats peuvent accéder aux propriétés pertinentes du bloc actuel
comme son numéro et son horodatage.

Pour une liste complète, veuillez consulter la liste :ref:` liste des opcodes <opcodes>` dans la documentation de l'insertion de langage assembleur.

.. index:: ! message call, function;call

Les Message Calls
=================

Les contrats peuvent appeler d'autres contrats ou envoyer des Ether sur des comptes non contractuels par le biais d'appels de messages ("message calls"). Les Message Calls sont similaires aux transactions, en ce sens qu'ils ont une source, une cible, une charge utile de données, d'éventuels Ether, le gas et le retour. En fait, chaque transaction consiste en un message call de niveau supérieur qui, à son tour, peut créer d'autres message calls.

Un contrat peut décider de la quantité de **gas** qu'il doit envoyer avec l'appel de message interne et de la quantité qu'il souhaite conserver.
Si une exception fin de gas se produit dans l'appel interne (ou toute autre exception), elle sera signalée par une valeur d'erreur placée sur la stack.
Dans ce cas, seul le gas envoyé avec l'appel est épuisé.
Dans Solidity, le contrat appelant provoque une exception manuelle par défaut dans de telles situations, de sorte que les exceptions "remontent en surface" de la pile d'appels.

Comme déjà dit, le contrat appelé (qui peut être le même que celui de l'appelant) recevra une instance de mémoire fraîchement effacée et aura accès à la charge utile de l'appel - qui sera fournie dans une zone séparée appelée **calldata**.
Une fois l'exécution terminée, il peut renvoyer des données qui seront stockées à un emplacement de la mémoire de l'appelant pré-alloué par ce dernier.
Tous ces appels sont entièrement synchrones.

Les appels sont **limités** à une profondeur de 1024, ce qui signifie que pour les opérations plus complexes, les boucles doivent être préférées aux appels récursifs. De plus, seul 63/64ème du gaz peut être transféré lors d'un appel de message, ce qui entraîne une limite de profondeur d'un peu moins de 1000 en pratique.

.. index:: delegatecall, callcode, library

Delegatecall / Callcode et Libraries
=====================================

Il existe une variante spéciale d'un message call, appelée **delegatecall**, qui est identique à un appel de message sauf que le code à l'adresse cible est exécuté dans le cadre du contrat d'appel et que ``msg.sender`` et ``msg.value`` ne changent pas leurs valeurs.

Cela signifie qu'un contrat peut charger dynamiquement du code à partir d'une adresse différente lors de l'exécution. Le stockage, l'adresse actuelle et le solde se réfèrent toujours au contrat d'appel, seul le code est repris de l'adresse appelée.

Cela permet d'implémenter la fonctionnalité "bibliothèque" dans Solidity :
Code de bibliothèque réutilisable qui peut être appliqué au stockage d'un contrat, par exemple pour implémenter une structure de données complexe.

.. index:: log

Logs / Journalisation
=====================

Il est possible de stocker les données dans une structure de données spécialement indexée qui s'étend jusqu'au niveau du bloc. Cette fonction appelée **logs** (journalisation) est utilisé par Solidity pour implémenter les :ref:`events <events>`.
Les contrats ne peuvent pas accéder aux données du journal une fois qu'elles ont été créées, mais ils peut être accédé efficacement de l'extérieur de la chaîne de blocs.
Puisqu'une partie des données du journal est stockée dans des `bloom filters <https://en.wikipedia.org/wiki/Bloom_filter>`_, il est possible de rechercher ces données de manière efficace et cryptographique de manière sécurisée, afin que les pairs du réseau qui ne téléchargent pas la totalité de la blockchain (appelés "clients légers") peuvent encore trouver ces logs.

.. index:: contract creation

Création
========

Les contrats peuvent même créer d'autres contrats à l'aide d'un opcode spécial (càd qu'ils n'appellent pas simplement l'adresse zéro comme le ferait une transaction). La seule différence entre ces **appels de création** et des appels de message normaux est que les données de charge utile sont
exécutées, le résultat stocké sous forme de code et l'appelant / créateur
reçoit l'adresse du nouveau contrat sur la stack.

.. index:: selfdestruct, self-destruct, deactivate

Désactivation et Auto-Destruction
=================================

La seule façon de supprimer du code de la blockchain est lorsqu'un contrat à cette adresse exécute l'opération d'autodestruction ``selfdestruct``. L'Ether restant stocké à cette adresse est envoyé à une cible désignée, puis le stockage et le code sont retirés de l'état. Supprimer le contrat en théorie semble être une bonne idée, mais c'est potentiellement dangereux, comme en cas d'envoi d'éther à des contrats supprimés, où l'éther est perdu à jamais.

.. note::
    Même si le code d'un contrat ne contient pas d'appel à ``selfdestruct``, il peut toujours effectuer cette opération en utilisant le ``delegate code`` ou le ``callcode``.

Si vous souhaitez désactiver vos contrats, vous devez plutôt les désactiver en changeant un état interne qui provoque un échec (``revert``) de toutes les fonctions. Il est donc impossible d'utiliser le contrat, car il retourne immédiatement l'éther.

.. warning::
    Même si un contrat est supprimé par ``selfdestruct``, il fait toujours partie de l'historique de la blockchain et probablement conservé par la plupart des nœuds Ethereum. L'utilisation de l'autodestruction n'est donc pas la même chose que la suppression de données d'un disque dur.
