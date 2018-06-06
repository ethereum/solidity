contract c {
    uint32[] a;
    uint8[80] b;
    function f() public { a = b; }
}
