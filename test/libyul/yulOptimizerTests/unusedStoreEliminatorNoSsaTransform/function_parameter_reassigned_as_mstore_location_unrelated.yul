{
    function f(arg)
    {
        let b := arg
        arg := add(arg, 32)
        let outLen := 32
        mstore(arg, 0xAA)
        return(b, outLen)
    }
}

// ----
// step: unusedStoreEliminatorNoSsaTransform
//
// {
//     function f(arg)
//     {
//         let b := arg
//         arg := add(arg, 32)
//         let outLen := 32
//         mstore(arg, 0xAA)
//         return(b, outLen)
//     }
// }
