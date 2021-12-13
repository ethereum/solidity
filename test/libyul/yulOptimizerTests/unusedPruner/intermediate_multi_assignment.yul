{
    let a, b
    function f() -> x { }
    a := f()
    b := 1
}
// ----
// step: unusedPruner
//
// {
//     {
//         let a, b
//         a := f()
//         b := 1
//     }
//     function f() -> x
//     { }
// }
