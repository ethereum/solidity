{
    let a
    let b
    function f() -> x, y { }
    a, b := f()
}
// ----
// unusedPruner
// {
//     let a
//     let b
//     function f() -> x, y
//     {
//     }
//     a, b := f()
// }
