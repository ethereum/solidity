{
    f()
    pop(mload(16))
    function f() {
        let x := 10
        let y := 20

        // cannot be removed
        mstore(x, y)
    }
}
// ----
// step: memoryStoreRemover
//
// {
//     f()
//     pop(mload(16))
//     function f()
//     {
//         let x := 10
//         let y := 20
//         mstore(x, y)
//     }
// }
