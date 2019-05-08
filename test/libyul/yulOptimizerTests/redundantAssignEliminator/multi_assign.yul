{
    function f() -> a, b {}
    let x, y
    x := 1
    x := 2
    // Will not be used, but is a multi-assign, so not removed.
    x, y := f()
    x := 3
    y := 4
}
// ====
// step: redundantAssignEliminator
// ----
// {
//     function f() -> a, b
//     { }
//     let x, y
//     x, y := f()
// }
