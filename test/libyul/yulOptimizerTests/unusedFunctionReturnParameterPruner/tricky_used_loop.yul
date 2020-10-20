// A test to see if loop condition is properly split
{
    let b := f()
    // Return value of f is used in for loop condition. So f cannot be rewritten.
    for {let a := 1} iszero(sub(f(), a)) {a := add(a, 1)}
    {}
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
//     let b := f()
//     for { let a := 1 } iszero(sub(f(), a)) { a := add(a, 1) }
//     { }
//     function f() -> x
//     {
//         x := sload(1)
//         sstore(x, x)
//     }
// }
