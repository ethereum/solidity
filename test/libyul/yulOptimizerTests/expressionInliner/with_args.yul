{
    function f(a) -> x { x := a }
    let y := f(7)
}
// ----
// step: expressionInliner
//
// {
//     function f(a) -> x
//     { x := a }
//     let y := 7
// }
