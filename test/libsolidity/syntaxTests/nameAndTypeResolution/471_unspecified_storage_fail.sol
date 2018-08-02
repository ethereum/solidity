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
// TypeError: (104-107): Storage location must be one of "storage", "memory" for variable, but none was given.
