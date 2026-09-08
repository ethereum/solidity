{
	mstore(0x40, memoryguard(128))
	// Self-recursive function `r`. Both `p` and `h` spill and must not share a slot.
	//
	//      +---+
	//      v    |
	// p -> r --+
	//      |
	//      v
	//      h
	r()
	p()
	function r() {
		r()
		h()
	}
	function h() {
		let $h := 1
		sstore(0, $h)
	}
	function p() {
		let $p := 2
		r()
		sstore(1, $p)
	}
}
// ----
// step: fakeStackLimitEvader
//
// {
//     mstore(0x40, memoryguard(0xc0))
//     r()
//     p()
//     function r()
//     {
//         r()
//         h()
//     }
//     function h()
//     {
//         mstore(0xa0, 1)
//         sstore(0, mload(0xa0))
//     }
//     function p()
//     {
//         mstore(0x80, 2)
//         r()
//         sstore(1, mload(0x80))
//     }
// }
