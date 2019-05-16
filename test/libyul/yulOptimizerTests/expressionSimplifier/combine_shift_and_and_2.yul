{
    let x := calldataload(0)
    let a := and(0xff, shr(248, shl(248, shr(248, and(x, 0xf)))))
    let b := shl(12, shr(4, and(x, 0xf0f0)))
    let c := shl(12, shr(4, and(0xf0f0, x)))
    let d := shl(12, shr(255, and(0xf0f0, x)))
    let e := shl(255, shr(4, and(0xf0f0, x)))
    let f := shl(12, shr(256, and(0xf0f0, x)))
    let g := shl(256, shr(4, and(0xf0f0, x)))
}
// ====
// EVMVersion: >byzantium
// step: expressionSimplifier
// ----
// {
//     let x := calldataload(0)
//     let a := 0
//     let b := and(shl(8, x), 15790080)
//     let c := and(shl(8, x), 15790080)
//     let d := 0
//     let e := and(shl(251, x), 0x8000000000000000000000000000000000000000000000000000000000000000)
//     let f := 0
//     let g := 0
// }
