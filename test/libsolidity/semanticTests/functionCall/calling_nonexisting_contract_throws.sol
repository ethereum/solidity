abstract contract D {
    function g() public virtual;
}


contract C {
    D d = D(0x1212);

    function f() public returns (uint256) {
        d.g();
        return 7;
    }

    function g() public returns (uint256) {
        d.g{gas: 200}();
        return 7;
    }

    function h() public returns (uint256) {
        address(d).call(""); // this does not throw (low-level)
        return 7;
    }
}

// ----
// f() -> FAILURE
// g() -> FAILURE
// h() -> 7
