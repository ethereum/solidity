{
    let a, b
    function f() -> x { }
    a := f()
    b := 1
}
// ----
// unusedPruner
// {
//     let a, b
//     function f() -> x
//     {
//     }
//     a := f()
//     b := 1
// }
