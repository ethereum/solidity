{
	function f(a) {
		sstore(a, a)
	}
	f(mload(0))
}
// ----
// step: fullInliner
//
// {
//     {
//         let a_1 := mload(0)
//         sstore(a_1, a_1)
//     }
//     function f(a)
//     { sstore(a, a) }
// }
