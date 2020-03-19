{
    function f() -> x:u256 { x := 2:u256 }
    let y:u256 := f()
}
// ====
// dialect: yul
// ----
// step: expressionInliner
//
// {
//     function f() -> x
//     { x := 2 }
//     let y := 2
// }
