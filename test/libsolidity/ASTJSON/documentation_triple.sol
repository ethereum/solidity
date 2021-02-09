contract C {
    /// test
    uint a;
    function f() public pure returns (uint x) {
        /// test2
        for (uint i = 0; i < 20; i++) {
            /// tee
            /// s "t" 3
            x *= 2;
        }
        /** tes "t4" */
        return x;
    }
}

// ----
