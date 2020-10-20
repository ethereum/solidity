// A test to see if loop condition is properly split
{
    let b := f()
    // Return value of f is used in for loop condition. So f cannot be rewritten,
    // unless LoopConditionIntoBody and ExpressionSplitter is run.
    for {let a := 1} iszero(sub(f(), a)) {a := add(a, 1)}
    {}
    function f() -> x
    {
        x := sload(1)
        sstore(x, x)
        if calldataload(0) { leave }
    }
}
// ----
// step: fullSuite
//
// {
//     {
//         pop(f())
//         let a := 1
//         let a_1 := a
//         for { } true { a_1 := add(a_1, a) }
//         {
//             if iszero(iszero(sub(f(), a_1))) { break }
//         }
//     }
//     function f() -> x
//     {
//         x := sload(1)
//         sstore(x, x)
//         if calldataload(0) { leave }
//     }
// }
