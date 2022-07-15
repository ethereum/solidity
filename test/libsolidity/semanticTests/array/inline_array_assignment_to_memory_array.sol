contract C {
    function f() public returns (uint8[3] memory) {
        uint8[3][] memory x = new uint8[3][](1);
        x[0] = [1, 2, 3];
        return x[0];
    }

    function g() public returns (uint8[] memory) {
	uint8[][] memory x = new uint8[][](1);
	x[0] = [4, 5, 6];
        return x[0];
    }
}
// ====
// compileViaYul: also
// ----
// f() -> 1, 2, 3
// g() -> 0x20, 3, 4, 5, 6
