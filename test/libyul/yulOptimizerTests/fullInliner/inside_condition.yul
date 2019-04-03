// This tests that splitting the expression inside the condition works properly.
{
	if gt(f(mload(1)), mload(0)) {
		sstore(0, 2)
	}
	function f(a) -> r {
		a := mload(a)
		r := add(a, calldatasize())
	}
}
// ====
// step: fullInliner
// ----
// {
//     {
//         let _2 := mload(0)
//         let a_10 := mload(1)
//         let r_11 := 0
//         a_10 := mload(a_10)
//         r_11 := add(a_10, calldatasize())
//         if gt(r_11, _2)
//         {
//             sstore(0, 2)
//         }
//     }
//     function f(a) -> r
//     {
//         a := mload(a)
//         r := add(a, calldatasize())
//     }
// }
