{
    function f() {
        let x := calldataload(2)
        mstore(x, 2)
        // This cannot be removed because we do not know what happens after the function.
        mstore(x, 3)
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
//         mstore(x, 3)
//     }
// }
