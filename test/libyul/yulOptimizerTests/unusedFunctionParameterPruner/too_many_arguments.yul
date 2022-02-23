{
    let a, b := f(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18)
    sstore(a, b)
    function f(x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, x12, x13, x14, x15, x16, x17, x18) -> y, z
    {
       y := mload(x3)
       z := mload(x7)
    }
}
// ----
// step: unusedFunctionParameterPruner
//
// {
//     let a, b := f_1(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18)
//     sstore(a, b)
//     function f(x3, x7) -> y, z
//     {
//         y := mload(x3)
//         z := mload(x7)
//     }
//     function f_1(x1_2, x2_3, x3_4, x4_5, x5_6, x6_7, x7_8, x8_9, x9_10, x10_11, x11_12, x12_13, x13_14, x14_15, x15_16, x16_17, x17_18, x18_19) -> y_20, z_21
//     { y_20, z_21 := f(x3_4, x7_8) }
// }
