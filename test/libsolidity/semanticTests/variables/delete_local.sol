contract test {
    function delLocal() public returns (uint res){
        uint v = 5;
        delete v;
        res = v;
    }
}
// ----
// delLocal() -> 0
