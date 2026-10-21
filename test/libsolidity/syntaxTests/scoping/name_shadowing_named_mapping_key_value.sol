contract C {
    mapping (uint x => uint y) m;
    uint k;
    mapping (uint k => uint v) n;
    uint v;

    function f() public view returns (uint) {
        uint a;
        uint b;
        mapping(uint a => uint b) storage mPtr1 = m;

        mapping(uint x => uint y) storage mPtr2 = n;
        uint x;
        uint y;

        return a + b + x + y + mPtr1[0] + mPtr2[0];
    }
}
