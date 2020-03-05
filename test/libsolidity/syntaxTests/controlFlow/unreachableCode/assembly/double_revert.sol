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
// Warning: (100-112): Unreachable code.
// Warning: (222-230): Unreachable code.
