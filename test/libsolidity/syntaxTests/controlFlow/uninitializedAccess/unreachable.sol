contract C {
    uint[] s;
    function f() internal returns (uint[] storage a)
    {
        revert();
        a[0] = 0;
        a = s;
    }
}
// ----
// Warning: (112-135): Unreachable code.
