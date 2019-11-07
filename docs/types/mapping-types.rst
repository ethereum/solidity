.. index:: !mapping
.. _mapping-types:

Mapping Types
=============

Mapping types use the syntax ``mapping(_KeyType => _ValueType)`` and variables
are declared as a mapping type using the syntax ``mapping (_KeyType => _ValueType) _VariableModifiers _VariableName``.
The ``_KeyType`` can be any elementary type. This means it can be any of
the built-in value types plus ``bytes`` and ``string``. User-defined
or complex types like contract types, enums, mappings, structs and any array type
apart from ``bytes`` and ``string`` are not allowed.
``_ValueType`` can be any type, including mappings.

You can think of mappings as `hash tables <https://en.wikipedia.org/wiki/Hash_table>`_, which are virtually initialised
such that every possible key exists and is mapped to a value whose
byte-representation is all zeros, a type's :ref:`default value <default-value>`. The similarity ends there, the key data is not stored in a
mapping, only its ``keccak256`` hash is used to look up the value.

Because of this, mappings do not have a length or a concept of a key or
value being set, and therefore cannot be erased without extra information
regarding the assigned keys (see :ref:`clearing-mappings`).

Mappings can only have a data location of ``storage`` and thus
are allowed for state variables, as storage reference types
in functions, or as parameters for library functions.
They cannot be used as parameters or return parameters
of contract functions that are publicly visible.

You can mark state variables of mapping type as ``public`` and Solidity creates a
:ref:`getter <visibility-and-getters>` for you. The ``_KeyType`` becomes a parameter for the getter.
If ``_ValueType`` is a value type or a struct, the getter returns ``_ValueType``.
If ``_ValueType`` is an array or a mapping, the getter has one parameter for
each ``_KeyType``, recursively.

In the example below, the ``MappingExample`` contract defines a public ``balances``
mapping, with the key type an ``address``, and a value type a ``uint``, mapping
an Ethereum address to an unsigned integer value. As ``uint`` is a value type, the getter
returns a value that matches the type, which you can see in the ``MappingUser``
contract that returns the value at the specified address.

::

    pragma solidity >=0.4.0 <0.7.0;

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

The example below is a simplified version of an `ERC20 token <https://github.com/OpenZeppelin/openzeppelin-contracts/blob/master/contracts/token/ERC20/ERC20.sol>`_.
``_allowances`` is an example of a mapping type inside another mapping type.
The example below uses ``_allowances`` to record the amount someone else is allowed to withdraw from your account.

::

    pragma solidity >=0.4.0 <0.7.0;

    contract MappingExample {

        mapping (address => uint256) private _balances;
        mapping (address => mapping (address => uint256)) private _allowances;

        event Transfer(address indexed from, address indexed to, uint256 value);
        event Approval(address indexed owner, address indexed spender, uint256 value);

        function allowance(address owner, address spender) public view returns (uint256) {
            return _allowances[owner][spender];
        }

        function transferFrom(address sender, address recipient, uint256 amount) public returns (bool) {
            _transfer(sender, recipient, amount);
            approve(sender, msg.sender, amount);
            return true;
        }

        function approve(address owner, address spender, uint256 amount) public returns (bool) {
            require(owner != address(0), "ERC20: approve from the zero address");
            require(spender != address(0), "ERC20: approve to the zero address");

            _allowances[owner][spender] = amount;
            emit Approval(owner, spender, amount);
            return true;
        }

        function _transfer(address sender, address recipient, uint256 amount) internal {
            require(sender != address(0), "ERC20: transfer from the zero address");
            require(recipient != address(0), "ERC20: transfer to the zero address");

            _balances[sender] -= amount;
            _balances[recipient] += amount;
            emit Transfer(sender, recipient, amount);
        }
    }


.. index:: !iterable mappings
.. _iterable-mappings:

Iterable Mappings
-----------------

Mappings are not iterable, but it is possible to implement a data structure on
top of them and iterate over that. For example, the code below implements an
``IterableMapping`` library that the ``User`` contract then adds data too, and
the ``sum`` function iterates over to sum all the values.

::

    pragma solidity >=0.4.0 <0.7.0;

    library IterableMapping {

        struct itmap {
            mapping(uint => IndexValue) data;
            KeyFlag[] keys;
            uint size;
        }

        struct IndexValue { uint keyIndex; uint value; }
        struct KeyFlag { uint key; bool deleted; }

        function insert(itmap storage self, uint key, uint value) internal returns (bool replaced) {
            uint keyIndex = self.data[key].keyIndex;
            self.data[key].value = value;
            if (keyIndex > 0)
                return true;
            else {
                self.keys.push();
                keyIndex = self.keys.length;
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

        function iterate_start(itmap storage self) internal view returns (uint keyIndex) {
            return iterate_next(self, uint(-1));
        }

        function iterate_valid(itmap storage self, uint keyIndex) internal view returns (bool) {
            return keyIndex < self.keys.length;
        }

        function iterate_next(itmap storage self, uint keyIndex) internal view returns (uint r_keyIndex) {
            keyIndex++;
            while (keyIndex < self.keys.length && self.keys[keyIndex].deleted)
                keyIndex++;
            return keyIndex;
        }

        function iterate_get(itmap storage self, uint keyIndex) internal view returns (uint key, uint value) {
            key = self.keys[keyIndex].key;
            value = self.data[key].value;
        }
    }

    // How to use it
    contract User {
        // Just a struct holding our data.
        IterableMapping.itmap data;

        // Insert something
        function insert(uint k, uint v) public returns (uint size) {
            // Actually calls itmap_impl.insert, auto-supplying the first parameter for us.
            IterableMapping.insert(data, k, v);
            // We can still access members of the struct - but we should take care not to mess with them.
            return data.size;
        }

        // Computes the sum of all stored data.
        function sum() public view returns (uint s) {
            for (uint i = IterableMapping.iterate_start(data);
            IterableMapping.iterate_valid(data, i);
            i = IterableMapping.iterate_next(data, i)) {
                (, uint value) = IterableMapping.iterate_get(data, i);
                s += value;
            }
        }
    }