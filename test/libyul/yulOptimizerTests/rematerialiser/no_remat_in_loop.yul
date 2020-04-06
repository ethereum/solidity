{
	// origin has zero cost and thus will be rematerialised,
	// calldataload(0) has low cost and will not be rematerialised
	let a := origin()
	let b := calldataload(0)
	let i := 0
	let z := calldataload(9)
	for {} lt(i, 10) {i := add(a, b)} {
		// This will be rematerialised, because it stays inside
		// the loop.
		let x := calldataload(1)
		mstore(9, x)
		// No, because again one loop further down.
		let y := calldataload(2)
		for {} y {} {
			// Again no.
			mstore(12, z)
		}
	}
}
// ----
// step: rematerialiser
//
// {
//     let a := origin()
//     let b := calldataload(0)
//     let i := 0
//     let z := calldataload(9)
//     for { } lt(i, 10) { i := add(origin(), b) }
//     {
//         let x := calldataload(1)
//         mstore(9, calldataload(1))
//         let y := calldataload(2)
//         for { } y { }
//         { mstore(12, z) }
//     }
// }
