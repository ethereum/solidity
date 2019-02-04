{
  let x := 8
  let y := calldataload(calldataload(9))
  mstore(y, add(add(add(add(add(add(add(add(add(add(add(add(add(add(add(add(add(add(y, 1), 1), 1), 1), 1), 1), 1), 1), 1), 1), 1), 1), 1), 1), 1), 1), 1), 1))
}
// ----
// stackCompressor
// {
//     mstore(calldataload(calldataload(9)), add(add(add(add(add(add(add(add(add(add(add(add(add(add(add(add(add(add(calldataload(calldataload(9)), 1), 1), 1), 1), 1), 1), 1), 1), 1), 1), 1), 1), 1), 1), 1), 1), 1), 1))
// }
