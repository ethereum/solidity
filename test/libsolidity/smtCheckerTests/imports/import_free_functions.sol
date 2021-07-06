==== Source: Address.sol ====
pragma solidity ^0.8.0;
function s() pure {}
==== Source: ERC20.sol ====
pragma solidity ^0.8.0;

import "./Address.sol";

function sub(uint256 a, uint256 b) pure returns (uint256) {
    return a - b;
}

contract ERC20 {
    mapping (address => uint256) private _balances;

    function transferFrom(uint256 amount) public view {
        sub(_balances[msg.sender], amount);
    }
}
// ====
// SMTEngine: all
// ----
// Warning 3944: (ERC20.sol:121-126): CHC: Underflow (resulting value less than 0) happens here.\nCounterexample:\n\namount = 1\n\nTransaction trace:\nERC20.constructor()\nERC20.transferFrom(1)\n    ERC20.sol:sub(0, 1) -- internal call
