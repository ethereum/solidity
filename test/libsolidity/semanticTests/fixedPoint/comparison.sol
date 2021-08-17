contract C {
    function test() public pure returns (bool) {
        fixed64x4 x = fixed64x4(-1.12346789);
        fixed64x4 y = -1.1235;
        ufixed32x4 z = 1;
        require(x == x);
        require(x > y);
        require(x >= y);
        require(x != y);
        require(y <= x);
        require(!(x < y));
        require(y < z);
        require(x < z);
        require(z >= x);
        require(z == z);
        return true;
    }
}
// ====
// compileViaYul: also
// ----
// test() -> true
