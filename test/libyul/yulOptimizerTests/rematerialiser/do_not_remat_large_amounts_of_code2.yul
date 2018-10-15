{
	let x := add(mul(calldataload(2), calldataload(4)), calldatasize())
	let b := x
}
// ----
// rematerialiser
// {
//     let x := add(mul(calldataload(2), calldataload(4)), calldatasize())
//     let b := add(mul(calldataload(2), calldataload(4)), calldatasize())
// }
