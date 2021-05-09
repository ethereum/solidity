interface I {
    function i() external;
}

library L {
    function f(I _i) internal {
        _i.i();
    }
	function g(I _i) internal {
		f(_i);
	}
}

contract C {
    uint x;
    bool inG;
    function s() public {
        require(inG);
        x = 2;
    }
    function g(I _i) public {
        inG = true;
        L.g(_i);
        assert(x == 0);
        inG = false;
    }
}
// ----
// Warning 6328: (337-351): CHC: Assertion violation happens here.\nCounterexample:\nx = 2, inG = true\n_i = 0\n\nTransaction trace:\nC.constructor()\nState: x = 0, inG = false\nC.g(0)\n    L.g(0) -- internal call\n        L.f(0) -- internal call\n            _i.i() -- untrusted external call, synthesized as:\n                C.s() -- reentrant call
