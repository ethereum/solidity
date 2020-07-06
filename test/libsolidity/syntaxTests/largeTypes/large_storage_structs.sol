contract C {
    struct P { uint256[2**63] x; }

    struct S0 {
        P[101] x;
        P y;
    }
    S0 s0;

    struct S1 {
        P x;
        P[102] y;
    }
    S1 s1;

    struct S2 {
        mapping(uint => P[103]) x;
        mapping(uint => P[103]) y;
        mapping(uint => P[104]) z;
        mapping(uint => S2) t;
    }
    S2 s2;

    struct Q0
    {
        uint[1][][10**20 + 1] x;
        uint[10**20 + 2][][1] y;
        uint[][10**20 + 3] z;
        uint[10**20 + 4][] t;
    }
    Q0 q0;

    struct Q1
    {
        uint[1][][10**20 + 5] x;
    }
    Q1 q1;

    struct Q2
    {
        uint[10**20 + 6][][1] y;
    }
    Q2 q2;

    struct Q3
    {
        uint[][10**20 + 7] x;
    }
    Q3 q3;

    struct Q4
    {
        uint[10**20 + 8][] y;
    }
    Q4 q4;
}
// ----
// Warning 3408: (106-111): Variable "s0" covers a large part of storage and thus makes collisions likely. Either use mappings or dynamic arrays and allow their size to be increased only in small quantities per transaction.
// Warning 3408: (171-176): Variable "s1" covers a large part of storage and thus makes collisions likely. Either use mappings or dynamic arrays and allow their size to be increased only in small quantities per transaction.
// Warning 7325: (341-343): Type "C.P[103]" covers a large part of storage and thus makes collisions likely. Either use mappings or dynamic arrays and allow their size to be increased only in small quantities per transaction.
// Warning 7325: (341-343): Type "C.P[104]" covers a large part of storage and thus makes collisions likely. Either use mappings or dynamic arrays and allow their size to be increased only in small quantities per transaction.
// Warning 3408: (505-510): Variable "q0" covers a large part of storage and thus makes collisions likely. Either use mappings or dynamic arrays and allow their size to be increased only in small quantities per transaction.
// Warning 7325: (505-507): Type "uint256[100000000000000000002]" covers a large part of storage and thus makes collisions likely. Either use mappings or dynamic arrays and allow their size to be increased only in small quantities per transaction.
// Warning 7325: (505-507): Type "uint256[100000000000000000004]" covers a large part of storage and thus makes collisions likely. Either use mappings or dynamic arrays and allow their size to be increased only in small quantities per transaction.
// Warning 3408: (576-581): Variable "q1" covers a large part of storage and thus makes collisions likely. Either use mappings or dynamic arrays and allow their size to be increased only in small quantities per transaction.
// Warning 7325: (647-649): Type "uint256[100000000000000000006]" covers a large part of storage and thus makes collisions likely. Either use mappings or dynamic arrays and allow their size to be increased only in small quantities per transaction.
// Warning 3408: (715-720): Variable "q3" covers a large part of storage and thus makes collisions likely. Either use mappings or dynamic arrays and allow their size to be increased only in small quantities per transaction.
// Warning 7325: (783-785): Type "uint256[100000000000000000008]" covers a large part of storage and thus makes collisions likely. Either use mappings or dynamic arrays and allow their size to be increased only in small quantities per transaction.
