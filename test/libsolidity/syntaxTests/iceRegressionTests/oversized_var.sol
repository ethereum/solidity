contract b {
    struct c {
        uint [2 ** 253] a;
    }

    c d;
    function e() public view {
        c storage x = d;
	x.a[0];
    }
}
// ----
// Warning 3408: (66-69): Variable "d" covers a large part of storage and thus makes collisions likely. Either use mappings or dynamic arrays and allow their size to be increased only in small quantities per transaction.
// Warning 2332: (110-111): Type "b.c" covers a large part of storage and thus makes collisions likely. Either use mappings or dynamic arrays and allow their size to be increased only in small quantities per transaction.
