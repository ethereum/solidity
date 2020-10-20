{
    let a := f()
    // Return value of f is used here. So f cannot be rewritten.
    sstore(sload(f()), 2)
    function f() -> x
    {
        x := sload(1)
        sstore(x, x)
    }
}
// ----
// step: unusedFunctionReturnParameterPruner
//
// {
//     let a := f()
//     sstore(sload(f()), 2)
//     function f() -> x
//     {
//         x := sload(1)
//         sstore(x, x)
//     }
// }
