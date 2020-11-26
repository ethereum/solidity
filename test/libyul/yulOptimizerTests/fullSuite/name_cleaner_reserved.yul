{
	// This function name can be shortened, the other cannot.
	function nonmstore_(x) {
		nonmstore_(x)
		sstore(10, calldataload(2))
	}
	function mstore_(x) -> y {
		let t3_3_ := mstore_(x)
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
//         pop(mstore_1011())
//         let y := 8
//         let _1 := calldataload(y)
//         sstore(y, _1)
//         sstore(y, _1)
//         sstore(y, _1)
//         sstore(y, _1)
//         sstore(y, _1)
//         sstore(y, _1)
//         sstore(y, _1)
//         sstore(y, _1)
//         sstore(y, _1)
//         sstore(y, _1)
//         sstore(y, _1)
//         nonmstore_1012()
//         let _2 := calldataload(2)
//         let _3 := 10
//         sstore(_3, _2)
//         sstore(_3, _2)
//         sstore(_3, _2)
//         sstore(_3, _2)
//         sstore(_3, _2)
//         sstore(_3, _2)
//         sstore(_3, _2)
//         sstore(_3, _2)
//         sstore(_3, _2)
//         sstore(_3, _2)
//         sstore(_3, _2)
//     }
//     function nonmstore_1012()
//     {
//         nonmstore(70)
//         sstore(10, calldataload(2))
//     }
//     function nonmstore(x)
//     {
//         nonmstore(x)
//         sstore(10, calldataload(2))
//     }
//     function mstore_1011() -> y
//     {
//         pop(mstore_(7))
//         y := 8
//         sstore(y, calldataload(y))
//     }
//     function mstore_(x) -> y
//     {
//         pop(mstore_(x))
//         y := 8
//         sstore(y, calldataload(y))
//     }
// }
