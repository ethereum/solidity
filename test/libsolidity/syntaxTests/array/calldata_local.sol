contract Test {
    function f(uint[] calldata a, uint[] calldata b, bool c) pure external returns (uint, uint, uint) {
        uint[] calldata l = c ? a : b;
        return (l.length, l[0], l[1]);
    }
}
