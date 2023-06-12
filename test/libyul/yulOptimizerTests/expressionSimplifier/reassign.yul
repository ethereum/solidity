{
	let x := mload(0)
	x := 0
	mstore(0, add(7, x))
}
// ====
// EVMVersion: >=shanghai
// ----
// step: expressionSimplifier
//
// {
//     {
//         let x := mload(0)
//         x := 0
//         mstore(0, 7)
//     }
// }
