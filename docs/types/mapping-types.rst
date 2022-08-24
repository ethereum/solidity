.. index:: !mapping
.. _mapping-types:

映射类型
=============

映射类型使用语法 ``mapping(_KeyType => _ValueType)``，
映射类型的变量使用语法 ``mapping(_KeyType => _ValueType) _VariableName`` 声明。
``_KeyType`` 可以是任何内置的值类型， ``bytes``， ``string``，或任何合约或枚举类型。
其他用户定义的或复杂的类型，如映射、结构体或数组类型是不允许的。
``_ValueType`` 可以是任何类型，包括映射、数组和结构体。

您可以把映射想象成 `哈希表 <https://en.wikipedia.org/wiki/Hash_table>`_，
它实际上被初始化了，使每一个可能的键都存在，
并将其映射到字节形式全是零的值，一个类型的 :ref:`默认值 <default-value>`。
相似性到此为止，键数据不存储在映射中，而是它的 ``keccak256`` 哈希值被用来查询。

正因为如此，映射没有长度，也没有被设置的键或值的概念，
因此，如果没有关于分配的键的额外信息，就不能被删除（见 :ref:`clearing-mappings`）。

映射只能有一个 ``storage`` 的数据位置，因此允许用于状态变量，
可作为函数中的存储引用类型，或作为库函数的参数。
但它们不能被用作公开可见的合约函数的参数或返回参数。
这些限制对于包含映射的数组和结构也是如此。

您可以把映射类型的状态变量标记为 ``public``，
Solidity会为您创建一个 :ref:`getter <visibility-and-getters>` 函数。
``_KeyType`` 将成为getter的参数。
如果 ``_ValueType`` 是一个值类型或一个结构，getter返回 ``_ValueType``。
如果 ``_ValueType`` 是一个数组或映射，getter对每个 ``_KeyType`` 递归出一个参数。

在下面的例子中， ``MappingExample`` 合约定义了一个公共的 ``balances`` 映射，
键类型是 ``address``，值类型是 ``uint``，将一个Ethereum地址映射到一个无符号整数值。
由于 ``uint`` 是一个值类型，getter 返回一个与该类型相匹配的值，
您可以在 ``MappingUser`` 合约中看到它返回指定地址对应的值。

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

下面的例子是一个简化版本的
`ERC20 代币 <https://github.com/OpenZeppelin/openzeppelin-contracts/blob/master/contracts/token/ERC20/ERC20.sol>`_。
``_allowances`` 是一个映射类型在另一个映射类型中的例子。
下面的例子使用 ``_allowances`` 来记录别人允许从您的账户中提取的金额。


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

递归映射
-----------------

您不能对映射进行递归调用，也就是说，您不能列举它们的键。
不过，可以在它们上层实现一个数据结构，并对其进行递归。例如，
下面的代码实现了一个 ``IterableMapping`` 库， ``User`` 合约也添加了数据，
``sum`` 函数对所有的值进行递归调用去累加这些值。

.. code-block:: solidity
    :force:

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.6.8 <0.9.0;

    struct IndexValue { uint keyIndex; uint value; }
    struct KeyFlag { uint key; bool deleted; }

    struct itmap {
        mapping(uint => IndexValue) data;
        KeyFlag[] keys;
        uint size;
    }

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

        function iterateStart(itmap storage self) internal view returns (uint keyIndex) {
            return iterateNext(self, type(uint).max);
        }

        function iterateValid(itmap storage self, uint keyIndex) internal view returns (bool) {
            return keyIndex < self.keys.length;
        }

        function iterateNext(itmap storage self, uint keyIndex) internal view returns (uint r_keyIndex) {
            keyIndex++;
            while (keyIndex < self.keys.length && self.keys[keyIndex].deleted)
                keyIndex++;
            return keyIndex;
        }

        function iterateGet(itmap storage self, uint keyIndex) internal view returns (uint key, uint value) {
            key = self.keys[keyIndex].key;
            value = self.data[key].value;
        }
    }

    // 如何使用
    contract User {
        // 只是一个保存我们数据的结构体。
        itmap data;
        // 对数据类型应用库函数。
        using IterableMapping for itmap;

        // 插入一些数据
        function insert(uint k, uint v) public returns (uint size) {
            // 这将调用 IterableMapping.insert(data, k, v)
            data.insert(k, v);
            // 我们仍然可以访问结构中的成员，
            // 但我们应该注意不要乱动他们。
            return data.size;
        }

        // 计算所有存储数据的总和。
        function sum() public view returns (uint s) {
            for (
                uint i = data.iterateStart();
                data.iterateValid(i);
                i = data.iterateNext(i)
            ) {
                (, uint value) = data.iterateGet(i);
                s += value;
            }
        }
    }
