.. _metadata:

#################
合约的元数据
#################

.. index:: metadata, contract verification

Solidity 编译器自动生成一个 JSON 文件，即合约元数据，
其中包含有关已编译合约的信息。您可以使用此文件来查询编译器版本，使用的源码，ABI 和 NatSpec 文档，
以便更安全地与合约交互并验证其源代码。

编译器默认将元数据文件的IPFS哈希附加到每个合约的字节码末尾（详见下文），
这样您就可以通过认证的方式来检索文件，而不必求助于中心化数据提供者。
其他可用的选项是Swarm哈希值和不将元数据哈希值附加到字节码上。
这些可以通过 :ref:`标准 JSON 接口 <compiler-api>` 来配置。

您必须将元数据文件发布到IPFS、Swarm或其他服务，
以便其他人可以访问它。您可以通过使用 ``solc --metadata`` 命令来创建该文件，
该命令生成一个名为 ``ContractName_meta.json`` 的文件。
它包含IPFS和Swarm对源代码的引用，所以您必须上传所有的源文件和元数据文件。

元数据文件有以下格式。下面的例子是以人类可读的方式呈现的。
正确的元数据格式应该正确地使用引号，
将空白减少到最低限度，并对所有对象的键进行排序，得出唯一的格式。
注释是不被允许的，在此仅用于解释目的。

.. code-block:: javascript

    {
      // 必选：元数据格式的版本
      "version": "1",
      // 必选：源代码的编程语言，一般会选择规范的“子版本”
      "language": "Solidity",
      // 必选：编译器的详情，内容视语言而定。
      "compiler": {
        // 对 Solidity 来说是必须的：编译器的版本
        "version": "0.4.6+commit.2dabbdf0.Emscripten.clang",
        // 可选： 生成此输出的编译器二进制文件的哈希值
        "keccak256": "0x123..."
      },
      // 必选：编译的源文件／源单位，键值为文件名
      "sources":
      {
        "myFile.sol": {
          // 必选：源文件的 keccak256 哈希值
          "keccak256": "0x123...",
          // 必选（除非定义了“content”，详见下文）：
          // 已排序的源文件的URL，URL的协议可以是任意的，但建议使用 Swarm 的URL
          "urls": [ "bzzr://56ab..." ],
          // 可选：源文件中给出的SPDX许可证标识符。
          "license": "MIT"
        },
        "destructible": {
          // 必选：源文件的 keccak256 哈希值
          "keccak256": "0x234...",
          // 必选（除非定义了“urls”）： 源文件的字面内容
          "content": "contract destructible is owned { function destroy() { if (msg.sender == owner) selfdestruct(owner); } }"
        }
      },
      // 必选：编译器的设置
      "settings":
      {
        // 对 Solidity 来说是必须的： 已排序的重定向列表
        "remappings": [ ":g=/dir" ],
        // 可选：优化器设置。“enabled” 和 "runs" 这两个字段已被废弃，
        // 这里只是为了向后兼容而给出。
        "optimizer": {
          "enabled": true,
          "runs": 500,
          "details": {
            // 默认值为 “true“
            "peephole": true,
            // 内联器默认值为 “true“
            "inliner": true,
            // 跳转目的地移除器默认为 “true“
            "jumpdestRemover": true,
            "orderLiterals": false,
            "deduplicate": false,
            "cse": false,
            "constantOptimizer": false,
            "yul": true,
            // 可选：只在 “yul“ 为 “true“ 时出现
            "yulDetails": {
              "stackAllocation": false,
              "optimizerSteps": "dhfoDgvulfnTUtnIf..."
            }
          }
        },
        "metadata": {
          // 显示输入json中使用的设置，默认为false。
          "useLiteralContent": true,
          // 显示输入json中使用的设置，默认为 “ipfs“
          "bytecodeHash": "ipfs"
        },
        // 对 Solidity 来说是必须的：用以生成该元数据的文件名和合约名或库名
        "compilationTarget": {
          "myFile.sol": "MyContract"
        },
        // 对 Solidity 来说是必须的：所使用的库合约的地址
        "libraries": {
          "MyLib": "0x123123..."
        }
      },
      // 必选：合约的生成信息
      "output":
      {
        // 必选：合约的 ABI 定义
        "abi": [/* ... */],
        // 必选：合约的 NatSpec 用户文档
        "userdoc": [/* ... */],
        // 必选：合约的 NatSpec 开发者文档
        "devdoc": [/* ... */]
      }
    }

.. warning::
  由于产生的合约的字节码默认包含元数据哈希值，
  对元数据的任何改变都可能导致字节码的改变。
  这包括对文件名或路径的改变，而且由于元数据包括所有使用的源的哈希值，
  一个空白的改变就会导致不同的元数据和不同的字节码。

.. note::
    上面的ABI定义没有固定的顺序。它可以随着编译器的版本而改变。
    不过，从Solidity 0.5.12版本开始，该数组保持一定的顺序。

.. _encoding-of-the-metadata-hash-in-the-bytecode:

在字节码中对元数据哈希值进行编码
=============================================

因为我们将来可能会支持其他方式来检索元数据文件，
所以映射 ``{"ipfs": <IPFS 哈希值>, "solc": <编译器版本>}`` 将以
`CBOR <https://tools.ietf.org/html/rfc7049>`_-编码来存储。
由于映射可能包含更多的键（见下文），而且该编码的开头不容易找到，
所以添加两个字节来表述其长度，以大端方式编码。
当前版本的 Solidity 编译器通常在部署的字节码的末尾添加以下内容

.. code-block:: text

    0xa2
    0x64 'i' 'p' 'f' 's' 0x58 0x22 <34字节的IPFS哈希值>
    0x64 's' 'o' 'l' 'c' 0x43 <3字节的版本编码>
    0x00 0x33

因此，为了检索数据，可以检查部署的字节码的结尾是否符合该模式，
并使用IPFS哈希值来检索文件。

SOLC的发布版本使用如上所示的3个字节的版本编码
（主要、次要和补丁版本号各一个字节），
而预发布版本将使用一个完整的版本字符串，包括提交哈希和构建日期。

.. note::
  CBOR映射也可以包含其他的键，所以最好是完全解码，
  而不是依靠它以 ``0xa264`` 开始。
  例如，如果使用了任何影响代码生成的实验性功能，
  映射也将包含 ``"experimental": true``。

.. note::
  编译器目前默认使用元数据的IPFS哈希值，
  但将来也可能使用bzzr1哈希值或其他哈希值，
  所以不要依赖这个序列以 ``0xa2 0x64 'i' 'p' 'f' 's'`` 开始。
  我们还可能向这个CBOR结构添加额外的数据，
  所以最好的选择是使用一个合适的CBOR解析器。


自动化接口生成和NatSpec 的使用方法
====================================================

元数据的使用方式如下：一个想要与合约交互的组件
（例如Mist或任何钱包）会检索合约的代码，
从中检索出一个文件的IPFS/Swarm哈希值，然后再检索。
该文件被JSON解码成一个类似于上述的结构。

然后，该组件可以使用ABI为合约自动生成一个基本的用户界面。

此外，钱包可以使用NatSpec用户文档，每当用户与合约交互时，
就会向用户显示一条确认信息，同时要求对交易签名进行授权。

有关其他信息，请阅读 :doc:`以太坊自然语言规范（NatSpec）格式 <natspec-format>`。

源代码验证的用法
==================================

为了验证编译，可以通过元数据文件中的链接从IPFS/Swarm检索源码。
正确版本的编译器（应该为“官方”编译器之一）以指定的设置在该输入上被调用。
产生的字节码与创建交易的数据或 ``CREATE`` 操作码数据进行比较。
这将自动验证元数据，因为其哈希值是字节码的一部分。
多余的数据对应于构造器的输入数据，应该根据接口进行解码并呈现给用户。

在资源库 `sourcify <https://github.com/ethereum/sourcify>`_
(`npm package <https://www.npmjs.com/package/source-verify>`_)，
您可以看到如何使用这一功能的示例代码。
