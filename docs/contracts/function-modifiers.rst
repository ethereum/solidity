.. index:: ! function;modifier

.. _modifiers:

**************************
Modificateurs de fonctions
**************************

Les modificateurs peuvent être utilisés pour modifier facilement le comportement des fonctions.  Par exemple, ils peuvent vérifier automatiquement une condition avant d'exécuter la fonction.

Les modificateurs sont des propriétés héritables des contrats et peuvent être redéfinis dans les contrats dérivés, mais seulement s'ils sont indiqués ``virtual``. Pour plus de détails, voir
:ref:`Modifier Overriding <modifier-overriding>`.

::

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.5.0 <0.7.0;

    contract owned {
        constructor() public { owner = msg.sender; }
        address payable owner;

        // Ce contrat ne définit qu'un modificateur mais ne l'utilise pas:
        // il sera utilisé dans les contrats dérivés.
        // Le corps de la fonction est inséré à l'endroit où le symbole spécial
        // `_;` apparaît dans la définition d'un modificateur.
        // Cela signifie que si le propriétaire appelle cette fonction, la fonction
        // est exécutée et dans le cas contraire, une exception est
        // levée.
        modifier onlyOwner {
            require(
                msg.sender == owner,
                "Only owner can call this function."
            );
            _;
        }
    }

    contract mortal is owned {
        // Ce contrat hérite du modificateur `onlyOwner` de `owned`
        // et l'applique à la fonction `close`, qui
        // cause que les appels à `close` n'ont un effet que s'il
        // sont passés par le propriétaire enregistré.
        function close() public onlyOwner {
            selfdestruct(owner);
        }
    }

    contract priced {
        // Les modificateurs peuvent prendre des arguments:
        modifier costs(uint price) {
            if (msg.value >= price) {
                _;
            }
        }
    }

    contract Register is priced, owned {
        mapping (address => bool) registeredAddresses;
        uint price;

        constructor(uint initialPrice) public { price = initialPrice; }

        // Il est important de fournir également le
        // mot-clé `payable` ici, sinon la fonction
        // rejettera automatiquement tous les Ethers qui lui sont envoyés.
        function register() public payable costs(price) {
            registeredAddresses[msg.sender] = true;
        }

        function changePrice(uint _price) public onlyOwner {
            price = _price;
        }
    }

    contract Mutex {
        bool locked;
        modifier noReentrancy() {
            require(
                !locked,
                "Reentrant call."
            );
            locked = true;
            _;
            locked = false;
        }

        /// Cette fonction est protégée par un mutex, ce qui signifie que
        /// les appels entrants à partir de `msg.sender.call` ne peuvent pas rappeler `f`.
        /// L'instruction `return 7` assigne 7 à la valeur de retour, mais en même temps
        /// exécute l'instruction `locked = false` dans le modificateur.
        function f() public noReentrancy returns (uint) {
            (bool success,) = msg.sender.call("");
            require(success);
            return 7;
        }
    }

Plusieurs modificateurs sont appliqués à une fonction en les spécifiant dans une liste séparée par des espaces et sont évalués dans l'ordre présenté.

.. warning::
    Dans une version antérieure de Solidity, les instructions ``return`` des fonctions ayant des modificateurs se comportaient différemment.

Les retours explicites d'un modificateur ou d'un corps de fonction ne laissent que le modificateur ou le corps de fonction courant. Les variables de retour sont affectées et le flow de contrôle continue après le "_" dans le modificateur précédent.

Des expressions arbitraires sont autorisées pour les arguments du modificateur et dans ce contexte, tous les symboles visibles depuis la fonction sont visibles dans le modificateur. Les symboles introduits dans le modificateur ne sont pas visibles dans la fonction (car ils peuvent changer en cas de redéfinition).