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
        verbatim_20i_0o("test", a_1, a_2, a_3, a_4, a_5, a_6, a_7, a_8, a_9, a_10, a_11, a_12, a_13, a_14, a_15, a_16, a_17, a_18, a_19, a_20)
    }
}
// ----
// step: stackLimitEvader
//
// {
//     {
//         mstore(0x40, memoryguard(0x0200))
//         mstore(0x80, 1)
//         mstore(0xa0, 2)
//         mstore(0xc0, 3)
//         mstore(0xe0, 4)
//         mstore(0x0100, 5)
//         mstore(0x0120, 6)
//         mstore(0x0140, 7)
//         mstore(0x0160, 8)
//         mstore(0x0180, 9)
//         mstore(0x01a0, 10)
//         mstore(0x01c0, 11)
//         mstore(0x01e0, 12)
//         let a_13 := 13
//         let a_14 := 14
//         let a_15 := 15
//         let a_16 := 16
//         let a_17 := 17
//         let a_18 := 18
//         let a_19 := 19
//         let a_20 := 20
//         verbatim_20i_0o("test", mload(0x80), mload(0xa0), mload(0xc0), mload(0xe0), mload(0x0100), mload(0x0120), mload(0x0140), mload(0x0160), mload(0x0180), mload(0x01a0), mload(0x01c0), mload(0x01e0), a_13, a_14, a_15, a_16, a_17, a_18, a_19, a_20)
//     }
// }
