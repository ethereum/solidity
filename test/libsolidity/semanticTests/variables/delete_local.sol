contract test {
    function delLocal() public returns (uint res){
        uint v = 5;
        delete v;
        res = v;
    }
}
// ====
// compileToEwasm: also
// compileViaYul: also
// ----
// delLocal() -> 0
