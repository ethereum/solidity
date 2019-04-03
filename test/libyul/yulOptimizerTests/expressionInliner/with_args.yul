{
    function f(a:u256) -> x:u256 { x := a }
    let y:u256 := f(7:u256)
}
// ====
// step: expressionInliner
// yul: true
// ----
// {
//     function f(a:u256) -> x:u256
//     {
//         x := a
//     }
//     let y:u256 := 7:u256
// }
