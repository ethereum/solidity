contract C {
    modifier m {
        uint256 a = 1;
        assembly {
            a := 2
        }
        if (a != 2) revert();
        _;
    }

    function f() public m returns (bool) {
        return true;
    }
}

// ====
// compileViaYul: also
// ----
// f() -> true
