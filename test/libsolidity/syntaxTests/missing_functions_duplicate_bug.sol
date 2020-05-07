pragma solidity ^0.6.0;

pragma experimental ABIEncoderV2;

contract Ownable {
    address private _owner;

    modifier onlyOwner() {
        require(msg.sender == _owner, "Ownable: caller is not the owner");
        _;
    }

    function renounceOwnership() public onlyOwner { }
}

library VoteTiming {
    function init(uint phaseLength) internal pure {
        require(true, "");
    }
}

contract Voting is Ownable {
    constructor() public {
        VoteTiming.init(1);
    }
}
// ----
// Warning: (324-340): Unused function parameter. Remove or comment out the variable name to silence this warning.
