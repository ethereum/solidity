contract C {
    int8 immutable a = -2;
    bytes2 immutable b = "ab";
    function() internal returns (uint) immutable f = g;
    function viaasm() view external returns (bytes32 x, bytes32 y) {
        int8 _a = a;
        bytes2 _b = b;
        assembly { x := _a y := _b }
    }
    function g() internal pure returns (uint) { return 2; }
}
// ----
// viaasm() -> 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe, 0x6162000000000000000000000000000000000000000000000000000000000000
