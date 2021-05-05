{
    function f() {
        let x := calldataload(2)
        sstore(x, 2)
        // This cannot be removed because we do not know what happens after the function.
        sstore(x, 3)
    }
}
// ----
// step: unusedStoreEliminator
//
// {
//     { }
//     function f()
//     {
//         let x := calldataload(2)
//         let _2 := 2
//         sstore(x, 3)
//     }
// }
