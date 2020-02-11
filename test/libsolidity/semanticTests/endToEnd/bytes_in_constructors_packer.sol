contract Base {
    uint public m_x;
    bytes m_s;
    constructor(uint x, bytes memory s) public {
        m_x = x;
        m_s = s;
    }

    function part(uint i) public returns(byte) {
        return m_s[i];
    }
}
contract Main is Base {
    constructor(bytes memory s, uint x) Base(x, f(s)) public {}

    function f(bytes memory s) public returns(bytes memory) {
        return s;
    }
}
contract Creator {
    function f(uint x, bytes memory s) public returns(uint r, byte ch) {
        Main c = new Main(s, x);
        r = c.m_x();
        ch = c.part(x);
    }
}

// ----
f(uint256, bytes): "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\x0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0@\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0Nabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
