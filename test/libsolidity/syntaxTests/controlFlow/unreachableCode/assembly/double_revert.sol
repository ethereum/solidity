contract C {
    function f() public pure {
        assembly {
            revert(0, 0)
            revert(0, 0)
        }
    }
    function g() public pure {
        assembly {
            revert(0, 0)
        }
        revert();
    }
}
// ----
// Warning 5740: (100-112): Unreachable code.
// Warning 5740: (222-230): Unreachable code.
