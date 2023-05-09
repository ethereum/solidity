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
// ----
// constructor(), 2000 wei ->
// gas irOptimized: 171865
// gas legacy: 230038
// gas legacyOptimized: 173883
// balance -> 1500
// gas irOptimized: 191881
// gas legacy: 235167
// gas legacyOptimized: 180756
// getAddress() -> 0x137aa4dfc0911524504fcd4d98501f179bc13b4a
// balance: 0x137aa4dfc0911524504fcd4d98501f179bc13b4a -> 500
