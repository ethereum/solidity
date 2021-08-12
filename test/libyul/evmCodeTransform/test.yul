{
    pop(abi_encode_array_array_array_uint256_dyn(0,0))
    function abi_encode_array_array_array_uint256_dyn(value_92, pos_93) -> end_94
    {
        let pos_93_742 := pos_93
        pos_93 := pos_93
        let tail := add(pos_93_742, 96)
        let srcPtr := value_92
        let i_97 := 0
        for { } lt(i_97, 0x03) { i_97 := add(i_97, 1) }
        {
            mstore(pos_93, sub(tail, pos_93_742))
            let _579 := mload(srcPtr)
            let pos_109_1625 := tail
            let length_111_1629 := mload(_579)
            mstore(tail, length_111_1629)
            let _6753 := 0x20
            pos_109_1625 := add(tail, _6753)
            let srcPtr_113_1641 := add(_579, _6753)
            let i_114_1644 := 0
            for { }
            lt(i_114_1644, length_111_1629)
            {
                i_114_1644 := add(i_114_1644, 1)
            }
            {
                let _596_1660 := mload(srcPtr_113_1641)
                let pos_125_1170_1382_1665 := pos_109_1625
                pos_125_1170_1382_1665 := pos_109_1625
                let srcPtr_128_1178_1390_1673 := _596_1660
                let i_129_1181_1393_1676 := 0
                for { }
                lt(i_129_1181_1393_1676, 0x02)
                {
                    i_129_1181_1393_1676 := add(i_129_1181_1393_1676, 1)
                }
                {
                    mstore(pos_125_1170_1382_1665, mload(srcPtr_128_1178_1390_1673))
                    pos_125_1170_1382_1665 := add(pos_125_1170_1382_1665, _6753)
                    srcPtr_128_1178_1390_1673 := add(srcPtr_128_1178_1390_1673, _6753)
                }
                pos_109_1625 := add(pos_109_1625, 0x40)
                srcPtr_113_1641 := add(srcPtr_113_1641, _6753)
            }
            tail := pos_109_1625
            srcPtr := add(srcPtr, _6753)
            pos_93 := add(pos_93, _6753)
        }
        end_94 := tail
    }

}
// ====
// stackOptimization: true
// ----
//     /* "":10:55   */
//   tag_1
//     /* "":53:54   */
//   0x00
//   dup1
//     /* "":10:55   */
//   tag_2
//   jump	// in
// tag_1:
//     /* "":6:56   */
//   pop
//   stop
//     /* "":61:1920   */
// tag_2:
//   swap1
//   dup1
//   swap1
//   swap1
//     /* "":239:241   */
//   0x60
//   dup2
//     /* "":223:242   */
//   add
//   swap3
//   swap2
//     /* "":294:295   */
//   0x00
//   swap1
// tag_3:
//     /* "":321:325   */
//   0x03
//   dup3
//     /* "":312:326   */
//   lt
//   tag_4
//   jumpi
// tag_5:
//   pop
//   pop
//   pop
//   pop
//     /* "":61:1920   */
//   swap1
//   jump	// out
// tag_4:
//   swap1
//   swap2
//   swap3
//   swap4
//   dup4
//   dup2
//     /* "":389:410   */
//   sub
//   dup3
//     /* "":374:411   */
//   mstore
//   dup5
//     /* "":436:449   */
//   mload
//   dup2
//   pop
//   dup1
//     /* "":522:533   */
//   mload
//   dup1
//   dup4
//     /* "":546:575   */
//   mstore
//     /* "":601:605   */
//   0x20
//   dup1
//   dup1
//   swap5
//     /* "":634:650   */
//   add
//   swap3
//     /* "":686:702   */
//   add
//   swap1
//     /* "":733:734   */
//   0x00
//   swap1
// tag_6:
//   dup1
//   dup3
//     /* "":767:798   */
//   lt
//   tag_7
//   jumpi
// tag_8:
//   pop
//   pop
//   pop
//   swap1
//   dup1
//     /* "":347:348   */
//   0x01
//   swap3
//   swap7
//     /* "":1822:1840   */
//   add
//   swap3
//     /* "":1863:1881   */
//   add
//   swap3
//     /* "":337:349   */
//   add
//   swap1
//   swap3
//   swap2
//   jump(tag_3)
// tag_7:
//   swap1
//   swap2
//   swap3
//   dup5
//   dup5
//     /* "":923:945   */
//   mload
//   dup3
//   pop
//   dup3
//   swap1
//   swap1
//     /* "":1163:1164   */
//   0x00
//   swap1
// tag_9:
//     /* "":1230:1234   */
//   0x02
//   dup3
//     /* "":1205:1235   */
//   lt
//   tag_10
//   jumpi
// tag_11:
//   pop
//   pop
//   pop
//     /* "":1684:1688   */
//   0x40
//     /* "":859:860   */
//   0x01
//   swap3
//     /* "":1666:1689   */
//   add
//   swap5
//     /* "":1725:1752   */
//   add
//   swap3
//     /* "":843:861   */
//   add
//   swap1
//   jump(tag_6)
// tag_10:
//   swap3
//   dup1
//   dup4
//   swap5
//     /* "":1324:1325   */
//   0x01
//   swap4
//   swap5
//     /* "":1414:1446   */
//   mload
//   dup2
//     /* "":1383:1447   */
//   mstore
//     /* "":1494:1528   */
//   add
//   swap4
//     /* "":1578:1615   */
//   add
//   swap2
//     /* "":1298:1326   */
//   add
//   dup8
//   swap3
//   jump(tag_9)
