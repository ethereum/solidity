contract C {
    uint immutable x = 1;

    function readX() internal view returns(uint) {
        return x + 3;
    }
}
// ----
