{
    let vloc_x := calldataload(0)
    let vloc_y := calldataload(1)
    if lt(vloc_x, 0x1000000000000000) {
        if lt(vloc_y, 0x1000000000000000) {
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
//     if lt(vloc_x, 0x1000000000000000)
//     {
//         if lt(vloc_y, 0x1000000000000000)
//         {
//             if iszero(and(iszero(iszero(vloc_x)), gt(vloc_y, div(not(0), vloc_x))))
//             {
//                 sstore(0, mul(vloc_x, vloc_y))
//             }
//         }
//     }
// }
