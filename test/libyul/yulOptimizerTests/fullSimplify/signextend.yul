{
  let x := 7
  mstore(0, signextend(50, x))
  let y := 255
  mstore(1, signextend(0, y))
}
// ----
// fullSimplify
// {
//     mstore(0, 7)
//     mstore(1, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff)
// }
