{
    let var_x_mpos := mload(0x40)
    let var_r := 0
    let var_i := 0
    for { }
    lt(var_i, mload(var_x_mpos))
    {
        // "not(0)" does not work here - can we
        // use a different step to do the bulk of the work here?
        if eq(var_i, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff) { revert(0, 0) }
        var_i := add(var_i, 1)
    }
    {
        let _1_1 := mload(add(add(var_x_mpos, shl(5, var_i)), 32))
        if gt(var_r, not(_1_1)) { revert(0, 0) }
        var_r := add(var_r, _1_1)
    }
}
// ----
// step: reasoningBasedSimplifier
//
// {
//     let var_x_mpos := mload(0x40)
//     let var_r := 0
//     let var_i := 0
//     for { }
//     lt(var_i, mload(var_x_mpos))
//     {
//         if 0 { }
//         var_i := add(var_i, 1)
//     }
//     {
//         let _1_1 := mload(add(add(var_x_mpos, shl(5, var_i)), 32))
//         if gt(var_r, not(_1_1)) { revert(0, 0) }
//         var_r := add(var_r, _1_1)
//     }
// }
