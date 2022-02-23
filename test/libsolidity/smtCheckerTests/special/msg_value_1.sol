contract C {
	function f() public payable {
		assert ((5 + msg.value + msg.value) - (4 + msg.value) > 0);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 4984: (55-68): CHC: Overflow (resulting value larger than 2**256 - 1) happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f(){ msg.value: 115792089237316195423570985008687907853269984665640564039457584007913129639931 }
// Warning 4984: (55-80): CHC: Overflow (resulting value larger than 2**256 - 1) happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f(){ msg.value: 57896044618658097711785492504343953926634992332820282019728792003956564819966 }
