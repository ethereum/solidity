contract C {
    function f() public pure {
        /// Documentation for x; will appear in ast json
        uint x = 1;
        for (
            /// documentation for i; will not appear in ast json
            uint i = 0;
            i < 10;
            ++i
        ) {
            /// documentation for j; will appear in ast json
            uint j = 0;
        }
    }
    function g(
        /// documentation for param1; will not appear in ast json
        uint param1,
        /// documentation for param2; will not appear in ast json
        uint param2,
        /// documentation for param3; will not appear in ast json
        uint param3
    ) public {}
}

// ----
