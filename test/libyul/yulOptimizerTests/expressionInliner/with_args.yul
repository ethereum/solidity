{
    function f(a:u256) -> x:u256 { x := a }
    let y:u256 := f(7:u256)
}
// ====
// dialect: yul
// ----
// step: expressionInliner
//
// {
//     function f(a) -> x
//     { x := a }
//     let y := 7
// }
