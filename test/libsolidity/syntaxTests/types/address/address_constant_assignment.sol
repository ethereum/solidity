contract C {
    address constant a = address(0);
    address payable constant b = address(0);
    function f() public {
        a = address(0);
        b = address(0);
    }
}
// ----
// TypeError: (129-130): Cannot assign to a constant variable.
// TypeError: (153-154): Cannot assign to a constant variable.
