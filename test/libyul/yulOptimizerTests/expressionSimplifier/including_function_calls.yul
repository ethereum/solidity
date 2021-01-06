{
    function f() -> a {}
    let b := add(7, sub(f(), 7))
    sstore(0, b)
}
// ----
// step: expressionSimplifier
//
// {
//     sstore(0, f())
//     function f() -> a
//     { }
// }
