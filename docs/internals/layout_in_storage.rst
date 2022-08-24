.. index:: storage, state variable, mapping

************************************
存储中的状态变量储存结构
************************************

.. _storage-inplace-encoding:

合约的状态变量以一种紧凑的方式存储，
这样多个值有时会使用同一个存储槽。
除了动态大小的数组和映射（见下文）之外，
数据是被逐项存储的，从第一个状态变量开始，
它被存储在槽 ``0`` 中。对于每个变量，
根据它的类型确定一个字节的大小。如果可能的话，需要少于32字节的多个连续项目被打包到一个存储槽中，
根据以下规则：

- 存储插槽的第一项会以低位对齐（即右对齐）的方式储存。
- 值类型只使用存储它们所需的字节数。
- 如果一个值类型不适合一个存储槽的剩余部分，它将被存储在下一个存储槽。
- 结构和数组数据总是从一个新的存储槽开始，它们的项根据这些规则被紧密地打包。
- 结构或数组数据之后的变量总是开辟一个新的存储槽。

对于使用继承的合约，状态变量的排序是从最基础的合约开始，由合约的C3线性化顺序决定的。
如果上述规则允许，来自不同合约的状态变量确实共享同一个存储槽。

结构体和数组中的元素都是顺序存储的，就像它们被明确给定的那样。

.. warning::
    当使用小于32字节的元素时，您的合约的气体用量可能会更高。
    这是因为EVM每次对32字节进行操作。因此，如果元素小于这个大小，
    EVM必须使用更多的操作，以便将元素的大小从32字节减少到所需的大小。

    如果您处理的是存储值，使用缩小尺寸的类型可能是有益的，
    因为编译器会将多个元素打包到一个存储槽中，
    从而将多个读或写合并到一个操作中。
    如果您不是在同一时间读取或写入一个槽中的所有值，
    这可能会产生相反的效果，虽然。当一个值被写入一个多值存储槽时，
    该存储槽必须先被读取，
    然后与新值结合，这样同一槽中的其他数据就不会被破坏。

    在处理函数参数或内存值时，因为编译器不会打包这些值，所以没有什么好处，

    最后，为了让EVM对此进行优化，
    确保您的存储变量和 ``struct`` 成员的顺序，使它们能够被紧密地包装起来。
    例如，按照 ``uint128, uint128, uint256`` 的顺序声明您的存储变量，
    而不是 ``uint128, uint256, uint128``，因为前者只占用两个存储槽，
    而后者则占用三个存储槽。

.. note::
     由于存储指针可以传递给库，存储中的状态变量的结构被认为是 Solidity 外部接口的一部分。
     这意味着对这一节中概述的规则的任何改变都被认为是对语言的重大改变，
     由于它的关键性质，在执行之前应该非常仔细地考虑。
     在发生这种重大变化的情况下，我们希望发布一种兼容模式，
     在这种模式下，编译器将生成支持旧结构的字节码。


映射和动态数组
===========================

.. _storage-hashed-encoding:

由于映射和动态数组的大小是不可预知的，他们不能被存储在其前后的状态变量之间。
相反，它们被认为只占用32个字节， 与 :ref:`上述规则 <storage-inplace-encoding>` 有关，
它们所包含的元素被存储在一个不同的存储槽，该存储槽是用Keccak-256哈希计算的。

假设映射或数组的存储位置在适应了 :ref:`存储结构规则 <storage-inplace-encoding>` 后，最终位于一个槽 ``p``。
对于动态数组，这个槽存储了数组中的元素数量
（字节数组和字符串是一个例外，参见 :ref:`下文 <bytes-and-string>`）。
对于映射来说，这个槽保持空的状态，
但是仍然需要它来确保即使有两个映射相邻，它们的内容最终也是在不同的存储位置。

数组数据从 ``keccak256(p)`` 开始，它的排列方式与静态大小的阵列数据相同：
一个元素接着一个元素，如果元素的长度不超过16字节，
就有可能共享存储槽。包含动态数组的动态数组递归地应用这一规则。
元素 ``x[i][j]`` 的位置为，其中 ``x`` 的类型是 ``uint24[][]`` ，
计算方法如下（同样，假设 ``x`` 本身存储在槽 ``p``）：
槽是 ``keccak256(keccak256(p)+i)+ floor(j / floor(256 / 24))``，
元素可以从槽数据 ``v`` 得到，使用 ``(v >> ((j % floor(256 / 24)) * 24)) & type(uint24).max``。

对应于映射键 ``k`` 的值位于 ``keccak256(h(k) . p)``，
其中 ``.`` 是连接符， ``h`` 是一个函数，根据键的类型应用于键。

- 对于值类型， 函数 ``h`` 将与在内存中存储值的相同方式来将值填充为32字节。
- 对于字符串和字节数组， ``h(k)`` 只是未填充的数据。

如果映射类型的值是一个非值类型，则计算的槽会标记为数据的开始位置。
例如，如果值是结构体类型，您必须添加一个与结构体成员相对应的偏移量才能访问到该成员。

作为示例，参考以下合约：

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.4.0 <0.9.0;


    contract C {
        struct S { uint16 a; uint16 b; uint256 c; }
        uint x;
        mapping(uint => mapping(uint => S)) data;
    }

让我们计算一下 ``data[4][9].c`` 的存储位置。
映射本身的位置是 ``1`` （变量 ``x`` 前面有32字节）。
这意味着 ``data[4]`` 存储在 ``keccak256(uint256(4) . uint256(1))``。
``data[4]`` 的类型还是一个映射，
``data[4][9]`` 的数据从 ``keccak256(uint256(9) . keccak256(uint256(4) . uint256(1)))`` 槽开始。
成员 ``c`` 在结构 ``S`` 中的槽位偏移是 ``1``，因为 ``a`` 和 ``b`` 被装在一个槽位中。
这意味着 ``data[4][9].c`` 的插槽是 ``keccak256(uint256(9) . keccak256(uint256(4) . uint256(1))) + 1``。
该值的类型是 ``uint256``，所以它占用一个槽。


.. _bytes-and-string:

``bytes`` 和 ``string``
------------------------

``bytes`` 和 ``string`` 的编码是相同的。
一般来说，编码与 ``bytes1[]`` 类似，即有一个槽用于存放数组本身和一个数据区，
这个数据区是用该槽的位置的 ``keccak256`` 哈希值计算的。
然而，对于较短的值（短于32字节），数组元素与长度一起存储在同一个槽中。

特别是：如果数据最多只有 ``31`` 字节长，
元素被存储在高阶字节中（左对齐），最低阶字节存储值 ``length * 2``。
对于存储数据长度为 ``32`` 或更多字节的字节数，主槽 ``p`` 存储 ``length * 2 + 1``，
数据照常存储在 ``keccak256(p)``。这意味着您可以通过检查最低位是否被设置来区分短数组和长数组：
短数组（未设置）和长数组（设置）。

.. note::
  目前不支持处理无效编码的槽，但将来可能会加入。
  如果您通过实验性的基于IR的编译器通道进行编译，读取一个无效编码的槽会导致 ``Panic(0x22)`` 错误。

JSON输出
===========

.. _storage-layout-top-level:

合约的存储结构可以通过 :ref:`标准的JSON接口 <compiler-api>` 请求获得。
输出的是一个JSON对象，包含两个键， ``storage`` 和 ``types``。
``storage`` 对象是一个数组，每个元素都有以下形式：


.. code::


    {
        "astId": 2,
        "contract": "fileA:A",
        "label": "x",
        "offset": 0,
        "slot": "0",
        "type": "t_uint256"
    }

上面的例子来自源于项目 ``fileA`` 的 ``contract A { uint x; }`` 的存储结构，并且

- ``astId`` 是状态变量声明的AST节点的ID
- ``contract`` 是合约的名称，包括其路径作为前缀
- ``label`` 是状态变量的名称
- ``offset`` 是根据编码在存储槽中的字节偏移量
- ``slot`` 是状态变量所在或开始的存储槽。这个数字可能非常大，因此它的JSON值被表示为一个字符串
- ``type`` 是一个标识符，作为变量类型信息的关键（如下所述）

给定的 ``type``，在这里是 ``t_uint256``，代表 ``types`` 中的一个元素，它的形式是：


.. code::

    {
        "encoding": "inplace",
        "label": "uint256",
        "numberOfBytes": "32",
    }

这里

- ``encoding`` 数据在存储中是如何编码的，可能的值是：

  - ``inplace``： 数据在存储中是连续布置的（参见 :ref:`上文 <storage-inplace-encoding>`）。
  - ``mapping``： 基于Keccak-256的哈希方法（参见 :ref:`上文 <storage-hashed-encoding>`）。
  - ``dynamic_array``： 基于Keccak-256的哈希方法（参见 :ref:`上文 <storage-hashed-encoding>`）。
  - ``bytes``： 单槽或基于Keccak-256哈希值，取决于数据大小（参见 :ref:`上文 <bytes-and-string>`）。

- ``label`` 是典型的类型名称。
- ``numberOfBytes`` 是使用的字节数（十进制字符串）。
  注意，如果 ``numberOfBytes > 32`` 这意味着使用了一个以上的槽。

除了上述四种类型外，有些类型还有额外的信息。
映射包含它的 ``key`` 和 ``value`` 类型（再次引用这个类型映射中的一个项），
数组有它的 ``base`` 类型，结构体会列出它们的 ``成员``，
其格式与高层次的 ``storage`` 相同（参见 :ref:`上文 <storage-layout-top-level>`）。

.. note::
  合约的存储结构的JSON输出格式仍被认为是实验性的，并且在Solidity的非重大版本中会有变化。

下面的例子显示了一个合约及其存储结构，包含值类型和引用类型，被编码打包的类型，以及嵌套的类型。


.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.4.0 <0.9.0;
    contract A {
        struct S {
            uint128 a;
            uint128 b;
            uint[2] staticArray;
            uint[] dynArray;
        }

        uint x;
        uint y;
        S s;
        address addr;
        mapping (uint => mapping (address => bool)) map;
        uint[] array;
        string s1;
        bytes b1;
    }

.. code:: json

    {
      "storage": [
        {
          "astId": 15,
          "contract": "fileA:A",
          "label": "x",
          "offset": 0,
          "slot": "0",
          "type": "t_uint256"
        },
        {
          "astId": 17,
          "contract": "fileA:A",
          "label": "y",
          "offset": 0,
          "slot": "1",
          "type": "t_uint256"
        },
        {
          "astId": 20,
          "contract": "fileA:A",
          "label": "s",
          "offset": 0,
          "slot": "2",
          "type": "t_struct(S)13_storage"
        },
        {
          "astId": 22,
          "contract": "fileA:A",
          "label": "addr",
          "offset": 0,
          "slot": "6",
          "type": "t_address"
        },
        {
          "astId": 28,
          "contract": "fileA:A",
          "label": "map",
          "offset": 0,
          "slot": "7",
          "type": "t_mapping(t_uint256,t_mapping(t_address,t_bool))"
        },
        {
          "astId": 31,
          "contract": "fileA:A",
          "label": "array",
          "offset": 0,
          "slot": "8",
          "type": "t_array(t_uint256)dyn_storage"
        },
        {
          "astId": 33,
          "contract": "fileA:A",
          "label": "s1",
          "offset": 0,
          "slot": "9",
          "type": "t_string_storage"
        },
        {
          "astId": 35,
          "contract": "fileA:A",
          "label": "b1",
          "offset": 0,
          "slot": "10",
          "type": "t_bytes_storage"
        }
      ],
      "types": {
        "t_address": {
          "encoding": "inplace",
          "label": "address",
          "numberOfBytes": "20"
        },
        "t_array(t_uint256)2_storage": {
          "base": "t_uint256",
          "encoding": "inplace",
          "label": "uint256[2]",
          "numberOfBytes": "64"
        },
        "t_array(t_uint256)dyn_storage": {
          "base": "t_uint256",
          "encoding": "dynamic_array",
          "label": "uint256[]",
          "numberOfBytes": "32"
        },
        "t_bool": {
          "encoding": "inplace",
          "label": "bool",
          "numberOfBytes": "1"
        },
        "t_bytes_storage": {
          "encoding": "bytes",
          "label": "bytes",
          "numberOfBytes": "32"
        },
        "t_mapping(t_address,t_bool)": {
          "encoding": "mapping",
          "key": "t_address",
          "label": "mapping(address => bool)",
          "numberOfBytes": "32",
          "value": "t_bool"
        },
        "t_mapping(t_uint256,t_mapping(t_address,t_bool))": {
          "encoding": "mapping",
          "key": "t_uint256",
          "label": "mapping(uint256 => mapping(address => bool))",
          "numberOfBytes": "32",
          "value": "t_mapping(t_address,t_bool)"
        },
        "t_string_storage": {
          "encoding": "bytes",
          "label": "string",
          "numberOfBytes": "32"
        },
        "t_struct(S)13_storage": {
          "encoding": "inplace",
          "label": "struct A.S",
          "members": [
            {
              "astId": 3,
              "contract": "fileA:A",
              "label": "a",
              "offset": 0,
              "slot": "0",
              "type": "t_uint128"
            },
            {
              "astId": 5,
              "contract": "fileA:A",
              "label": "b",
              "offset": 16,
              "slot": "0",
              "type": "t_uint128"
            },
            {
              "astId": 9,
              "contract": "fileA:A",
              "label": "staticArray",
              "offset": 0,
              "slot": "1",
              "type": "t_array(t_uint256)2_storage"
            },
            {
              "astId": 12,
              "contract": "fileA:A",
              "label": "dynArray",
              "offset": 0,
              "slot": "3",
              "type": "t_array(t_uint256)dyn_storage"
            }
          ],
          "numberOfBytes": "128"
        },
        "t_uint128": {
          "encoding": "inplace",
          "label": "uint128",
          "numberOfBytes": "16"
        },
        "t_uint256": {
          "encoding": "inplace",
          "label": "uint256",
          "numberOfBytes": "32"
        }
      }
    }
