{
    let r
    r := 1
    function f(x, y) -> a, b {
        // Can be removed, is param
        x := 1
        y := 2
        // Cannot be removed, is return param
        a := 3
        b := 4
    }
    r := 2
}
// ----
// redundantAssignEliminator
// {
//     let r
//     function f(x, y) -> a, b
//     {
//         a := 3
//         b := 4
//     }
// }
