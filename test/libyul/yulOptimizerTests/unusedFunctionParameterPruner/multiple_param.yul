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
//     function f_1(a_1, b, c_1) -> x, y_1, z_1
//     { y_1, z_1 := f(a_1, c_1) }
// }
