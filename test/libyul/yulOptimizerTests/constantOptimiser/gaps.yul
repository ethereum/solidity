{
  let a := 0x10000000000000000000000000000000000000000000
  let x := 0x11000000000000000000000000000000000000ffffffffffffffffffffffff23
  let y := 0xfffffffff00000000000000000000000000000000000000000000000000000ff
  for { let i := 0xff00000000000 } lt(i, 873687623427364) { i := add(i, 3234234234234) } {
  }
}
// ====
// EVMVersion: >=constantinople
// step: constantOptimiser
// ----
// {
//     let a := shl(172, 1)
//     let x := add(shl(248, 17), 0xffffffffffffffffffffffff23)
//     let y := add(shl(220, 0x0fffffffff), 255)
//     for { let i := 0xff00000000000 }
//     lt(i, 873687623427364)
//     { i := add(i, 3234234234234) }
//     { }
// }
