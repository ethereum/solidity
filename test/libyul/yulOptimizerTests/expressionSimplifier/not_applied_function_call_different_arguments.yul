{
    function f(a) -> b { }
    let c := sub(f(0), f(1))
    sstore(0, c)
}
// ====
// EVMVersion: >=shanghai
// ----
// step: expressionSimplifier
//
// {
//     { sstore(0, sub(f(0), f(1))) }
//     function f(a) -> b
//     { }
// }
