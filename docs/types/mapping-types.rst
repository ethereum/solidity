.. index:: !mapping
.. _mapping-types:

Mapping Types
=============

Mapping types use the syntax ``mapping(KeyType => ValueType)`` and variables
of mapping type are declared using the syntax ``mapping(KeyType => ValueType) VariableName``.
The ``KeyType`` can be any
built-in value type, ``bytes``, ``string``, or any contract or enum type. Other user-defined
or complex types, such as mappings, structs or array types are not allowed.
``ValueType`` can be any type, including mappings, arrays and structs.

You can think of mappings as `hash tables <https://en.wikipedia.org/wiki/Hash_table>`_, which are virtually initialised
such that every possible key exists and is mapped to a value whose
byte-representation is all zeros, a type's :ref:`default value <default-value>`.
The similarity ends there, the key data is not stored in a
mapping, only its ``keccak256`` hash is used to look up the value.

Because of this, mappings do not have a length or a concept of a key or
value being set, and therefore cannot be erased without extra information
regarding the assigned keys (see :ref:`clearing-mappings`).

Mappings can only have a data location of ``storage`` and thus
are allowed for state variables, as storage reference types
in functions, or as parameters for library functions.
They cannot be used as parameters or return parameters
of contract functions that are publicly visible.
These restrictions are also true for arrays and structs that contain mappings.

You can mark state variables of mapping type as ``public`` and Solidity creates a
:ref:`getter <visibility-and-getters>` for you. The ``KeyType`` becomes a parameter for the getter.
If ``ValueType`` is a value type or a struct, the getter returns ``ValueType``.
If ``ValueType`` is an array or a mapping, the getter has one parameter for
each ``KeyType``, recursively.

In the example below, the ``MappingExample`` contract defines a public ``balances``
mapping, with the key type an ``address``, and a value type a ``uint``, mapping
an Ethereum address to an unsigned integer value. As ``uint`` is a value type, the getter
returns a value that matches the type, which you can see in the ``MappingUser``
contract that returns the value at the specified address.

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.4.0 <0.9.0;

    contract MappingExample {
        mapping(address => uint) public balances;

        function update(uint newBalance) public {
            balances[msg.sender] = newBalance;
        }
    }

    contract MappingUser {
        function f() public returns (uint) {
            MappingExample m = new MappingExample();
            m.update(100);
            return m.balances(address(this));
        }
    }

The example below is a simplified version of an
`ERC20 token <https://github.com/OpenZeppelin/openzeppelin-contracts/blob/master/contracts/token/ERC20/ERC20.sol>`_.
``_allowances`` is an example of a mapping type inside another mapping type.
The example below uses ``_allowances`` to record the amount someone else is allowed to withdraw from your account.

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.4.22 <0.9.0;

    contract MappingExample {

        mapping (address => uint256) private _balances;
        mapping (address => mapping (address => uint256)) private _allowances;

        event Transfer(address indexed from, address indexed to, uint256 value);
        event Approval(address indexed owner, address indexed spender, uint256 value);

        function allowance(address owner, address spender) public view returns (uint256) {
            return _allowances[owner][spender];
        }

        function transferFrom(address sender, address recipient, uint256 amount) public returns (bool) {
            require(_allowances[sender][msg.sender] >= amount, "ERC20: Allowance not high enough.");
            _allowances[sender][msg.sender] -= amount;
            _transfer(sender, recipient, amount);
            return true;
        }

        function approve(address spender, uint256 amount) public returns (bool) {
            require(spender != address(0), "ERC20: approve to the zero address");

            _allowances[msg.sender][spender] = amount;
            emit Approval(msg.sender, spender, amount);
            return true;
        }

        function _transfer(address sender, address recipient, uint256 amount) internal {
            require(sender != address(0), "ERC20: transfer from the zero address");
            require(recipient != address(0), "ERC20: transfer to the zero address");
            require(_balances[sender] >= amount, "ERC20: Not enough funds.");

            _balances[sender] -= amount;
            _balances[recipient] += amount;
            emit Transfer(sender, recipient, amount);
        }
    }


.. index:: !iterable mappings
.. _iterable-mappings:

Iterable Mappings
-----------------

You cannot iterate over mappings, i.e. you cannot enumerate their keys.
It is possible, though, to implement a data structure on
top of them and iterate over that. For example, the code below implements an
``IterableMapping`` library that the ``User`` contract then adds data too, and
the ``sum`` function iterates over to sum all the values.

.. code-block:: solidity
    :force:

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity ^0.8.8;

    struct IndexValue { uint keyIndex; uint value; }
    struct KeyFlag { uint key; bool deleted; }

    struct itmap {
        mapping(uint => IndexValue) data;
        KeyFlag[] keys;
        uint size;
    }

    type Iterator is uint;

    library IterableMapping {
        function insert(itmap storage self, uint key, uint value) internal returns (bool replaced) {
            uint keyIndex = self.data[key].keyIndex;
            self.data[key].value = value;
            if (keyIndex > 0)
                return true;
            else {
                keyIndex = self.keys.length;
                self.keys.push();
                self.data[key].keyIndex = keyIndex + 1;
                self.keys[keyIndex].key = key;
                self.size++;
                return false;
            }
        }

        function remove(itmap storage self, uint key) internal returns (bool success) {
            uint keyIndex = self.data[key].keyIndex;
            if (keyIndex == 0)
                return false;
            delete self.data[key];
            self.keys[keyIndex - 1].deleted = true;
            self.size --;
        }

        function contains(itmap storage self, uint key) internal view returns (bool) {
            return self.data[key].keyIndex > 0;
        }

        function iterateStart(itmap storage self) internal view returns (Iterator) {
            return iteratorSkipDeleted(self, 0);
        }

        function iterateValid(itmap storage self, Iterator iterator) internal view returns (bool) {
            return Iterator.unwrap(iterator) < self.keys.length;
        }

        function iterateNext(itmap storage self, Iterator iterator) internal view returns (Iterator) {
            return iteratorSkipDeleted(self, Iterator.unwrap(iterator) + 1);
        }

        function iterateGet(itmap storage self, Iterator iterator) internal view returns (uint key, uint value) {
            uint keyIndex = Iterator.unwrap(iterator);
            key = self.keys[keyIndex].key;
            value = self.data[key].value;
        }

        function iteratorSkipDeleted(itmap storage self, uint keyIndex) private view returns (Iterator) {
            while (keyIndex < self.keys.length && self.keys[keyIndex].deleted)
                keyIndex++;
            return Iterator.wrap(keyIndex);
        }
    }

    // How to use it
    contract User {
        // Just a struct holding our data.
        itmap data;
        // Apply library functions to the data type.
        using IterableMapping for itmap;

        // Insert something
        function insert(uint k, uint v) public returns (uint size) {
            // This calls IterableMapping.insert(data, k, v)
            data.insert(k, v);
            // We can still access members of the struct,
            // but we should take care not to mess with them.
            return data.size;
        }

        // Computes the sum of all stored data.
        function sum() public view returns (uint s) {
            for (
                Iterator i = data.iterateStart();
                data.iterateValid(i);
                i = data.iterateNext(i)
            ) {
                (, uint value) = data.iterateGet(i);
                s += value;
            }
        }
    }
