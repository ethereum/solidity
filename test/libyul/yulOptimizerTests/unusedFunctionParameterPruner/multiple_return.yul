{
    let a, b, c := f(sload(0))
    sstore(a, 0)
    // d is unused, x is unassigned
    function f(d) -> x, y, z
    {
       y := mload(1)
       z := mload(2)
    }
}
// ----
// step: unusedFunctionParameterPruner
//
// {
//     let a, b, c := f_8(sload(0))
//     sstore(a, 0)
//     function f()
//     {
//         let y, z
//         y := mload(1)
//         z := mload(2)
//     }
//     function f_8(d_9) -> x_10, y_11, z_12
//     { f() }
// }
