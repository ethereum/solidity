{
    {
        mstore(0x40, memoryguard(128))
        let a_1 := 1
        let a_2 := 2
        let a_3 := 3
        let a_4 := 4
        let a_5 := 5
        let a_6 := 6
        let a_7 := 7
        let a_8 := 8
        let a_9 := 9
        let a_10 := 10
        let a_11 := 11
        let a_12 := 12
        let a_13 := 13
        let a_14 := 14
        let a_15 := 15
        let a_16 := 16
        let a_17 := 17
        let a_18 := 18
        let a_19 := 19
        let a_20 := 20
        let b_1, b_2, b_3, b_4, b_5, b_6, b_7, b_8, b_9, b_10, b_11, b_12, b_13, b_14, b_15, b_16, b_17, b_18, b_19, b_20 := verbatim_20i_20o("test", a_1, a_2, a_3, a_4, a_5, a_6, a_7, a_8, a_9, a_10, a_11, a_12, a_13, a_14, a_15, a_16, a_17, a_18, a_19, a_20)
        sstore(1, b_1)
        sstore(2, b_2)
        sstore(3, b_3)
        sstore(4, b_4)
        sstore(5, b_5)
        sstore(6, b_6)
        sstore(7, b_7)
        sstore(8, b_8)
        sstore(9, b_9)
        sstore(10, b_10)
        sstore(11, b_11)
        sstore(12, b_12)
        sstore(13, b_13)
        sstore(14, b_14)
        sstore(15, b_15)
        sstore(16, b_16)
        sstore(17, b_17)
        sstore(18, b_18)
        sstore(19, b_19)
        sstore(20, b_20)

    }
}
// ----
// step: stackLimitEvader
//
// {
//     {
//         mstore(0x40, memoryguard(0x0280))
//         mstore(0x0100, 1)
//         mstore(0x0120, 2)
//         mstore(0x0140, 3)
//         mstore(0x0160, 4)
//         mstore(0x0180, 5)
//         mstore(0x01a0, 6)
//         mstore(0x01c0, 7)
//         mstore(0x01e0, 8)
//         mstore(0x0200, 9)
//         mstore(0x0220, 10)
//         mstore(0x0240, 11)
//         mstore(0x0260, 12)
//         let a_13 := 13
//         let a_14 := 14
//         let a_15 := 15
//         let a_16 := 16
//         let a_17 := 17
//         let a_18 := 18
//         let a_19 := 19
//         let a_20 := 20
//         let b_1_1, b_2_2, b_3_3, b_4_4, b_5_5, b_6_6, b_7_7, b_8_8, b_9_9, b_10_10, b_11_11, b_12_12, b_13_13, b_14_14, b_15_15, b_16_16, b_17_17, b_18_18, b_19_19, b_20_20 := verbatim_20i_20o("test", mload(0x0100), mload(0x0120), mload(0x0140), mload(0x0160), mload(0x0180), mload(0x01a0), mload(0x01c0), mload(0x01e0), mload(0x0200), mload(0x0220), mload(0x0240), mload(0x0260), a_13, a_14, a_15, a_16, a_17, a_18, a_19, a_20)
//         mstore(0x80, b_4_4)
//         mstore(0xa0, b_3_3)
//         mstore(0xc0, b_2_2)
//         mstore(0xe0, b_1_1)
//         let b_20 := b_20_20
//         let b_19 := b_19_19
//         let b_18 := b_18_18
//         let b_17 := b_17_17
//         let b_16 := b_16_16
//         let b_15 := b_15_15
//         let b_14 := b_14_14
//         let b_13 := b_13_13
//         let b_12 := b_12_12
//         let b_11 := b_11_11
//         let b_10 := b_10_10
//         let b_9 := b_9_9
//         let b_8 := b_8_8
//         let b_7 := b_7_7
//         let b_6 := b_6_6
//         let b_5 := b_5_5
//         sstore(1, mload(0xe0))
//         sstore(2, mload(0xc0))
//         sstore(3, mload(0xa0))
//         sstore(4, mload(0x80))
//         sstore(5, b_5)
//         sstore(6, b_6)
//         sstore(7, b_7)
//         sstore(8, b_8)
//         sstore(9, b_9)
//         sstore(10, b_10)
//         sstore(11, b_11)
//         sstore(12, b_12)
//         sstore(13, b_13)
//         sstore(14, b_14)
//         sstore(15, b_15)
//         sstore(16, b_16)
//         sstore(17, b_17)
//         sstore(18, b_18)
//         sstore(19, b_19)
//         sstore(20, b_20)
//     }
// }
