{
	// This function name can be shortened, the other cannot.
	function nonmstore_(x) {
		if calldataload(0) { nonmstore_(x) }
		sstore(10, calldataload(2))
	}
	function mstore_(x) -> y {
		if calldataload(0) { let t3_3_ := mstore_(x) }
		y := 8
		sstore(y, calldataload(y))
	}
	let t2_ := mstore_(7)
	nonmstore_(70)
}
// ----
// step: fullSuite
//
// {
//     {
//         pop(mstore_(7))
//         nonmstore(70)
//     }
//     function nonmstore(x)
//     {
//         if calldataload(0) { nonmstore(x) }
//         sstore(10, calldataload(2))
//     }
//     function mstore_(x) -> y
//     {
//         if calldataload(0) { pop(mstore_(x)) }
//         y := 8
//         sstore(y, calldataload(y))
//     }
// }
