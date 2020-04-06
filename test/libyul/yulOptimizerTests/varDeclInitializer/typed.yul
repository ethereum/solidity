{
    let a1
    let a2: bool
    let b1, b2: bool
    function f(a:u256, b:u256, c:bool) -> r:bool, t {
        let x1: bool, x2
    }
}
// ====
// dialect: evmTyped
// ----
// step: varDeclInitializer
//
// {
//     let a1 := 0
//     let a2:bool := false
//     let b1 := 0
//     let b2:bool := false
//     function f(a, b, c:bool) -> r:bool, t
//     {
//         let x1:bool := false
//         let x2 := 0
//     }
// }
