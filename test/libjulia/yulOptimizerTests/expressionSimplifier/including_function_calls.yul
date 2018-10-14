{
    function f() -> a {}
    let b := add(7, sub(f(), 7))
}
// ----
// expressionSimplifier
// {
//     function f() -> a
//     {
//     }
//     let b := f()
// }
