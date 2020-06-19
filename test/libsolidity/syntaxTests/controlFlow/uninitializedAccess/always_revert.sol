contract C {
    function f() internal view returns(uint[] storage a)
    {
        uint b = a[0];
        revert();
        b;
    }
}
// ----
// Warning 5740: (125-126): Unreachable code.
