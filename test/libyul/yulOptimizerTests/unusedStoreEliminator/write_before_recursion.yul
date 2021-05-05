{
    sstore(0, 1)
    mstore(0, 2)
    f()
    function f() {
        g()
    }
    function g() {
        f()
    }
}
// ----
// step: unusedStoreEliminator
//
// {
//     {
//         let _1 := 1
//         let _2 := 0
//         let _3 := 2
//         let _4 := 0
//         f()
//     }
//     function f()
//     { g() }
//     function g()
//     { f() }
// }
