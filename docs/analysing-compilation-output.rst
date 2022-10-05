.. index:: analyse, asm

#############################
分析编译器的输出结果
#############################

看一下编译器生成的汇编代码往往是有用的。生成的二进制文件，
即 ``solc --bin contract.sol`` 的输出，通常很难阅读。
建议使用标志 ``--asm`` 来分析汇编输出。
即使是很大的合约，看一下改变前后的汇编结果的差异，往往是很有启发的。

以下合约（命名为 ``contract.sol`` ）为例：

.. code-block:: Solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.5.0 <0.9.0;
    contract C {
        function one() public pure returns (uint) {
            return 1;
        }
    }

以下是 ``solc --asm contract.sol`` 的输出

.. code-block:: none

    ======= contract.sol:C =======
    EVM assembly:
        /* "contract.sol":0:86  contract C {... */
      mstore(0x40, 0x80)
      callvalue
      dup1
      iszero
      tag_1
      jumpi
      0x00
      dup1
      revert
    tag_1:
      pop
      dataSize(sub_0)
      dup1
      dataOffset(sub_0)
      0x00
      codecopy
      0x00
      return
    stop

    sub_0: assembly {
            /* "contract.sol":0:86  contract C {... */
          mstore(0x40, 0x80)
          callvalue
          dup1
          iszero
          tag_1
          jumpi
          0x00
          dup1
          revert
        tag_1:
          pop
          jumpi(tag_2, lt(calldatasize, 0x04))
          shr(0xe0, calldataload(0x00))
          dup1
          0x901717d1
          eq
          tag_3
          jumpi
        tag_2:
          0x00
          dup1
          revert
            /* "contract.sol":17:84  function one() public pure returns (uint) {... */
        tag_3:
          tag_4
          tag_5
          jump	// in
        tag_4:
          mload(0x40)
          tag_6
          swap2
          swap1
          tag_7
          jump	// in
        tag_6:
          mload(0x40)
          dup1
          swap2
          sub
          swap1
          return
        tag_5:
            /* "contract.sol":53:57  uint */
          0x00
            /* "contract.sol":76:77  1 */
          0x01
            /* "contract.sol":69:77  return 1 */
          swap1
          pop
            /* "contract.sol":17:84  function one() public pure returns (uint) {... */
          swap1
          jump	// out
            /* "#utility.yul":7:125   */
        tag_10:
            /* "#utility.yul":94:118   */
          tag_12
            /* "#utility.yul":112:117   */
          dup2
            /* "#utility.yul":94:118   */
          tag_13
          jump	// in
        tag_12:
            /* "#utility.yul":89:92   */
          dup3
            /* "#utility.yul":82:119   */
          mstore
            /* "#utility.yul":72:125   */
          pop
          pop
          jump	// out
            /* "#utility.yul":131:353   */
        tag_7:
          0x00
            /* "#utility.yul":262:264   */
          0x20
            /* "#utility.yul":251:260   */
          dup3
            /* "#utility.yul":247:265   */
          add
            /* "#utility.yul":239:265   */
          swap1
          pop
            /* "#utility.yul":275:346   */
          tag_15
            /* "#utility.yul":343:344   */
          0x00
            /* "#utility.yul":332:341   */
          dup4
            /* "#utility.yul":328:345   */
          add
            /* "#utility.yul":319:325   */
          dup5
            /* "#utility.yul":275:346   */
          tag_10
          jump	// in
        tag_15:
            /* "#utility.yul":229:353   */
          swap3
          swap2
          pop
          pop
          jump	// out
            /* "#utility.yul":359:436   */
        tag_13:
          0x00
            /* "#utility.yul":425:430   */
          dup2
            /* "#utility.yul":414:430   */
          swap1
          pop
            /* "#utility.yul":404:436   */
          swap2
          swap1
          pop
          jump	// out

        auxdata: 0xa2646970667358221220a5874f19737ddd4c5d77ace1619e5160c67b3d4bedac75fce908fed32d98899864736f6c637827302e382e342d646576656c6f702e323032312e332e33302b636f6d6d69742e65613065363933380058
    }

另外，上述输出也可以从 `Remix <https://remix.ethereum.org/>`_ ，
在编译合约后的 "编译细节" 选项下获得。

请注意， ``asm`` 输出以创建/构造器代码开始。
部署代码是作为子对象的一部分提供的（在上面的例子中，它是子对象 ``sub_0`` 的一部分）。
``auxdata`` 字段对应于合约 :ref:`元数据 <encoding-of-the-metadata-hash-in-the-bytecode>` 。
汇编输出中的注释指向源文件的位置。注意 ``#utility.yul`` 是一个内部生成的实用函数文件，
可以使用标志 ``--combined-json generated-sources,generated-sources-runtime`` 获得。

类似地，可以通过 ``solc --optimize --asm contract.sol`` 命令获得优化后的程序集。
通常情况下，观察两个不同的Solidity源是否会产生相同的优化代码是很有趣的。
例如，查看表达式 ``(a * b) / c``， ``a * b / c`` 是否生成相同的字节码。
在可能的话，在剥离引用源位置的注释之后，通过获取相应程序集输出的 ``diff`` 很容易做到这一点。

.. note::

   ``--asm`` 的输出不是设计成机器可读的。因此，在solc的各个小版本之间，输出可能会有重大的变化。
