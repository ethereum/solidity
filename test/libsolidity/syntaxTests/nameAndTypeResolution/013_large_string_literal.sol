contract test {
    function f() public { var x = "123456789012345678901234567890123"; }
}
// ----
// Warning: (42-47): Use of the "var" keyword is deprecated.
// Warning: (42-47): Unused local variable.
// Warning: (20-88): Function state mutability can be restricted to pure
