contract test {
    constructor() payable {}

    function getBalance() public returns (uint256 balance) {
        return address(this).balance;
    }

    function getAddress() public returns (uint256 addr) {
        return uint256(address(this));
    }
}

// ====
// compileToEwasm: also
// compileViaYul: also
// ----
// constructor(), 22774453838368691933757882222884355840 wei ->
// getAddress() -> 0xFDD67305928FCAC8D213D1E47BFA6165CD0B87B
// getBalance() -> 22774453838368691933757882222884355840
