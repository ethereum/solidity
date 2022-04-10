.. index:: ! type;reference, ! reference type, storage, memory, location, array, struct

.. _reference-types:

引用类型
==========

引用类型的值可以通过多个不同的名称进行修改。
这与值类型形成鲜明对比，在值类型的变量被使用时，您会得到一个独立的副本。
正因为如此，对引用类型的处理要比对值类型的处理更加谨慎。目前，
引用类型包括结构、数组和映射。如果您使用一个引用类型，
您必须明确地提供存储该类型的数据区域。 ``memory`` （其寿命限于外部函数调用），
``storage`` （存储状态变量的位置，其寿命限于合约的寿命）
或 ``calldata`` （包含函数参数的特殊数据位置）。

改变数据位置的赋值或类型转换将总是导致自动复制操作，
而同一数据位置内的赋值只在某些情况下对存储类型进行复制。

.. _data-location:

数据位置
----------

每个引用类型都有一个额外的属性，即 "数据位置"，
关于它的存储位置。有三个数据位置。 ``memory``, ``storage`` 和 ``calldata``。
Calldata是一个不可修改的、非持久性的区域，用于存储函数参数，其行为主要类似于memory。

.. 注解::
    如果可以的话，尽量使用 ``calldata`` 作为数据位置，因为这样可以避免复制，
    也可以确保数据不能被修改。使用 ``calldata`` 数据位置的数组和结构也可以从函数中返回，
    但不可能分配这种类型。

.. 注解::
    在0.6.9版本之前，引用型参数的数据位置被限制在外部函数中的 ``calldata``，
    公开函数中的 ``memory``，以及内部和私有函数中的 ``memory`` 或 ``storage``。
    现在 ``memory`` 和 ``calldata`` 在所有函数中都被允许使用，无论其可见性如何。

.. 注解::
    在0.5.0版本之前，数据位置可以省略，并且会根据变量的种类、函数类型等默认为不同的位置，
    但现在所有的复杂类型都必须给出一个明确的数据位置。

.. _data-location-assignment:

数据位置和分配行为
^^^^^^^^^^^^^^^^^^

数据位置不仅与数据的持久性有关，而且也与分配的语义有关：

* 在 ``storage`` 和 ``memory`` 之间的分配（或从 ``calldata`` 中分配） 总是创建一个独立的拷贝。
* 从 ``memory`` 到 ``memory`` 的赋值只创建引用。
  这意味着对一个内存变量的改变在所有其他引用相同数据的内存变量中也是可见的。
* 从  ``storage`` 到 **local** 存储变量的赋值也只赋值一个引用。
* 所有其他对 ``storage`` 的赋值总是拷贝的。
  这种情况的例子是对状态变量或存储结构类型的局部变量成员的赋值，
  即使局部变量本身只是一个引用。

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.5.0 <0.9.0;

    contract C {
        // x 的数据存储位置是 storage。
        // 这是唯一可以省略数据位置的地方。
        uint[] x;

        // memoryArray 的数据存储位置是 memory。
        function f(uint[] memory memoryArray) public {
            x = memoryArray; // 将整个数组拷贝到 storage 中，可行
            uint[] storage y = x; // 分配一个指针，其中 y 的数据存储位置是 storage，可行
            y[7]; // 返回第 8 个元素，可行
            y.pop(); // 通过y修改x，可行
            delete x; // 清除数组，同时修改 y，可行
            // 下面的就不可行了；需要在 storage 中创建新的未命名的临时数组，/
            // 但 storage 是“静态”分配的：
            // y = memoryArray;
            // 下面这一行也不可行，因为这会“重置”指针，
            // 但并没有可以让它指向的合适的存储位置。
            // delete y;
            g(x); // 调用 g 函数，同时移交对 x 的引用
            h(x); // 调用 h 函数，同时在 memory 中创建一个独立的临时拷贝
        }

        function g(uint[] storage) internal pure {}
        function h(uint[] memory) public pure {}
    }

.. index:: ! array

.. _arrays:

数组
------

数组可以在声明时指定长度，也可以动态调整大小。

一个元素类型为 ``T``，固定长度为 ``k`` 的数组可以声明为 ``T[k]``，
而动态数组声明为 ``T[]``。

例如，一个由5个 ``uint`` 的动态数组组成的数组被写成 ``uint[][5]``。
与其他一些语言相比, 这种记法是相反的.
在Solidity中, ``X[3]`` 总是一个包含三个 ``X`` 类型元素的数组,
即使 ``X`` 本身是一个数组. 这在其他语言中是不存在的，如C语言。

索引是基于零的，访问方向与声明相反。

例如，如果您有一个变量 ``uint[][5] memory x``，您用 ``x[2][6]`` 访问第三个动态数组中的第七个 ``uint``，
要访问第三个动态数组，用 ``x[2]``。同样，如果您有一个数组 ``T[5] a`` 的类型 ``T``，
也可以是一个数组，那么 ``a[2]`` 总是有类型 ``T``。

数组元素可以是任何类型，包括映射或结
类型的一般限制也适用，映射只能存储在 ``storage`` 数据位置，
公开可见的函数需要参数是 :ref:`ABI类型<ABI>`。

可以将状态变量数组标记为 ``public``，
并让Solidity创建一个 :ref:`getter <visibility-and-getters>` 函数。数字索引成为该函数的一个必要参数。

访问一个超过它的末端的数组会导致一个失败的断言。
方法 ``.push()`` 和 ``.push(value)`` 可以用来在数组的末端追加一个新的元素，
其中 ``.push()`` 追加一个零初始化的元素并返回它的引用。

.. index:: ! string, ! bytes

.. _strings:

.. _bytes:

``bytes`` 和 ``string`` 类型的数组
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

``bytes`` 和 ``string`` 类型的变量是特殊的数组。 ``bytes`` 类似于 ``bytes1[]``，
但它在 calldata 中会被“紧打包”（译者注：将元素连续地存在一起，不会按每 32 字节一单元的方式来存放）。
``string`` 与 ``bytes`` 相同，但不允许用长度或索引来访问。

Solidity没有字符串操作函数，但有第三方的字符串库。
您也可以用 ``keccak256(abi.encodePacked(s1)) == keccak256(abi.encodePacked(s2))``
来比较两个字符串的keccak256-hash，用 ``string.concat(s1, s2)`` 来连接两个字符串。

您应该使用 ``bytes`` 而不是 ``bytes1[]``，因为它更便宜，
因为在 ``memory`` 中使用 ``bytes1[]`` 会在元素之间增加31个填充字节。
请注意，在 ``storage`` 中，由于紧打包，没有填充，参见 :ref:`字节和字符串 <bytes-and-string>`。
一般来说，对于任意长度的原始字节数据使用 ``bytes``，对于任意长度的字符串（UTF-8）数据使用 ``string``。
如果您能将长度限制在一定的字节数，总是使用 ``bytes1`` 到 ``bytes32`` 中的一种值类型，因为它们更便宜。

.. 注解::
    如果想要访问以字节表示的字符串 ``s``，
    请使用 ``bytes(s).length`` / ``bytes(s)[7] = 'x';``。
    注意这时您访问的是 UTF-8 形式的低级 bytes 类型，而不是单个的字符。

.. index:: ! bytes-concat, ! string-concat

.. _bytes-concat:
.. _string-concat:

函数 ``bytes.concat`` 和 ``string.concat``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

您可以使用 ``string.concat`` 连接任意数量的 ``string`` 值。
该函数返回一个单一的 ``string memory`` 数组，其中包含没有填充的参数内容。
如果您想使用不能隐式转换为 ``string`` 的其他类型的参数，您需要先将它们转换为 ``string``。

同样， ``bytes.concat`` 函数可以连接任意数量的 ``bytes`` 或 ``bytes1 ... bytes32`` 值。
该函数返回一个单一的 ``bytes memory`` 数组，其中包含没有填充的参数内容。
如果您想使用字符串参数或其他不能隐式转换为 ``bytes`` 的类型，
您需要先将它们转换为 ``bytes`` 或 ``bytes1`` /.../ ``bytes32``。

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity ^0.8.12;

    contract C {
        string s = "Storage";
        function f(bytes calldata bc, string memory sm, bytes16 b) public view {
            string memory concatString = string.concat(s, string(bc), "Literal", sm);
            assert((bytes(s).length + bc.length + 7 + bytes(sm).length) == bytes(concatString).length);

            bytes memory concatBytes = bytes.concat(bytes(s), bc, bc[:2], "Literal", bytes(sm), b);
            assert((bytes(s).length + bc.length + 2 + 7 + bytes(sm).length + b.length) == concatBytes.length);
        }
    }

如果您不带参数调用 ``string.concat`` 或 ``bytes.concat``，它们会返回一个空数组。

.. index:: ! array;allocating, new

创建内存数组
^^^^^^^^^^^^^

具有动态长度的内存数组可以使用 ``new`` 操作符创建。
与存储数组不同的是，**不可能** 调整内存数组的大小（例如， ``.push`` 成员函数不可用）。
您必须事先计算出所需的大小，或者创建一个新的内存数组并复制每个元素。

正如Solidity中的所有变量一样，新分配的数组元素总是以 :ref:`默认值 <default-value>` 进行初始化。

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.4.16 <0.9.0;

    contract C {
        function f(uint len) public pure {
            uint[] memory a = new uint[](7);
            bytes memory b = new bytes(len);
            assert(a.length == 7);
            assert(b.length == len);
            a[6] = 8;
        }
    }

.. index:: ! array;literals, ! inline;arrays

数组字面常数
^^^^^^^^^^^^^^

数组字面常数表达式是一个逗号分隔的一个或多个表达式的列表，用方括号（ ``[...]`` ）括起来。
例如， ``[1, a, f(3)]``。数组字面常数的类型确定如下：

它总是一个静态大小的内存数组，其长度是表达式的数量。

数组的基本类型是列表上第一个表达式的类型，这样所有其他表达式都可以隐含地转换为它。
如果不能做到这一点，则会有一个类型错误。

仅仅存在一个所有元素都可以转换的类型是不够的。其中一个元素必须是该类型的。

在下面的例子中， ``[1, 2, 3]`` 的类型是 ``uint8[3] memory``，
因为这些常量的类型都是 ``uint8``。如果您想让结果是 ``uint[3] memory`` 类型，
您需要把第一个元素转换为 ``uint``。

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.4.16 <0.9.0;

    contract C {
        function f() public pure {
            g([uint(1), 2, 3]);
        }
        function g(uint[3] memory) public pure {
            // ...
        }
    }

数组表达式 ``[1, -1]`` 是无效的，因为第一个表达式的类型是 ``uint8``，
而第二个表达式的类型是 ``int8``，它们不能相互隐式转换。为了使其有效，
例如，您可以使用 ``[int8(1), -1]``。

由于不同类型的固定大小的内存数组不能相互转换（即使基类可以），
如果您想使用二维数组字面常数，您必须总是明确指定一个共同的基类：

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.4.16 <0.9.0;

    contract C {
        function f() public pure returns (uint24[2][4] memory) {
            uint24[2][4] memory x = [[uint24(0x1), 1], [0xffffff, 2], [uint24(0xff), 3], [uint24(0xffff), 4]];
            // 下面的方法不会起作用，因为一些内部数组的类型不对。
            // uint[2][4] memory x = [[0x1, 1], [0xffffff, 2], [0xff, 3], [0xffff, 4]];
            return x;
        }
    }

固定大小的内存数组不能分配给动态大小的内存数组，也就是说，以下情况是不可能的：

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.4.0 <0.9.0;

    // This will not compile.
    contract C {
        function f() public {
            // 下一行会产生一个类型错误，因为uint[3]内存不能被转换为uint[]内存。
            uint[] memory x = [uint(1), 3, 4];
        }
    }

计划在将来取消这一限制，但由于ABI中数组的传递方式，它产生了一些复杂的问题。

如果您想初始化动态大小的数组，您必须分配各个元素：

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.4.16 <0.9.0;

    contract C {
        function f() public pure {
            uint[] memory x = new uint[](3);
            x[0] = 1;
            x[1] = 3;
            x[2] = 4;
        }
    }

.. index:: ! array;length, length, push, pop, !array;push, !array;pop

.. _array-members:

数组成员
^^^^^^^^^^

**length**:
    数组有 ``length`` 成员变量表示当前数组的长度。一经创建，
    内存memory数组的大小就是固定的（但却是动态的，也就是说，它依赖于运行时的参数）。
**push()**:
    动态存储数组和 ``bytes`` （不是 ``string`` ）有一个叫 ``push()`` 的成员函数，
    您可以用它在数组的末尾追加一个零初始化的元素。它返回一个元素的引用，
    因此可以像 ``x.push().t = 2`` 或 ``x.push() = b`` 那样使用。
**push(x)**:
    动态存储数组和 ``bytes`` （不是 ``string`` ）有一个叫 ``push(x)`` 的成员函数，
    您可以用它在数组的末端追加一个指定的元素。该函数不返回任何东西。
**pop()**:
    动态存储数组和 ``bytes`` （不是 ``string`` ）有一个叫 ``pop()`` 的成员函数，
    您可以用它来从数组的末端移除一个元素。这也隐含地在被删除的元素上调用 :ref:`delete <delete>`。

.. 注解::
    通过调用 ``push()`` 增加存储数组的长度有恒定的气体成本，因为存储是零初始化的，
    而通过调用 ``pop()`` 减少长度的成本取决于被移除元素的 "大小"。
    如果该元素是一个数组，它的成本可能非常高，
    因为它包括明确地清除被移除的元素，类似于对它们调用 :ref:`delete <delete>`。

.. 注解::
    要在外部（而不是公开）函数中使用数组的数组，
    您需要激活ABI coder v2。

.. 注解::
    在Byzantium之前的EVM版本中，不可能访问从函数调用返回的动态数组。
    如果您调用返回动态数组的函数，请确保使用设置为Byzantium模式的EVM。

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.6.0 <0.9.0;

    contract ArrayContract {
        uint[2**20] aLotOfIntegers;
        // 注意下面的代码并不是一对动态数组，
        // 而是一个数组元素为一对变量的动态数组（也就是数组元素为长度为 2 的定长数组的动态数组）。
        // 因为，T[] 总是 T 的一个动态数组，即使 T 本身是一个数组。
        // 所有状态变量的数据位置是 storage。
        bool[2][] pairsOfFlags;

        // newPairs被存储在memory中--这是公开合约函数参数的唯一可能性。
        function setAllFlagPairs(bool[2][] memory newPairs) public {
            // 赋值到一个存储数组会执行 ``newPairs`` 的拷贝，
            // 并替换完整的阵列 ``pairsOfFlags``。
            pairsOfFlags = newPairs;
        }

        struct StructType {
            uint[] contents;
            uint moreInfo;
        }
        StructType s;

        function f(uint[] memory c) public {
            // 在 ``g`` 中存储一个对 ``s`` 的引用。
            StructType storage g = s;
            // 也改变了 ``s.moreInfo``.
            g.moreInfo = 2;
            // 指定一个拷贝，因为 ``g.contents`` 不是一个局部变量，
            // 而是一个局部变量的成员。
            g.contents = c;
        }

        function setFlagPair(uint index, bool flagA, bool flagB) public {
            // 访问一个不存在的数组索引会引发一个异常
            pairsOfFlags[index][0] = flagA;
            pairsOfFlags[index][1] = flagB;
        }

        function changeFlagArraySize(uint newSize) public {
            // 使用push和pop是改变数组长度的唯一方法。
            if (newSize < pairsOfFlags.length) {
                while (pairsOfFlags.length > newSize)
                    pairsOfFlags.pop();
            } else if (newSize > pairsOfFlags.length) {
                while (pairsOfFlags.length < newSize)
                    pairsOfFlags.push();
            }
        }

        function clear() public {
            // 这些完全清除了数组
            delete pairsOfFlags;
            delete aLotOfIntegers;
            // 这里有相同的效果
            pairsOfFlags = new bool[2][](0);
        }

        bytes byteData;

        function byteArrays(bytes memory data) public {
            // 字节数组（"byte"）是不同的，因为它们的存储没有填充，
            // 但可以与 "uint8[]"相同。
            byteData = data;
            for (uint i = 0; i < 7; i++)
                byteData.push();
            byteData[3] = 0x08;
            delete byteData[2];
        }

        function addFlag(bool[2] memory flag) public returns (uint) {
            pairsOfFlags.push(flag);
            return pairsOfFlags.length;
        }

        function createMemoryArray(uint size) public pure returns (bytes memory) {
            // 使用 `new` 创建动态 memory 数组：
            uint[2][] memory arrayOfPairs = new uint[2][](size);

            // 内联数组总是静态大小的，如果您只使用字面常数表达式，您必须至少提供一种类型。
            arrayOfPairs[0] = [uint(1), 2];

            // 创建一个动态字节数组：
            bytes memory b = new bytes(200);
            for (uint i = 0; i < b.length; i++)
                b[i] = bytes1(uint8(i));
            return b;
        }
    }

.. index:: ! array;slice

.. _array-slices:

数组切片
------------


数组切片是对一个数组的连续部分的预览。
它们被写成 ``x[start:end]``，其中 ``start`` 和 ``end`` 是表达式，
结果是uint256类型（或隐含的可转换类型）。分片的第一个元素是 ``x[start]``，
最后一个元素是 ``x[end - 1]``。

如果 ``start`` 大于 ``end``，或者 ``end`` 大于数组的长度，
就会出现异常。

``start`` 和 ``end`` 都是可选的： ``start`` 默认为 ``0``，
``end`` 默认为数组的长度。

数组切片没有任何成员。它们可以隐含地转换为其底层类型的数组并支持索引访问。
索引访问在底层数组中不是绝对的，而是相对于分片的开始。

数组切片没有类型名，这意味着任何变量都不能以数组切片为类型，
它们只存在于中间表达式中。

.. 注解::
    到现在为止，数组切片只有calldata数组可以实现。

数组切片对于ABI解码在函数参数中传递的二级数据很有用：

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.8.5 <0.9.0;
    contract Proxy {
        /// @dev 由代理管理的客户合约的地址，即本合约的地址
        address client;

        constructor(address _client) {
            client = _client;
        }

        /// 转发对 "setOwner(address)" 的调用，
        /// 该调用在对地址参数进行基本验证后由客户端执行。
        function forward(bytes calldata _payload) external {
            bytes4 sig = bytes4(_payload[:4]);
            // 由于截断行为，bytes4(_payload)的表现是相同的。
            // bytes4 sig = bytes4(_payload);
            if (sig == bytes4(keccak256("setOwner(address)"))) {
                address owner = abi.decode(_payload[4:], (address));
                require(owner != address(0), "Address of owner cannot be zero.");
            }
            (bool status,) = client.delegatecall(_payload);
            require(status, "Forwarded call failed.");
        }
    }



.. index:: ! struct, ! type;struct

.. _structs:

结构体
-------

Solidity 提供了一种以结构形式定义新类型的方法，以下是一个结构体使用的示例：

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.6.0 <0.9.0;

    // 定义一个包含两个属性的新类型。
    // 在合约之外声明一个结构，
    // 可以让它被多个合约所共享。
    // 在这里，这并不是真的需要。
    struct Funder {
        address addr;
        uint amount;
    }

    contract CrowdFunding {
        // 结构体也可以被定义在合约内部，这使得它们只在本合约和派生合约中可见。
        struct Campaign {
            address payable beneficiary;
            uint fundingGoal;
            uint numFunders;
            uint amount;
            mapping (uint => Funder) funders;
        }

        uint numCampaigns;
        mapping (uint => Campaign) campaigns;

        function newCampaign(address payable beneficiary, uint goal) public returns (uint campaignID) {
            campaignID = numCampaigns++; // campaignID 作为一个变量返回
            // 我们不能使用 "campaigns[campaignID] = Campaign(beneficiary, goal, 0, 0)"
            // 因为右侧创建了一个内存结构 "Campaign"，其中包含一个映射。
            Campaign storage c = campaigns[campaignID];
            c.beneficiary = beneficiary;
            c.fundingGoal = goal;
        }

        function contribute(uint campaignID) public payable {
            Campaign storage c = campaigns[campaignID];
            // 以给定的值初始化，创建一个新的临时 memory 结构体，
            // 并将其拷贝到 storage 中。
            // 注意您也可以使用 Funder(msg.sender, msg.value) 来初始化。
            c.funders[c.numFunders++] = Funder({addr: msg.sender, amount: msg.value});
            c.amount += msg.value;
        }

        function checkGoalReached(uint campaignID) public returns (bool reached) {
            Campaign storage c = campaigns[campaignID];
            if (c.amount < c.fundingGoal)
                return false;
            uint amount = c.amount;
            c.amount = 0;
            c.beneficiary.transfer(amount);
            return true;
        }
    }

上面的合约并没有提供众筹合约的全部功能，
但它包含了理解结构体所需的基本概念。
结构类型可以在映射和数组内使用，
它们本身可以包含映射和数组。

结构体不可能包含其自身类型的成员，尽管结构本身可以是映射成员的值类型，
或者它可以包含其类型的动态大小的数组。
这一限制是必要的，因为结构的大小必须是有限的。

注意在所有的函数中，结构类型被分配到数据位置为 ``storage`` 的局部变量。
这并不是拷贝结构体，而只是存储一个引用，
因此对本地变量成员的赋值实际上是写入状态。

当然，您也可以直接访问该结构的成员，
而不把它分配给本地变量，如 ``campaigns[campaignID].amount = 0``。

.. 注解::
    在 Solidity 0.7.0 之前，包含仅有存储类型（例如映射）的成员的内存结构是允许的，
    像上面例子中的 ``campaigns[campaignID] = Campaign(beneficiary, goal, 0, 0)`` 这样的赋值是可以的，
    只是会默默地跳过这些成员。
