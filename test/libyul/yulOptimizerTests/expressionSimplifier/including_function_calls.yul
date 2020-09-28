{
    function f() -> a {}
    let b := add(7, sub(f(), 7))
    sstore(0, b)
}
// ----
// step: expressionSimplifier
//
// {
//     function f() -> a
//     { }
//     sstore(0, f())
// }
