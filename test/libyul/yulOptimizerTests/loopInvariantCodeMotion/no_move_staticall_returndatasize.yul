{
  let b := 1
  // invalidates state in post
  for { let a := 1 } iszero(eq(a, 10)) {pop(call(2, 0x01, 2, 0x00, 32, 0x010, 32))} {
    let inv := add(b, 42)
    let x := returndatasize()
    a := add(x, 1)
    pop(staticcall(2, 3, 0, 32, 64, 32)) // prevents moving returndatasize
    mstore(a, inv)
  }
}
// ====
// EVMVersion: >=byzantium
// ----
// step: loopInvariantCodeMotion
//
// {
//     let b := 1
//     let a := 1
//     let inv := add(b, 42)
//     for { }
//     iszero(eq(a, 10))
//     {
//         pop(call(2, 0x01, 2, 0x00, 32, 0x010, 32))
//     }
//     {
//         let x := returndatasize()
//         a := add(x, 1)
//         pop(staticcall(2, 3, 0, 32, 64, 32))
//         mstore(a, inv)
//     }
// }
