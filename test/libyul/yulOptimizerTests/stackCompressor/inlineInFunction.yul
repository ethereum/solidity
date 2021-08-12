{
  let x := 8
  function f() {
    let y := calldataload(calldataload(9))
    mstore(y, add(add(add(add(add(add(add(add(add(add(add(add(add(add(add(add(add(add(y, 1), 1), 1), 1), 1), 1), 1), 1), 1), 1), 1), 1), 1), 1), 1), 1), 1), 1))
  }
}
// ====
// EVMVersion: =homestead
// ----
// step: stackCompressor
//
// {
//     let x := 8
//     function f()
//     {
//         mstore(calldataload(calldataload(9)), add(add(add(add(add(add(add(add(add(add(add(add(add(add(add(add(add(add(calldataload(calldataload(9)), 1), 1), 1), 1), 1), 1), 1), 1), 1), 1), 1), 1), 1), 1), 1), 1), 1), 1))
//     }
// }
