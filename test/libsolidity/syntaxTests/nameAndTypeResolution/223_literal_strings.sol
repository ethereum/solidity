contract Foo {
    function f() public {
        string memory long = "01234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890";
        string memory short = "123";
        long; short;
    }
}
// ----
// Warning: (19-238): Function state mutability can be restricted to pure
