library L {
    struct S
    {
        function(uint) internal returns (uint)[] x;
    }
    function f(S storage s) public { }
}
// ----
// TypeError: (104-115): Internal type is not allowed for public or external functions.
