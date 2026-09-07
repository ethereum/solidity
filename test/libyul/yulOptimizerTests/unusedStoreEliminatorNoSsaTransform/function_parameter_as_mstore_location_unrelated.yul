{
    function f(arg)
    {
        let a := add(arg, 32)
        let b := arg
        let outLen := 32
        mstore(a, 0xAA)
        return(b, outLen)
    }
}

// ----
// step: unusedStoreEliminatorNoSsaTransform
//
// {
//     function f(arg)
//     {
//         let a := add(arg, 32)
//         let b := arg
//         let outLen := 32
//         return(b, outLen)
//     }
// }
