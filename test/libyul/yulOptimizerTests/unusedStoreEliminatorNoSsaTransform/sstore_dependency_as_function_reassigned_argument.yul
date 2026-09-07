{
    function f(arg)
    {
        let x := arg
        x := add(arg, 32)
        sstore(x, 0xAA)
        sstore(arg, 0xBB)
    }
}
// ----
// step: unusedStoreEliminatorNoSsaTransform
//
// {
//     function f(arg)
//     {
//         let x := arg
//         x := add(arg, 32)
//         sstore(x, 0xAA)
//         sstore(arg, 0xBB)
//     }
// }
