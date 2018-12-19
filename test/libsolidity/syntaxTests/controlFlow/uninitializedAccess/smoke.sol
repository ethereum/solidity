contract C {
    uint[] s;
    function f() internal returns (uint[] storage a)
    {
        a[0] = 0;
        a = s;
    }
}
// ----
// TypeError: (94-95): This variable is of storage pointer type and can be accessed without prior assignment.
