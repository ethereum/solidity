{
    function direct(arg, value)
    {
        sstore(arg, value)
        let v1 := add(value, 1)
        sstore(arg, v1)
    }

    function indirect(arg, value)
    {
        let loc := arg
        sstore(loc, value)
        let value1 := add(value, 1)
        sstore(loc, value1)
    }
}
// ----
// step: unusedStoreEliminatorNoSsaTransform
//
// {
//     function direct(arg, value)
//     {
//         let v1 := add(value, 1)
//         sstore(arg, v1)
//     }
//     function indirect(arg_1, value_2)
//     {
//         let loc := arg_1
//         let value1 := add(value_2, 1)
//         sstore(loc, value1)
//     }
// }
