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
//     let a, b, c := f_1(sload(0))
//     sstore(a, 0)
//     function f() -> y, z
//     {
//         y := mload(1)
//         z := mload(2)
//     }
//     function f_1(d) -> x, y_1, z_1
//     { y_1, z_1 := f() }
// }
