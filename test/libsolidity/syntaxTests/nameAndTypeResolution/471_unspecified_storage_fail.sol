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
// TypeError: (104-107): Data location must be "storage" or "memory" for variable, but none was given.
// TypeError: (123-131): Data location must be "storage" or "memory" for variable, but none was given.
