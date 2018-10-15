{
    function f() -> x, y { }
    let a, b := f()
}
// ----
// unusedPruner
// {
//     function f() -> x, y
//     {
//     }
//     let a, b := f()
// }
