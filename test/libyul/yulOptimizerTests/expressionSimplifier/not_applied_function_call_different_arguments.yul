{
    function f(a) -> b { }
    let c := sub(f(0), f(1))
    sstore(0, c)
}
// ----
// step: expressionSimplifier
//
// {
//     {
//         let _1 := f(1)
//         let _2 := 0
//         sstore(_2, sub(f(_2), _1))
//     }
//     function f(a) -> b
//     { }
// }
