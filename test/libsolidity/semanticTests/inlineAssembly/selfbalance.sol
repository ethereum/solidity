contract C {
    function f() public payable returns (uint ret) {
        assembly {
            ret := selfbalance()
        }
    }
}
// ====
// EVMVersion: >=istanbul
// ----
// f(), 254 wei -> 254
