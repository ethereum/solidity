contract C {
    int256 mutex;

    function t() public returns (uint256) {
        if (mutex > 0) {
            assembly {
                mstore(0, 7)
                return(0, 0x20)
            }
        }
        mutex = 1;
        // Avoid re-executing this function if we jump somewhere.
        function() internal returns (uint)[200] memory x;
        x[0]();
        return 2;
    }
}
// ====
// compileToEwasm: also
// ----
// t() -> FAILURE, hex"4e487b71", 0x51
