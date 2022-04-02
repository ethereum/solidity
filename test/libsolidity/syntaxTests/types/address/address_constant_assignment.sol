contract C {
    address constant a = address(0);
    address payable constant b = payable(0);
    function f() public {
        a = address(0);
        b = payable(0);
    }
}
// ----
// TypeError 6520: (129-130='a'): Cannot assign to a constant variable.
// TypeError 6520: (153-154='b'): Cannot assign to a constant variable.
