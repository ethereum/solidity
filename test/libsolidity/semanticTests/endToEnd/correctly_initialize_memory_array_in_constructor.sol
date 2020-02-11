contract C {
    bool public success;
    constructor() public {
        // Make memory dirty.
        assembly {
            for {
                let i := 0
            }
            lt(i, 64) {
                i := add(i, 1)
            } {
                mstore(msize(), not(0))
            }
        }
        uint16[3] memory c;
        require(c[0] == 0 && c[1] == 0 && c[2] == 0);
        uint16[] memory x = new uint16[](3);
        require(x[0] == 0 && x[1] == 0 && x[2] == 0);
        success = true;
    }
}

// ----
// success() -> 1
