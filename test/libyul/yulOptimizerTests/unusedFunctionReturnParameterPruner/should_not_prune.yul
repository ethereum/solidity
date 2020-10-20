// An example where the return parameter of the function should not be pruned
{
    function f() -> y {
        y := sload(1)
    }

    // return value is unused here
    let x := f()
    // return value is used here, so f cannot be pruned.
    sstore(1, f())
}
// ----
// step: unusedFunctionReturnParameterPruner
//
// {
//     let x := f()
//     sstore(1, f())
//     function f() -> y
//     { y := sload(1) }
// }
