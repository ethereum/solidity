contract A {
    function b() public returns (uint i) {
        assembly {
            i := 3_200_000_000
        }
    }

    function c() public returns (uint i) {
        assembly {
            i := 3200000000
        }
    }

    function d() public returns (int i) {
        assembly {
            i := 3_200_000_000
        }
    }
}
// ====
// compileToEwasm: also
// ----
// b() -> 3200000000
// c() -> 3200000000
// d() -> 3200000000