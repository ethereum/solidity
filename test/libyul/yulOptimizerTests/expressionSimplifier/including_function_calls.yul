{
    function f() -> a {}
    let b := add(7, sub(f(), 7))
}
// ====
// step: expressionSimplifier
// ----
// {
//     function f() -> a
//     {
//     }
//     let b := f()
// }
