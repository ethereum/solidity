{
	mstore(0x40, memoryguard(128))
	// ``f`` and ``g`` are mutually recursive and need no memory themselves, but ``f`` reaches ``h``,
	// which does. ``p`` needs memory too and calls into the cycle, so it must not be given the same
	// offset as ``h``: ``p`` holds ``$p`` across the call to ``g``, which reaches ``h``.
	// Visiting ``f`` before ``p`` is what makes the requirements of the cycle observable to ``p``.
	f()
	p()
	function f() {
		h()
		g()
	}
	function g() {
		f()
	}
	function h() {
		let $h := 1
		sstore(0, $h)
	}
	function p() {
		let $p := 2
		g()
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
//     { f() }
//     function h()
//     {
//         mstore(0xa0, 1)
//         sstore(0, mload(0xa0))
//     }
//     function p()
//     {
//         mstore(0x80, 2)
//         g()
//         sstore(1, mload(0x80))
//     }
// }
