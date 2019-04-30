{
    function f(a) -> b { }
    let c := sub(f(0), f(1))
}
// ====
// step: expressionSimplifier
// ----
// {
//     function f(a) -> b
//     {
//     }
//     let c := sub(f(0), f(1))
// }
