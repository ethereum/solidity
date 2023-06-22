{
	function f(a) {
		sstore(a, a)
	}
	f(mload(0))
}
// ====
// EVMVersion: >=shanghai
// ----
// step: fullInliner
//
// {
//     {
//         let a_2 := mload(0)
//         sstore(a_2, a_2)
//     }
//     function f(a)
//     { sstore(a, a) }
// }
