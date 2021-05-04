{
    f()
    function f() {
        let x := 10
        let y := 20
        // will never be read, so can be removed
        mstore(x, y)
    }
}
// ----
// step: memoryStoreRemover
//
// {
//     f()
//     function f()
//     {
//         let x := 10
//         let y := 20
//         pop(y)
//     }
// }
