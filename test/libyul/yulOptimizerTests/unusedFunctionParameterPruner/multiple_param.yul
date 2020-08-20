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
//     let d, e, i := f_7(1, 2, 3)
//     sstore(d, 0)
//     function f(a, c)
//     {
//         let y, z
//         y := mload(a)
//         z := mload(c)
//     }
//     function f_7(a_8, b_9, c_10) -> x_11, y_12, z_13
//     { f(a_8, c_10) }
// }
