contract C
{
    error E(uint);

    function r() internal returns (uint)
    {
        assembly { mstore(0, 7) return (0, 32) }
        return 42;
    }

    function f() public returns (uint)
    {
        require(false, E(r()));
        return 42;
    }

    function g() public returns (uint)
    {
        require(true, E(r()));
        return 42;
    }
}

// ----
// f() -> 7
// g() -> 7
