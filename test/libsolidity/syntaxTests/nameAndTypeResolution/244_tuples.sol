contract C {
    function f() public pure {
        uint a = (1);
        (uint b,) = (uint8(1),2);
        (uint c, uint d) = (uint32(1), 2 + a);
        (uint e, ,) = (uint64(1), 2, b);
        a;b;c;d;e;
    }
}
// ----
