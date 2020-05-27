{
    let vloc_x := calldataload(0)
    let vloc_y := calldataload(1)
    if iszero(lt(vloc_x, shl(100, 1))) { revert(0, 0) }
    if iszero(lt(vloc_y, shl(100, 1))) { revert(0, 0) }
    if and(iszero(iszero(vloc_x)), gt(vloc_y, div(not(0), vloc_x))) { revert(0, 0) }
    let vloc := mul(vloc_x, vloc_y)
    sstore(0, vloc)
}
// ----
// step: reasoningBasedSimplifier
