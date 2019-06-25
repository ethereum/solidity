{
    let or_bool := 2
    if or_bool { sstore(0, 1) }
}
// ====
// step: wordSizeTransform
// ----
// {
//     let or_bool_3_0 := 0
//     let or_bool_3_1 := 0
//     let or_bool_3_2 := 0
//     let or_bool_3_3 := 2
//     if or_bool(or_bool_3_0, or_bool_3_1, or_bool_3_2, or_bool_3_3)
//     {
//         let _1_0 := 0
//         let _1_1 := 0
//         let _1_2 := 0
//         let _1_3 := 1
//         let _2_0 := 0
//         let _2_1 := 0
//         let _2_2 := 0
//         let _2_3 := 0
//         sstore(_2_0, _2_1, _2_2, _2_3, _1_0, _1_1, _1_2, _1_3)
//     }
// }
