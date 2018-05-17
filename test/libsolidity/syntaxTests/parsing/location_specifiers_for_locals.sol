contract Foo {
    function f() {
        uint[] storage x;
        uint[] memory y;
    }
}
// ----
// Warning: (42-58): Uninitialized storage pointer.
// Warning: (19-90): No visibility specified. Defaulting to "public". 
// Warning: (42-58): Unused local variable.
// Warning: (68-83): Unused local variable.
// Warning: (19-90): Function state mutability can be restricted to pure
