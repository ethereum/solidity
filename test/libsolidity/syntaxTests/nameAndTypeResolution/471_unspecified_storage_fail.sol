contract C {
    struct S { uint a; }
    S m_x;
    uint[] m_y;
    function f() view public {
        S x = m_x;
        uint[] y = m_y;
        x; y;
    }
}
// ----
// TypeError 6651: (104-107): Data location must be "storage", "memory" or "calldata" for variable, but none was given.
// TypeError 6651: (123-131): Data location must be "storage", "memory" or "calldata" for variable, but none was given.
