contract C {
    function f() public pure {
        assembly {
            for { let a := 0} lt(a,1) { a := add(a, 1) } {
                break
                let b := 42
            }
        }
    }
}
// ----
// Warning 5740: (103-117): Unreachable code.
// Warning 5740: (160-171): Unreachable code.
