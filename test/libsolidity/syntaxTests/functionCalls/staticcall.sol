contract C {
    function f() public {
        (bool success, ) = address(10).staticcall{gas: 3}("");
        success;
    }
}
// ----
// Warning 2018: (17-124): Function state mutability can be restricted to view
