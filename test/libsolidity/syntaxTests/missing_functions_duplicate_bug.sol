pragma abicoder               v2;

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
    constructor() {
        VoteTiming.init(1);
    }
}
// ----
// Warning 5667: (299-315): Unused function parameter. Remove or comment out the variable name to silence this warning.
