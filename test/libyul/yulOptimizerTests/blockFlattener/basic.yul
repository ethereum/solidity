{
  let _1 := mload(0)
  let f_a := mload(1)
  let f_r
  {
    f_a := mload(f_a)
    f_r := add(f_a, calldatasize())
  }
  let z := mload(2)
}
// ====
// step: blockFlattener
// ----
// {
//     let _1 := mload(0)
//     let f_a := mload(1)
//     let f_r
//     f_a := mload(f_a)
//     f_r := add(f_a, calldatasize())
//     let z := mload(2)
// }
