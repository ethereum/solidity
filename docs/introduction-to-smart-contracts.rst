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

... note::
    Tous les identifiants (noms de contrat, noms de fonctions et noms de variables) sont limités au jeu de caractères ASCII. Il est possible de stocker des données encodées en UTF-8 dans des variables de type string.

..warning::
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

Blockchains as a concept are not too hard to understand for programmers. The reason is that
most of the complications (mining, `hashing <https://en.wikipedia.org/wiki/Cryptographic_hash_function>`_, `elliptic-curve cryptography <https://en.wikipedia.org/wiki/Elliptic_curve_cryptography>`_, `peer-to-peer networks <https://en.wikipedia.org/wiki/Peer-to-peer>`_, etc.)
are just there to provide a certain set of features and promises for the platform. Once you accept these
features as given, you do not have to worry about the underlying technology - or do you have
to know how Amazon's AWS works internally in order to use it?

.. index:: transaction

Transactions
============

A blockchain is a globally shared, transactional database.
This means that everyone can read entries in the database just by participating in the network.
If you want to change something in the database, you have to create a so-called transaction
which has to be accepted by all others.
The word transaction implies that the change you want to make (assume you want to change
two values at the same time) is either not done at all or completely applied. Furthermore,
while your transaction is being applied to the database, no other transaction can alter it.

As an example, imagine a table that lists the balances of all accounts in an
electronic currency. If a transfer from one account to another is requested,
the transactional nature of the database ensures that if the amount is
subtracted from one account, it is always added to the other account. If due
to whatever reason, adding the amount to the target account is not possible,
the source account is also not modified.

Furthermore, a transaction is always cryptographically signed by the sender (creator).
This makes it straightforward to guard access to specific modifications of the
database. In the example of the electronic currency, a simple check ensures that
only the person holding the keys to the account can transfer money from it.

.. index:: ! block

Blocks
======

One major obstacle to overcome is what (in Bitcoin terms) is called a "double-spend attack":
What happens if two transactions exist in the network that both want to empty an account?
Only one of the transactions can be valid, typically the one that is accepted first.
The problem is that "first" is not an objective term in a peer-to-peer network.

The abstract answer to this is that you do not have to care. A globally accepted order of the transactions
will be selected for you, solving the conflict. The transactions will be bundled into what is called a "block"
and then they will be executed and distributed among all participating nodes.
If two transactions contradict each other, the one that ends up being second will
be rejected and not become part of the block.

These blocks form a linear sequence in time and that is where the word "blockchain"
derives from. Blocks are added to the chain in rather regular intervals - for
Ethereum this is roughly every 17 seconds.

As part of the "order selection mechanism" (which is called "mining") it may happen that
blocks are reverted from time to time, but only at the "tip" of the chain. The more
blocks are added on top of a particular block, the less likely this block will be reverted. So it might be that your transactions
are reverted and even removed from the blockchain, but the longer you wait, the less
likely it will be.

.. note::
    Transactions are not guaranteed to be included in the next block or any specific future block,
    since it is not up to the submitter of a transaction, but up to the miners to determine in which block the transaction is included.

    If you want to schedule future calls of your contract, you can use
    the `alarm clock <http://www.ethereum-alarm-clock.com/>`_ or a similar oracle service.

.. _the-ethereum-virtual-machine:

.. index:: !evm, ! ethereum virtual machine

****************************
The Ethereum Virtual Machine
****************************

Overview
========

The Ethereum Virtual Machine or EVM is the runtime environment
for smart contracts in Ethereum. It is not only sandboxed but
actually completely isolated, which means that code running
inside the EVM has no access to network, filesystem or other processes.
Smart contracts even have limited access to other smart contracts.

.. index:: ! account, address, storage, balance

Accounts
========

There are two kinds of accounts in Ethereum which share the same
address space: **External accounts** that are controlled by
public-private key pairs (i.e. humans) and **contract accounts** which are
controlled by the code stored together with the account.

The address of an external account is determined from
the public key while the address of a contract is
determined at the time the contract is created
(it is derived from the creator address and the number
of transactions sent from that address, the so-called "nonce").

Regardless of whether or not the account stores code, the two types are
treated equally by the EVM.

Every account has a persistent key-value store mapping 256-bit words to 256-bit
words called **storage**.

Furthermore, every account has a **balance** in
Ether (in "Wei" to be exact, `1 ether` is `10**18 wei`) which can be modified by sending transactions that
include Ether.

.. index:: ! transaction

Transactions
============

A transaction is a message that is sent from one account to another
account (which might be the same or empty, see below).
It can include binary data (which is called "payload") and Ether.

If the target account contains code, that code is executed and
the payload is provided as input data.

If the target account is not set (the transaction does not have
a recipient or the recipient is set to ``null``), the transaction
creates a **new contract**.
As already mentioned, the address of that contract is not
the zero address but an address derived from the sender and
its number of transactions sent (the "nonce"). The payload
of such a contract creation transaction is taken to be
EVM bytecode and executed. The output data of this execution is
permanently stored as the code of the contract.
This means that in order to create a contract, you do not
send the actual code of the contract, but in fact code that
returns that code when executed.

.. note::
  While a contract is being created, its code is still empty.
  Because of that, you should not call back into the
  contract under construction until its constructor has
  finished executing.

.. index:: ! gas, ! gas price

Gas
===

Upon creation, each transaction is charged with a certain amount of **gas**,
whose purpose is to limit the amount of work that is needed to execute
the transaction and to pay for this execution at the same time. While the EVM executes the
transaction, the gas is gradually depleted according to specific rules.

The **gas price** is a value set by the creator of the transaction, who
has to pay ``gas_price * gas`` up front from the sending account.
If some gas is left after the execution, it is refunded to the creator in the same way.

If the gas is used up at any point (i.e. it would be negative),
an out-of-gas exception is triggered, which reverts all modifications
made to the state in the current call frame.

.. index:: ! storage, ! memory, ! stack

Storage, Memory and the Stack
=============================

The Ethereum Virtual Machine has three areas where it can store data,
storage, memory and the stack, which are explained in the following
paragraphs.

Each account has a data area called **storage**, which is persistent between function calls
and transactions.
Storage is a key-value store that maps 256-bit words to 256-bit words.
It is not possible to enumerate storage from within a contract and it is
comparatively costly to read, and even more to modify storage.
A contract can neither read nor write to any storage apart from its own.

The second data area is called **memory**, of which a contract obtains
a freshly cleared instance for each message call. Memory is linear and can be
addressed at byte level, but reads are limited to a width of 256 bits, while writes
can be either 8 bits or 256 bits wide. Memory is expanded by a word (256-bit), when
accessing (either reading or writing) a previously untouched memory word (i.e. any offset
within a word). At the time of expansion, the cost in gas must be paid. Memory is more
costly the larger it grows (it scales quadratically).

The EVM is not a register machine but a stack machine, so all
computations are performed on an data area called the **stack**. It has a maximum size of
1024 elements and contains words of 256 bits. Access to the stack is
limited to the top end in the following way:
It is possible to copy one of
the topmost 16 elements to the top of the stack or swap the
topmost element with one of the 16 elements below it.
All other operations take the topmost two (or one, or more, depending on
the operation) elements from the stack and push the result onto the stack.
Of course it is possible to move stack elements to storage or memory
in order to get deeper access to the stack,
but it is not possible to just access arbitrary elements deeper in the stack
without first removing the top of the stack.

.. index:: ! instruction

Instruction Set
===============

The instruction set of the EVM is kept minimal in order to avoid
incorrect or inconsistent implementations which could cause consensus problems.
All instructions operate on the basic data type, 256-bit words or on slices of memory
(or other byte arrays).
The usual arithmetic, bit, logical and comparison operations are present.
Conditional and unconditional jumps are possible. Furthermore,
contracts can access relevant properties of the current block
like its number and timestamp.

For a complete list, please see the :ref:`list of opcodes <opcodes>` as part of the inline
assembly documentation.

.. index:: ! message call, function;call

Message Calls
=============

Contracts can call other contracts or send Ether to non-contract
accounts by the means of message calls. Message calls are similar
to transactions, in that they have a source, a target, data payload,
Ether, gas and return data. In fact, every transaction consists of
a top-level message call which in turn can create further message calls.

A contract can decide how much of its remaining **gas** should be sent
with the inner message call and how much it wants to retain.
If an out-of-gas exception happens in the inner call (or any
other exception), this will be signaled by an error value put onto the stack.
In this case, only the gas sent together with the call is used up.
In Solidity, the calling contract causes a manual exception by default in
such situations, so that exceptions "bubble up" the call stack.

As already said, the called contract (which can be the same as the caller)
will receive a freshly cleared instance of memory and has access to the
call payload - which will be provided in a separate area called the **calldata**.
After it has finished execution, it can return data which will be stored at
a location in the caller's memory preallocated by the caller.
All such calls are fully synchronous.

Calls are **limited** to a depth of 1024, which means that for more complex
operations, loops should be preferred over recursive calls. Furthermore,
only 63/64th of the gas can be forwarded in a message call, which causes a
depth limit of a little less than 1000 in practice.

.. index:: delegatecall, callcode, library

Delegatecall / Callcode and Libraries
=====================================

There exists a special variant of a message call, named **delegatecall**
which is identical to a message call apart from the fact that
the code at the target address is executed in the context of the calling
contract and ``msg.sender`` and ``msg.value`` do not change their values.

This means that a contract can dynamically load code from a different
address at runtime. Storage, current address and balance still
refer to the calling contract, only the code is taken from the called address.

This makes it possible to implement the "library" feature in Solidity:
Reusable library code that can be applied to a contract's storage, e.g. in
order to implement a complex data structure.

.. index:: log

Logs
====

It is possible to store data in a specially indexed data structure
that maps all the way up to the block level. This feature called **logs**
is used by Solidity in order to implement :ref:`events <events>`.
Contracts cannot access log data after it has been created, but they
can be efficiently accessed from outside the blockchain.
Since some part of the log data is stored in `bloom filters <https://en.wikipedia.org/wiki/Bloom_filter>`_, it is
possible to search for this data in an efficient and cryptographically
secure way, so network peers that do not download the whole blockchain
(so-called "light clients") can still find these logs.

.. index:: contract creation

Create
======

Contracts can even create other contracts using a special opcode (i.e.
they do not simply call the zero address as a transaction would). The only difference between
these **create calls** and normal message calls is that the payload data is
executed and the result stored as code and the caller / creator
receives the address of the new contract on the stack.

.. index:: selfdestruct, self-destruct, deactivate

Deactivate and Self-destruct
============================

The only way to remove code from the blockchain is when a contract at that address performs the ``selfdestruct`` operation. The remaining Ether stored at that address is sent to a designated target and then the storage and code is removed from the state. Removing the contract in theory sounds like a good idea, but it is potentially dangerous, as if someone sends Ether to removed contracts, the Ether is forever lost.

.. note::
    Even if a contract's code does not contain a call to ``selfdestruct``, it can still perform that operation using ``delegatecall`` or ``callcode``.

If you want to deactivate your contracts, you should instead **disable** them by changing some internal state which causes all functions to revert. This makes it impossible to use the contract, as it returns Ether immediately.

.. warning::
    Even if a contract is removed by "selfdestruct", it is still part of the history of the blockchain and probably retained by most Ethereum nodes. So using "selfdestruct" is not the same as deleting data from a hard disk.
