{
    {
        mstore(0x40, memoryguard(128))
        let a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a13, a14, a15, a16, a17, a18 := verbatim_0i_16o("test")
        a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a13, a14, a15, a16, a17, a18 := verbatim_0i_16o("test")
        sstore(a1, 10)
        sstore(a18, 20)
    }
}
// ----
// step: stackLimitEvader
//
// {
//     {
//         mstore(0x40, memoryguard(0xa0))
//         let a1_1, a2_2, a3_3, a4_4, a5_5, a6_6, a7_7, a8_8, a9_9, a10_10, a13_11, a14_12, a15_13, a16_14, a17_15, a18_16 := verbatim_0i_16o("test")
//         mstore(0x80, a1_1)
//         let a18 := a18_16
//         let a17 := a17_15
//         let a16 := a16_14
//         let a15 := a15_13
//         let a14 := a14_12
//         let a13 := a13_11
//         let a10 := a10_10
//         let a9 := a9_9
//         let a8 := a8_8
//         let a7 := a7_7
//         let a6 := a6_6
//         let a5 := a5_5
//         let a4 := a4_4
//         let a3 := a3_3
//         let a2 := a2_2
//         let a1_17, a2_18, a3_19, a4_20, a5_21, a6_22, a7_23, a8_24, a9_25, a10_26, a13_27, a14_28, a15_29, a16_30, a17_31, a18_32 := verbatim_0i_16o("test")
//         mstore(0x80, a1_17)
//         a18 := a18_32
//         a17 := a17_31
//         a16 := a16_30
//         a15 := a15_29
//         a14 := a14_28
//         a13 := a13_27
//         a10 := a10_26
//         a9 := a9_25
//         a8 := a8_24
//         a7 := a7_23
//         a6 := a6_22
//         a5 := a5_21
//         a4 := a4_20
//         a3 := a3_19
//         a2 := a2_18
//         sstore(mload(0x80), 10)
//         sstore(a18, 20)
//     }
// }
