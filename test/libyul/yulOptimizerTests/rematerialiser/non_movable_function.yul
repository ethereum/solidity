{
    function f(x) -> y { log0(0, 0x20) }
    let a := 1
    let b := f(a)
    let c := a
    mstore(add(a, b), c)
}
// ----
// step: rematerialiser
//
// {
//     let a := 1
//     let b := f(1)
//     let c := 1
//     mstore(add(1, b), 1)
//     function f(x) -> y
//     { log0(0, 0x20) }
// }
