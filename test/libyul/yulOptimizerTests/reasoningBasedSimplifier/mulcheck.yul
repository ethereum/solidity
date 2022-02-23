{
    let vloc_x := calldataload(0)
    let vloc_y := calldataload(1)
    if lt(vloc_x, shl(100, 1)) {
        if lt(vloc_y, shl(100, 1)) {
            if iszero(and(iszero(iszero(vloc_x)), gt(vloc_y, div(not(0), vloc_x)))) {
                let vloc := mul(vloc_x, vloc_y)
                sstore(0, vloc)
            }
        }
    }
}
// ====
// EVMVersion: >=constantinople
// ----
// step: reasoningBasedSimplifier
//
// {
//     let vloc_x := calldataload(0)
//     let vloc_y := calldataload(1)
//     if lt(vloc_x, shl(100, 1))
//     {
//         if lt(vloc_y, shl(100, 1))
//         {
//             if 1
//             {
//                 let vloc := mul(vloc_x, vloc_y)
//                 sstore(0, vloc)
//             }
//         }
//     }
// }
