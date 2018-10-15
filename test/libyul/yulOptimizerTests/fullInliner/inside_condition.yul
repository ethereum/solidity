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
// fullInliner
// {
//     {
//         let _1 := mload(0)
//         let f_a := mload(1)
//         let f_r
//         {
//             f_a := mload(f_a)
//             f_r := add(f_a, calldatasize())
//         }
//         if gt(f_r, _1)
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
