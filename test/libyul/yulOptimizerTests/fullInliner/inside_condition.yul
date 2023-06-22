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
// ----
// step: fullInliner
//
// {
//     {
//         let _1 := mload(0)
//         let a_8 := mload(1)
//         let r_9 := 0
//         a_8 := mload(a_8)
//         r_9 := add(a_8, calldatasize())
//         if gt(r_9, _1) { sstore(0, 2) }
//     }
//     function f(a) -> r
//     {
//         a := mload(a)
//         r := add(a, calldatasize())
//     }
// }
