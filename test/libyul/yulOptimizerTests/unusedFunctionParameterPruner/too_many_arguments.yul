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
//     function f_1(x1, x2, x3_1, x4, x5, x6, x7_1, x8, x9, x10, x11, x12, x13, x14, x15, x16, x17, x18) -> y_1, z_1
//     { y_1, z_1 := f(x3_1, x7_1) }
// }
