contract C {
    struct P { uint256[2**63] x; }

    struct S0 {
        P[2**62] x;
        P y;
    }
    S0 s0;

    struct S1 {
        P x;
        P[2**62] y;
    }
    S1 s1;

    struct S2 {
        mapping(uint => P[2**62]) x;
        mapping(uint => P[2**62]) y;
        mapping(uint => S2) z;
    }
    S2 s2;

    struct Q0
    {
        uint[1][][2**65] x;
        uint[2**65][][1] y;
        uint[][2**65] z;
        uint[2**65][] t;
    }
    Q0 q0;

    struct Q1
    {
        uint[1][][2**65] x;
    }
    Q1 q1;

    struct Q2
    {
        uint[2**65][][1] y;
    }
    Q2 q2;

    struct Q3
    {
        uint[][2**65] z;
    }
    Q3 q3;

    struct Q4
    {
        uint[2**65][] t;
    }
    Q4 q4;
}
// ----
// Warning 3408: (108-113): Variable covers a large part of storage and thus makes collisions likely. Either use mappings or dynamic arrays and allow their size to be increased only in small quantities per transaction.
// Warning 3408: (175-180): Variable covers a large part of storage and thus makes collisions likely. Either use mappings or dynamic arrays and allow their size to be increased only in small quantities per transaction.
// Warning 7325: (314-319): Type C.P[4611686018427387904] has large size and thus makes collisions likely. Either use mappings or dynamic arrays and allow their size to be increased only in small quantities per transaction.
// Warning 3408: (458-463): Variable covers a large part of storage and thus makes collisions likely. Either use mappings or dynamic arrays and allow their size to be increased only in small quantities per transaction.
// Warning 7325: (458-463): Type uint256[36893488147419103232] has large size and thus makes collisions likely. Either use mappings or dynamic arrays and allow their size to be increased only in small quantities per transaction.
// Warning 3408: (524-529): Variable covers a large part of storage and thus makes collisions likely. Either use mappings or dynamic arrays and allow their size to be increased only in small quantities per transaction.
// Warning 7325: (590-595): Type uint256[36893488147419103232] has large size and thus makes collisions likely. Either use mappings or dynamic arrays and allow their size to be increased only in small quantities per transaction.
// Warning 3408: (653-658): Variable covers a large part of storage and thus makes collisions likely. Either use mappings or dynamic arrays and allow their size to be increased only in small quantities per transaction.
// Warning 7325: (716-721): Type uint256[36893488147419103232] has large size and thus makes collisions likely. Either use mappings or dynamic arrays and allow their size to be increased only in small quantities per transaction.
