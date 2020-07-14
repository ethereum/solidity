contract b {
    struct c {
        uint [2 ** 253] a;
    }

    c d;
    function e() public {
        var d = d;
    }
}
// ----
// Warning 2519: (105-110): This declaration shadows an existing declaration.
// SyntaxError 1719: (105-114): Use of the "var" keyword is disallowed. Use explicit declaration `struct b.c storage pointer d = ...´ instead.
