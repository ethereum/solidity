contract Other {
    constructor() payable {
    }
    function getAddress() public returns (address) {
        return address(this);
    }
}
contract ClientReceipt {
    Other other;
    constructor() payable {
        other = new Other{value:500}();
    }
    function getAddress() public returns (address) {
        return other.getAddress();
    }
}
// ====
// compileViaYul: also
// ----
// constructor(), 2000 wei ->
// gas irOptimized: 187946
// gas legacy: 235195
// gas legacyOptimized: 176766
// balance -> 1500
// gas irOptimized: 191881
// gas legacy: 235167
// gas legacyOptimized: 180756
// getAddress() -> 0xf01f7809444bd9a93a854361c6fae3f23d9e23db
// balance: 0xf01f7809444bd9a93a854361c6fae3f23d9e23db -> 500
