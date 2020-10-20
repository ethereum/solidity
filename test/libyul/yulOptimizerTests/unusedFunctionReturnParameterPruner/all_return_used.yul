// A test where the function should not be rewritten, since each parameter is used eventually.
{
    let a, b, c := f(sload(0))
    sstore(a, 0)
    let a1, b1, c1 := f(sload(2))
    sstore(b1, 0)
    let a2, b2, c3 := f(sload(3))
    sstore(c3, 0)
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
//     let a, b, c := f(sload(0))
//     sstore(a, 0)
//     let a1, b1, c1 := f(sload(2))
//     sstore(b1, 0)
//     let a2, b2, c3 := f(sload(3))
//     sstore(c3, 0)
//     function f(d) -> x, y, z
//     {
//         y := mload(d)
//         z := mload(2)
//     }
// }
