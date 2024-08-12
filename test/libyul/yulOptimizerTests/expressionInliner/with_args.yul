{
    function f(a:u256) -> x:u256 { x := a }
    let y:u256 := f(7:u256)
}
// ====
// dialect: evmTyped
// ----
// step: expressionInliner
//
// {
//     function f(a) -> x
//     { x := a }
//     let y := 7
// }
