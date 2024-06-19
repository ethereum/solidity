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
//         let a_1 := mload(1)
//         let r_1 := 0
//         a_1 := mload(a_1)
//         r_1 := add(a_1, calldatasize())
//         if gt(r_1, _1) { sstore(0, 2) }
//     }
//     function f(a) -> r
//     {
//         a := mload(a)
//         r := add(a, calldatasize())
//     }
// }
