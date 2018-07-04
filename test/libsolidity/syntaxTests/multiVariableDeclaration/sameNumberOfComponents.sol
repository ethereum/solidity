contract C {
    function f() public pure {
        (uint a1, uint b1, uint c1, uint d1) = (1,2,3,4);
        (uint a2, uint b2, uint c2) = (1,2,3);
        (uint a3, uint b3) = (1,2);
        a1; b1; c1; d1; a2; b2; c2; a3; b3;
    }
}
// ----
