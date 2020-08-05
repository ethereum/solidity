contract C {
    struct S { uint256[2**255] x; }
    function f(S storage) internal {}
}
// ----
// Warning 2332: (64-65): Type "C.S" covers a large part of storage and thus makes collisions likely. Either use mappings or dynamic arrays and allow their size to be increased only in small quantities per transaction.
