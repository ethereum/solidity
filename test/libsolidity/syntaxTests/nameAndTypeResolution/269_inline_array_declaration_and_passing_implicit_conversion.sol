    contract C {
        function f() public returns (uint) {
            uint8 x = 7;
            uint16 y = 8;
            uint32 z = 9;
            uint32[3] memory ending = [x, y, z];
            return (ending[1]);
        }
    }
// ----
// Warning: (25-229): Function state mutability can be restricted to pure
