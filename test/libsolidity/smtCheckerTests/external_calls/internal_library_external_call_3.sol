interface I {
    function i() external;
}

library M {
    function f(I _i) internal {
        _i.i();
    }
}

library L {
	function g(I _i) internal {
		M.f(_i);
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
// Warning 6328: (354-368): CHC: Assertion violation happens here.
