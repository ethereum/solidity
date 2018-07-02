contract Foo {
    function f() public {
        uint[] storage x;
        uint[] memory y;
    }
}
// ----
// Warning: (49-65): Uninitialized storage pointer.
// Warning: (49-65): Unused local variable.
// Warning: (75-90): Unused local variable.
// Warning: (19-97): Function state mutability can be restricted to pure
