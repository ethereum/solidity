{
    function f() -> x {
        let t := 0
        x := 1
        let a := 0
        let b := 1
    }
}
// ----
// step: commonSubexpressionEliminator
//
// {
//     function f() -> x
//     {
//         let t := 0
//         x := 1
//         let a := t
//         let b := x
//     }
// }
