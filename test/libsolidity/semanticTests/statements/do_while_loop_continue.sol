contract test {
    function f() public pure returns(uint r) {
        uint i = 0;
        do
        {
            if (i > 0) return 0;
            i++;
            continue;
        } while (false);
        return 42;
    }
}
// ====
// compileViaYul: also
// ----
// f() -> 42
