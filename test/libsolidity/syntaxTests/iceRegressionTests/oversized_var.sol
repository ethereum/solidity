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
// Warning 3408: (66-69): Variable "d" covers a large part of storage and thus makes collisions likely. Either use mappings or dynamic arrays and allow their size to be increased only in small quantities per transaction.
// Warning 2332: (105-110): Type "b.c" covers a large part of storage and thus makes collisions likely. Either use mappings or dynamic arrays and allow their size to be increased only in small quantities per transaction.
// SyntaxError 1719: (105-114): Use of the "var" keyword is disallowed. Use explicit declaration `struct b.c storage pointer d = ...´ instead.
