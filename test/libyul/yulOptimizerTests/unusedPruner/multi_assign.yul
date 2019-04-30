{
    let a
    let b
    function f() -> x, y { }
    a, b := f()
}
// ====
// step: unusedPruner
// ----
// {
//     let a
//     let b
//     function f() -> x, y
//     {
//     }
//     a, b := f()
// }
