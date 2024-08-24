{
    function g() -> a, b {
        let x, y := g()
        a := x
        b := y
    }
}
// ----
// step: constantFunctionEvaluator
//
// {
//     function g() -> a, b
//     {
//         let x, y := g()
//         a := x
//         b := y
//     }
// }
