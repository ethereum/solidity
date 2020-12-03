contract test {
    function f() public {
        payable(0x12).send(1);
    }
}
// ----
// Warning 5878: (50-71): Failure condition of 'send' ignored. Consider using 'transfer' instead.
