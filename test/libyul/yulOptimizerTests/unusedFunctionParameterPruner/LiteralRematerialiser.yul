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
//     let a := f_1(mload(1))
//     let b := f_1(a)
//     sstore(a, b)
//     function f(x)
//     {
//         if iszero(x) { revert(0, 0) }
//         if iszero(add(x, 1)) { revert(0, 0) }
//     }
//     function f_1(x_2) -> y_3
//     { f(x_2) }
// }
