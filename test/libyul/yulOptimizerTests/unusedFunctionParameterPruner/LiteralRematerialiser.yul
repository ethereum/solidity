{
    let a := f(mload(1))
    let b := f(a)
    sstore(a, b)
    function f(x) -> y
    {
        // Test if LiteralRematerializer can convert `y` to `0` and therefore allowing us to
        // rewrite the function f
        if iszero(x) { revert(y, y)}
        if iszero(add(x, 1)) { revert(y, y)}
    }
}
// ----
// step: unusedFunctionParameterPruner
//
// {
//     let a := f_11(mload(1))
//     sstore(a, f_11(a))
//     function f(x)
//     {
//         if iszero(x) { revert(0, 0) }
//         if iszero(add(x, 1)) { revert(0, 0) }
//     }
//     function f_11(x_12) -> y_13
//     { f(x_12) }
// }
