contract Test {
    struct S {
        uint8 x;
        uint16 y;
        uint256 z;
    }
    struct X {
        uint8 x;
        S s;
        uint8[2] a;
    }
    X m_x;

    function load()
        public
        returns (
            uint256 a,
            uint256 x,
            uint256 y,
            uint256 z,
            uint256 a1,
            uint256 a2
        )
    {
        m_x.x = 1;
        m_x.s.x = 2;
        m_x.s.y = 3;
        m_x.s.z = 4;
        m_x.a[0] = 5;
        m_x.a[1] = 6;
        X memory d = m_x;
        a = d.x;
        x = d.s.x;
        y = d.s.y;
        z = d.s.z;
        a1 = d.a[0];
        a2 = d.a[1];
    }

    function store()
        public
        returns (
            uint256 a,
            uint256 x,
            uint256 y,
            uint256 z,
            uint256 a1,
            uint256 a2
        )
    {
        X memory d;
        d.x = 1;
        d.s.x = 2;
        d.s.y = 3;
        d.s.z = 4;
        d.a[0] = 5;
        d.a[1] = 6;
        m_x = d;
        a = m_x.x;
        x = m_x.s.x;
        y = m_x.s.y;
        z = m_x.s.z;
        a1 = m_x.a[0];
        a2 = m_x.a[1];
    }
}
// ====
// compileToEwasm: also
// compileViaYul: also
// ----
// load() -> 0x01, 0x02, 0x03, 0x04, 0x05, 0x06
// gas irOptimized: 111179
// gas legacy: 112999
// gas legacyOptimized: 110881
// store() -> 0x01, 0x02, 0x03, 0x04, 0x05, 0x06
