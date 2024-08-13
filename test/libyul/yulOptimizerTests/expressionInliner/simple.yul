{
    function f() -> x { x := 2 }
    let y := f()
}
// ----
// step: expressionInliner
//
// {
//     function f() -> x
//     { x := 2 }
//     let y := 2
// }
