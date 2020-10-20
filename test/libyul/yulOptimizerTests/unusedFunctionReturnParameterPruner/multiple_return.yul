{
    // b and c are unused
    let a, b, c := f(sload(0))
    sstore(a, 0)
    function f(d) -> x, y, z
    {
       y := mload(d)
       z := mload(2)
    }
}
// ----
// step: unusedFunctionReturnParameterPruner
//
// {
//     let a, b, c := f_1(sload(0))
//     sstore(a, 0)
//     function f(d) -> x
//     {
//         let y
//         let z
//         y := mload(d)
//         z := mload(2)
//     }
//     function f_1(d_2) -> x_3, y_4, z_5
//     { x_3 := f(d_2) }
// }
