{
    function f(x) -> y {}
    let a := 1
    let b := f(a)
    let c := a
    mstore(add(a, b), c)
}
// ====
// step: rematerialiser
// ----
// {
//     function f(x) -> y
//     {
//     }
//     let a := 1
//     let b := f(1)
//     let c := 1
//     mstore(add(1, b), 1)
// }
