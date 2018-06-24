contract test {
    function f(uint256 arg1, uint32 arg2) public returns (bool ret) {
        var x = arg1 + arg2 == 8; ret = x;
    }
}
// ----
// Warning: (94-99): Use of the "var" keyword is deprecated.
// Warning: (20-134): Function state mutability can be restricted to pure
