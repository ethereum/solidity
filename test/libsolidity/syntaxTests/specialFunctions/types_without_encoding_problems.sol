contract C {
    uint[3] sarr;
    function f() view public {
        uint[3] memory arr;
        bytes32 h = keccak256(this.f, arr, sarr);
        h;
    }
}
// ----
