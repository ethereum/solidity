{
    function direct(arg, value)
    {
        sstore(arg, value)
        pop(sload(arg))
        let value1 := add(value, 1)
        sstore(arg, value1)
    }

    function indirect(arg, value)
    {
        let loc := arg
        sstore(loc, value)
        pop(sload(arg))
        let value1 := add(value, 1)
        sstore(loc, value1)
    }

    function mixReadUnrelatedSlot(arg, value)
    {
        let loc := add(arg, 1)
        sstore(arg, value) // Eliminated: loc (below) does not alias arg
        pop(sload(loc))
        let value1 := add(value, 1)
        sstore(arg, value1)
    }

    function mixReadSameSlot(arg, value)
    {
        let loc := arg
        sstore(arg, value) // Not eliminated: loc (below) aliases arg
        pop(sload(loc))
        let value1 := add(value, 1)
        sstore(arg, value1)
    }
}
// ----
// step: unusedStoreEliminatorNoSsaTransform
//
// {
//     function direct(arg, value)
//     {
//         sstore(arg, value)
//         pop(sload(arg))
//         let value1 := add(value, 1)
//         sstore(arg, value1)
//     }
//     function indirect(arg_1, value_2)
//     {
//         let loc := arg_1
//         sstore(loc, value_2)
//         pop(sload(arg_1))
//         let value1_3 := add(value_2, 1)
//         sstore(loc, value1_3)
//     }
//     function mixReadUnrelatedSlot(arg_4, value_5)
//     {
//         let loc_6 := add(arg_4, 1)
//         pop(sload(loc_6))
//         let value1_7 := add(value_5, 1)
//         sstore(arg_4, value1_7)
//     }
//     function mixReadSameSlot(arg_8, value_9)
//     {
//         let loc_10 := arg_8
//         sstore(arg_8, value_9)
//         pop(sload(loc_10))
//         let value1_11 := add(value_9, 1)
//         sstore(arg_8, value1_11)
//     }
// }
