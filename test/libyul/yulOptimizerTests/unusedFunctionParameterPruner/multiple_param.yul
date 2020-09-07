{
    let d, e, i := f(1, 2, 3)
    sstore(d, 0)
    // b and x are unused
    function f(a, b, c) -> x, y, z
    {
       y := mload(a)
       z := mload(c)
    }
}
// ----
// step: unusedFunctionParameterPruner
//
// {
//     let d, e, i := f_1(1, 2, 3)
//     sstore(d, 0)
//     function f(a, c) -> y, z
//     {
//         y := mload(a)
//         z := mload(c)
//     }
//     function f_1(a_2, b_3, c_4) -> x_5, y_6, z_7
//     { y_6, z_7 := f(a_2, c_4) }
// }
