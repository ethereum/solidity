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
//         let a1, a2_1, a3_1, a4_1, a5_1, a6_1, a7_1, a8_1, a9_1, a10_1, a13_1, a14_1, a15_1, a16_1, a17_1, a18_1 := verbatim_0i_16o("test")
//         mstore(0x80, a1)
//         let a18 := a18_1
//         let a17 := a17_1
//         let a16 := a16_1
//         let a15 := a15_1
//         let a14 := a14_1
//         let a13 := a13_1
//         let a10 := a10_1
//         let a9 := a9_1
//         let a8 := a8_1
//         let a7 := a7_1
//         let a6 := a6_1
//         let a5 := a5_1
//         let a4 := a4_1
//         let a3 := a3_1
//         let a2 := a2_1
//         let a1_1, a2_2, a3_2, a4_2, a5_2, a6_2, a7_2, a8_2, a9_2, a10_2, a13_2, a14_2, a15_2, a16_2, a17_2, a18_2 := verbatim_0i_16o("test")
//         mstore(0x80, a1_1)
//         a18 := a18_2
//         a17 := a17_2
//         a16 := a16_2
//         a15 := a15_2
//         a14 := a14_2
//         a13 := a13_2
//         a10 := a10_2
//         a9 := a9_2
//         a8 := a8_2
//         a7 := a7_2
//         a6 := a6_2
//         a5 := a5_2
//         a4 := a4_2
//         a3 := a3_2
//         a2 := a2_2
//         sstore(mload(0x80), 10)
//         sstore(a18, 20)
//     }
// }
