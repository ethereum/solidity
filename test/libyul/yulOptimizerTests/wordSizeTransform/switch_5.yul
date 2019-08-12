{
    switch calldataload(0)
    default { sstore(8, 9) }
}
// ====
// step: wordSizeTransform
// ----
// {
//     let _1_0 := 0
//     let _1_1 := 0
//     let _1_2 := 0
//     let _1_3 := 0
//     let _2_0, _2_1, _2_2, _2_3 := calldataload(_1_0, _1_1, _1_2, _1_3)
//     let run_default
//     switch _2_0
//     default { run_default := 1 }
//     if run_default
//     {
//         let _3_0 := 0
//         let _3_1 := 0
//         let _3_2 := 0
//         let _3_3 := 9
//         let _4_0 := 0
//         let _4_1 := 0
//         let _4_2 := 0
//         let _4_3 := 8
//         sstore(_4_0, _4_1, _4_2, _4_3, _3_0, _3_1, _3_2, _3_3)
//     }
// }
