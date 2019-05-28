{
    function f() -> x, y { }
    let a, b := f()
}
// ====
// step: unusedPruner
// ----
// {
//     function f() -> x, y
//     { }
//     let a, b := f()
// }
