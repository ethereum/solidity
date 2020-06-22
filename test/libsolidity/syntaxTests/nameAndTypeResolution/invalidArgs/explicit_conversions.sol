contract test {
    function f() public {
        uint(1, 1);
        uint({arg:1});
    }
}
// ----
// TypeError 2558: (50-60): Exactly one argument expected for explicit type conversion.
// TypeError 5153: (70-83): Type conversion cannot allow named arguments.
