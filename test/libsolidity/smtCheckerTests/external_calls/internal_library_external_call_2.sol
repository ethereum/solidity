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
// Warning 6328: (337-351): CHC: Assertion violation happens here.
