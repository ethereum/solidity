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
// Warning 7325: (106-108): Type struct C.S0 covers a large part of storage and thus makes collisions likely. Either use mappings or dynamic arrays and allow their size to be increased only in small quantities per transaction.
// Warning 7325: (106-108): Type struct C.P[101] covers a large part of storage and thus makes collisions likely. Either use mappings or dynamic arrays and allow their size to be increased only in small quantities per transaction.
// Warning 7325: (171-173): Type struct C.S1 covers a large part of storage and thus makes collisions likely. Either use mappings or dynamic arrays and allow their size to be increased only in small quantities per transaction.
// Warning 7325: (171-173): Type struct C.P[102] covers a large part of storage and thus makes collisions likely. Either use mappings or dynamic arrays and allow their size to be increased only in small quantities per transaction.
// Warning 7325: (341-343): Type struct C.P[103] covers a large part of storage and thus makes collisions likely. Either use mappings or dynamic arrays and allow their size to be increased only in small quantities per transaction.
// Warning 7325: (341-343): Type struct C.P[104] covers a large part of storage and thus makes collisions likely. Either use mappings or dynamic arrays and allow their size to be increased only in small quantities per transaction.
// Warning 7325: (505-507): Type struct C.Q0 covers a large part of storage and thus makes collisions likely. Either use mappings or dynamic arrays and allow their size to be increased only in small quantities per transaction.
// Warning 7325: (505-507): Type uint256[1][][100000000000000000001] covers a large part of storage and thus makes collisions likely. Either use mappings or dynamic arrays and allow their size to be increased only in small quantities per transaction.
// Warning 7325: (505-507): Type uint256[][100000000000000000003] covers a large part of storage and thus makes collisions likely. Either use mappings or dynamic arrays and allow their size to be increased only in small quantities per transaction.
// Warning 7325: (505-507): Type uint256[100000000000000000004] covers a large part of storage and thus makes collisions likely. Either use mappings or dynamic arrays and allow their size to be increased only in small quantities per transaction.
// Warning 7325: (505-507): Type uint256[100000000000000000002] covers a large part of storage and thus makes collisions likely. Either use mappings or dynamic arrays and allow their size to be increased only in small quantities per transaction.
// Warning 7325: (576-578): Type struct C.Q1 covers a large part of storage and thus makes collisions likely. Either use mappings or dynamic arrays and allow their size to be increased only in small quantities per transaction.
// Warning 7325: (576-578): Type uint256[1][][100000000000000000005] covers a large part of storage and thus makes collisions likely. Either use mappings or dynamic arrays and allow their size to be increased only in small quantities per transaction.
// Warning 7325: (647-649): Type uint256[100000000000000000006] covers a large part of storage and thus makes collisions likely. Either use mappings or dynamic arrays and allow their size to be increased only in small quantities per transaction.
// Warning 7325: (715-717): Type struct C.Q3 covers a large part of storage and thus makes collisions likely. Either use mappings or dynamic arrays and allow their size to be increased only in small quantities per transaction.
// Warning 7325: (715-717): Type uint256[][100000000000000000007] covers a large part of storage and thus makes collisions likely. Either use mappings or dynamic arrays and allow their size to be increased only in small quantities per transaction.
// Warning 7325: (783-785): Type uint256[100000000000000000008] covers a large part of storage and thus makes collisions likely. Either use mappings or dynamic arrays and allow their size to be increased only in small quantities per transaction.
