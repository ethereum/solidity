{
	function f(a) {
		sstore(a, a)
	}
	f(mload(0))
}
// ----
// fullInliner
// {
//     {
//         let a_3 := mload(0)
//         sstore(a_3, a_3)
//     }
//     function f(a)
//     {
//         sstore(a, a)
//     }
// }
