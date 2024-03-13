contract C {
    uint public state = 0;
    constructor(uint _state) payable {
        state = _state;
    }
    function balance() public payable returns (uint256) {
        return address(this).balance;
    }
    function update(uint _state) public {
        state = _state;
    }
}
// ----
// constructor(), 2 wei: 3 ->
// gas irOptimized: 78996
// gas irOptimized code: 25400
// gas legacy: 83055
// gas legacy code: 65200
// gas legacyOptimized: 78898
// gas legacyOptimized code: 27800
// state() -> 3
// balance() -> 2
// balance -> 2
// update(uint256): 4
// state() -> 4
