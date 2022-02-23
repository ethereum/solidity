contract C {
    uint immutable x = 1;

    function readX() internal pure returns(uint) {
        return x + 3;
    }
}
// ----
