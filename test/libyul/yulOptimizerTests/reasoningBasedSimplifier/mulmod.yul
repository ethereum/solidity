{
    let x := calldataload(0)
    let y := calldataload(32)
    let z := calldataload(64)
    let result := mulmod(x, y, z)

    // should be zero
    if gt(result, z) { sstore(0, 1) }

    // mulmod is equal to mod of product for small numbers
    if and(and(lt(x, 1000), lt(y, 1000)), lt(z, 1000)) {
        if eq(result, mod(mul(x, y), z)) { sstore(0, 9) }
    }

    // but not in general
    if and(and(gt(x, sub(0, 5)), eq(y, 2)), eq(z, 3)) {
        if eq(result, mod(mul(x, y), z)) { sstore(0, 5) }
    }
}
// ----
// step: reasoningBasedSimplifier
//
// {
//     let x := calldataload(0)
//     let y := calldataload(32)
//     let z := calldataload(64)
//     let result := mulmod(x, y, z)
//     if 0 { }
//     if and(and(lt(x, 1000), lt(y, 1000)), lt(z, 1000)) { if 1 { sstore(0, 9) } }
//     if and(and(gt(x, sub(0, 5)), eq(y, 2)), eq(z, 3)) { if 0 { } }
// }
