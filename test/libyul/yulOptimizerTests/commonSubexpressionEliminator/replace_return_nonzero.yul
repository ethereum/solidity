{
    function f() -> x {
        let t := 0
        x := 1
        let a := 0
        let b := 1
    }
}
// ====
// EVMVersion: >=shanghai
// ----
// step: commonSubexpressionEliminator
//
// {
//     function f() -> x
//     {
//         let t := 0
//         x := 1
//         let a := 0
//         let b := x
//     }
// }
