{
	mstore(0x40, memoryguard(128))
	// Generalization of `mutual_recursion_with_spilling_caller` to a three-member cycle:
	//
	//      +---------+
	//      |         |
	//      v         |
	// p -> k -> f -> g
	//           |
	//           v
	//           h
	f()
	p()
	function f() {
		h()
		g()
	}
	function g() {
		k()
	}
	function k() {
		f()
	}
	function h() {
		let $h := 1
		sstore(0, $h)
	}
	function p() {
		let $p := 2
		k()
		sstore(1, $p)
	}
}
// ----
// step: fakeStackLimitEvader
//
// {
//     mstore(0x40, memoryguard(0xc0))
//     f()
//     p()
//     function f()
//     {
//         h()
//         g()
//     }
//     function g()
//     { k() }
//     function k()
//     { f() }
//     function h()
//     {
//         mstore(0xa0, 1)
//         sstore(0, mload(0xa0))
//     }
//     function p()
//     {
//         mstore(0x80, 2)
//         k()
//         sstore(1, mload(0x80))
//     }
// }
