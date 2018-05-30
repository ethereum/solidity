contract test {
    function f() public {
        uint[3.5] a; a;
    }
}
// ----
// TypeError: (55-58): Array with fractional length specified.
