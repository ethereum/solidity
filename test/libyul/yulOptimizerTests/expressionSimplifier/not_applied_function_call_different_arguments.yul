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
//         let _2 := f(1)
//         let _3 := 0
//         sstore(_3, sub(f(_3), _2))
//     }
//     function f(a) -> b
//     { }
// }
