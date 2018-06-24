contract C {
    function f() public {
        uint[] storage x;
        uint[] memory y;
        uint[] memory z;
        x;y;z;
    }
}
// ----
// Warning: (47-63): Uninitialized storage pointer.
// Warning: (17-135): Function state mutability can be restricted to pure
