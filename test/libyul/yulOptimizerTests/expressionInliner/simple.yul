// yul
{
    function f() -> x:u256 { x := 2:u256 }
    let y:u256 := f()
}
// ----
// expressionInliner
// {
//     function f() -> x:u256
//     {
//         x := 2:u256
//     }
//     let y:u256 := 2:u256
// }
