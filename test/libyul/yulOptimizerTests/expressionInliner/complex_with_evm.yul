{
    function f(a) -> x { x := add(a, a) }
    let y := f(calldatasize())
}
// ----
// expressionInliner
// {
//     function f(a) -> x
//     {
//         x := add(a, a)
//     }
//     let y := add(calldatasize(), calldatasize())
// }
