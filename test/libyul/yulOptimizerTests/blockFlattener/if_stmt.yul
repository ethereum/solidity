{
  if add(mload(7), sload(mload(3)))
  {
    let y := add(mload(3), 3)
	{
      y := add(y, 7)
	}
  }
  let t := add(3, 9)
}
// ====
// step: blockFlattener
// ----
// {
//     if add(mload(7), sload(mload(3)))
//     {
//         let y := add(mload(3), 3)
//         y := add(y, 7)
//     }
//     let t := add(3, 9)
// }
