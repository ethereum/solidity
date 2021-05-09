{
    // Tests that masks that "add" up to
    // the full bit width are removed.
    let a := sload(0)
    let x := sload(a)
    let mask := 0xffffffffffffffff
    x := or(and(x, not(mask)), 0x2b67)
    mask := shl(64, mask)
    x := or(and(x, not(mask)), 0x56ce0000000000000000)
    mask := shl(64, mask)
    x := or(and(x, not(mask)), shl(0x80, 0x8235))
    mask := shl(64, mask)
    x := or(and(x, not(mask)), shl(0xc2, 0x2b67))
    sstore(a, x)
}
// ====
// EVMVersion: >byzantium
// ----
// step: fullSuite
//
// {
//     {
//         sstore(sload(0), 0xad9c000000000000823500000000000056ce0000000000002b67)
//     }
// }
