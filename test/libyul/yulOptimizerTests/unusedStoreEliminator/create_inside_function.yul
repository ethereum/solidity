{
    let x := 5
    function f() {
        pop(create(0, 0, 0))
    }
    sstore(x, 10)
    f()
    sstore(x, 20)
}
// ----
// step: unusedStoreEliminator
//
// {
//     {
//         let x := 5
//         sstore(x, 10)
//         f()
//         sstore(x, 20)
//     }
//     function f()
//     { pop(create(0, 0, 0)) }
// }
