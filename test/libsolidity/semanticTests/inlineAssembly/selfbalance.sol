contract C {
    function f() public payable returns (uint ret) {
        assembly {
            ret := selfbalance()
        }
    }
}
// ====
// EVMVersion: >=istanbul
// compileViaYul: also
// ----
// f(), 254 ether -> 254
