contract C {
    function f() public {
        var x = 0xfA0bFc97E48458494Ccd857e1A85DC91F7F0046E;
        x.send(2);
    }
}
// ----
// Warning: (47-52): Use of the "var" keyword is deprecated.
// Warning: (107-116): Failure condition of 'send' ignored. Consider using 'transfer' instead.
