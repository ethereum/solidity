{
    function direct(arg, value)
    {
        sstore(arg, value)
        // Reassignment does not change the value (self-assignment), but it still
        // makes USE lose track of arg, so the two stores below are not recognized
        // as targeting the same slot even though they provably do.
        arg := arg
        let value1 := add(value, 1)
        sstore(arg, value1)
    }

    function indirect(arg, value)
    {
        let loc := arg
        sstore(loc, value)
        // Reassigning arg (not loc) still invalidates loc, because loc's tracked
        // value transitively depends on arg.
        arg := add(arg, 1)
        let value1 := add(value, 1)
        sstore(loc, value1)
    }

    function mix(arg, value)
    {
        let loc := arg
        // The first sstore could theoretically be eliminated since loc == arg,
        // but USE cannot prove this without a known constant value for arg;
        // a subsequent optimizer pass (after, e.g., CSE resolves the alias) would eliminate it.
        sstore(loc, value)
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
//         arg := arg
//         let value1 := add(value, 1)
//         sstore(arg, value1)
//     }
//     function indirect(arg_1, value_2)
//     {
//         let loc := arg_1
//         sstore(loc, value_2)
//         arg_1 := add(arg_1, 1)
//         let value1_3 := add(value_2, 1)
//         sstore(loc, value1_3)
//     }
//     function mix(arg_4, value_5)
//     {
//         let loc_6 := arg_4
//         sstore(loc_6, value_5)
//         let value1_7 := add(value_5, 1)
//         sstore(arg_4, value1_7)
//     }
// }
