{
    let a, b, c := f(sload(0))
    function f(d) -> x, y, z
    {
       y := mload(d)
       z := mload(2)
       sstore(y, z)
    }
}
// ----
// step: unusedFunctionReturnParameterPruner
//
// {
//     let a, b, c := f_1(sload(0))
//     function f(d)
//     {
//         let x
//         let y
//         let z
//         y := mload(d)
//         z := mload(2)
//         sstore(y, z)
//     }
//     function f_1(d_2) -> x_3, y_4, z_5
//     { f(d_2) }
// }
