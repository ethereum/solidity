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
//     let a, b := f_21(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18)
//     sstore(a, b)
//     function f(x3, x7) -> y, z
//     {
//         y := mload(x3)
//         z := mload(x7)
//     }
//     function f_21(x1_22, x2_23, x3_24, x4_25, x5_26, x6_27, x7_28, x8_29, x9_30, x10_31, x11_32, x12_33, x13_34, x14_35, x15_36, x16_37, x17_38, x18_39) -> y_40, z_41
//     { y_40, z_41 := f(x3_24, x7_28) }
// }
