{
	mstore(0x40, memoryguard(128))
	// Two cycles in a chain: p -> f <=> g -> u <=> w -> h
	// A local of p and h, respectively, is spilled and can't end up in same slots.
	f()
	p()
	function f() {
		g()
	}
	function g() {
		f()
		u()
	}
	function u() {
		w()
	}
	function w() {
		u()
		h()
	}
	function h() {
		let $h := 1
		sstore(0, $h)
	}
	function p() {
		let $p := 2
		f()
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
//     { g() }
//     function g()
//     {
//         f()
//         u()
//     }
//     function u()
//     { w() }
//     function w()
//     {
//         u()
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
//         f()
//         sstore(1, mload(0x80))
//     }
// }
