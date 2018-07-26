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
// TypeError: (104-107): Location has to be storage or memory for local variables. Use an explicit data location keyword to fix this error.
// TypeError: (123-131): Location has to be storage or memory for local variables. Use an explicit data location keyword to fix this error.
