object "C" {
    code {
        let g := memoryguard(0x80)
        mstore(0x40, g)
        f(calldataload(0x0), calldataload(0x20), calldataload(0x40), calldataload(0x60), calldataload(0x80), calldataload(0xa0), calldataload(0xc0), calldataload(0xe0), calldataload(0x100), calldataload(0x120), calldataload(0x140), calldataload(0x160), calldataload(0x180), calldataload(0x1a0), calldataload(0x1c0), calldataload(0x1e0), calldataload(0x200))
        function f(b1, b2, b3, b4, b5, b6, b7, b8, b9, b10, b11, b12, b13, b14, b15, b16, b17) {
            for { let i := 0 } lt(i, add(b1, b2)) { i := add(i, 1) } {
                sstore(i, b17)
                sstore(add(i, 1), b16)
                sstore(add(i, 2), b15)
                sstore(add(i, 3), b14)
                sstore(add(i, 4), b13)
                sstore(add(i, 5), b12)
                sstore(add(i, 6), b11)
                sstore(add(i, 7), b10)
                sstore(add(i, 8), b9)
                sstore(add(i, 9), b8)
                sstore(add(i, 10), b7)
                sstore(add(i, 11), b6)
                sstore(add(i, 12), b5)
                sstore(add(i, 13), b4)
                sstore(add(i, 14), b3)
                sstore(add(i, 15), b2)
                sstore(add(i, 16), b1)
            }
        }
    }
}
// ====
// EVMVersion: =current
// stackOptimization: true
// viaSSACFG: true
// ----
// tag_2:
//   mstore(0x40, 0xe0)
//   calldataload(0x0200)
//   calldataload(0x01e0)
//   calldataload(0x01c0)
//   calldataload(0x01a0)
//   calldataload(0x0180)
//   calldataload(0x0160)
//   calldataload(0x0140)
//   calldataload(0x0120)
//   calldataload(0x0100)
//   calldataload(0xe0)
//   calldataload(0xc0)
//   calldataload(0xa0)
//   calldataload(0x80)
//   calldataload(0x60)
//   calldataload(0x40)
//   calldataload(0x20)
//   calldataload(0x00)
//   dup1
//   0x80
//   mstore
//   pop
//   dup16
//   dup16
//   dup16
//   dup16
//   dup16
//   dup16
//   dup16
//   dup16
//   dup16
//   dup16
//   dup16
//   dup16
//   dup16
//   dup16
//   dup16
//   tag_3
//   swap16
//   mload(0x80)
//   tag_1
//   jump	// in
// tag_3:
//   stop
// tag_1:
//   dup1
//   0xa0
//   mstore
// tag_4:
//   0x00
//   jump(tag_5)
// tag_5:
//   dup1
//   0xc0
//   mstore
//   dup3
//   dup3
//   add
//   dup2
//   lt
//   tag_6
//   jumpi
//   jump(tag_8)
// tag_8:
//   pop
//   pop
//   pop
//   pop
//   pop
//   pop
//   pop
//   pop
//   pop
//   pop
//   pop
//   pop
//   pop
//   pop
//   pop
//   pop
//   pop
//   pop
//   jump	// out
// tag_6:
//   pop
//   pop
//   dup16
//   dup1
//   dup1
//   mload(0xc0)
//   sstore
//   add(mload(0xc0), 0x01)
//   swap2
//   pop
//   pop
//   dup16
//   dup1
//   dup3
//   sstore
//   add(mload(0xc0), 0x02)
//   swap1
//   pop
//   dup16
//   dup2
//   sstore
//   add(mload(0xc0), 0x03)
//   dup16
//   swap1
//   sstore
//   add(mload(0xc0), 0x04)
//   dup15
//   swap1
//   sstore
//   add(mload(0xc0), 0x05)
//   dup14
//   swap1
//   sstore
//   add(mload(0xc0), 0x06)
//   dup13
//   swap1
//   sstore
//   add(mload(0xc0), 0x07)
//   dup12
//   swap1
//   sstore
//   add(mload(0xc0), 0x08)
//   dup11
//   swap1
//   sstore
//   add(mload(0xc0), 0x09)
//   dup10
//   swap1
//   sstore
//   add(mload(0xc0), 0x0a)
//   dup9
//   swap1
//   sstore
//   add(mload(0xc0), 0x0b)
//   dup8
//   swap1
//   sstore
//   add(mload(0xc0), 0x0c)
//   dup7
//   swap1
//   sstore
//   add(mload(0xc0), 0x0d)
//   dup6
//   swap1
//   sstore
//   add(mload(0xc0), 0x0e)
//   dup5
//   swap1
//   sstore
//   add(mload(0xc0), 0x0f)
//   dup4
//   swap1
//   sstore
//   add(mload(0xc0), 0x10)
//   mload(0xa0)
//   swap1
//   sstore
//   add(mload(0xc0), 0x01)
//   swap1
//   pop
//   swap1
//   pop
//   mload(0xa0)
//   swap1
//   jump(tag_5)
