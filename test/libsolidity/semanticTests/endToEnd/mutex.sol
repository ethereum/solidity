contract mutexed {
    bool locked;
    modifier protected {
        if (locked) revert();
        locked = true;
        _;
        locked = false;
    }
}
contract Fund is mutexed {
    uint shares;
    constructor() public payable {
        shares = msg.value;
    }

    function withdraw(uint amount) public protected returns(uint) {
        // NOTE: It is very bad practice to write this function this way.
        // Please refer to the documentation of how to do this properly.
        if (amount > shares) revert();
        (bool success, ) = msg.sender.call.value(amount)("");
        require(success);
        shares -= amount;
        return shares;
    }

    function withdrawUnprotected(uint amount) public returns(uint) {
        // NOTE: It is very bad practice to write this function this way.
        // Please refer to the documentation of how to do this properly.
        if (amount > shares) revert();
        (bool success, ) = msg.sender.call.value(amount)("");
        require(success);
        shares -= amount;
        return shares;
    }
}
contract Attacker {
    Fund public fund;
    uint callDepth;
    bool protected;

    function setProtected(bool _protected) public {
        protected = _protected;
    }
    constructor(Fund _fund) public {
        fund = _fund;
    }

    function attack() public returns(uint) {
        callDepth = 0;
        return attackInternal();
    }

    function attackInternal() internal returns(uint) {
        if (protected)
            return fund.withdraw(10);
        else
            return fund.withdrawUnprotected(10);
    }
    fallback() external payable {
        callDepth++;
        if (callDepth < 4)
            attackInternal();
    }
}

// ----

contract mutexed {
    bool locked;
    modifier protected {
        if (locked) revert();
        locked = true;
        _;
        locked = false;
    }
}
contract Fund is mutexed {
    uint shares;
    constructor() public payable {
        shares = msg.value;
    }

    function withdraw(uint amount) public protected returns(uint) {
        // NOTE: It is very bad practice to write this function this way.
        // Please refer to the documentation of how to do this properly.
        if (amount > shares) revert();
        (bool success, ) = msg.sender.call.value(amount)("");
        require(success);
        shares -= amount;
        return shares;
    }

    function withdrawUnprotected(uint amount) public returns(uint) {
        // NOTE: It is very bad practice to write this function this way.
        // Please refer to the documentation of how to do this properly.
        if (amount > shares) revert();
        (bool success, ) = msg.sender.call.value(amount)("");
        require(success);
        shares -= amount;
        return shares;
    }
}
contract Attacker {
    Fund public fund;
    uint callDepth;
    bool protected;

    function setProtected(bool _protected) public {
        protected = _protected;
    }
    constructor(Fund _fund) public {
        fund = _fund;
    }

    function attack() public returns(uint) {
        callDepth = 0;
        return attackInternal();
    }

    function attackInternal() internal returns(uint) {
        if (protected)
            return fund.withdraw(10);
        else
            return fund.withdrawUnprotected(10);
    }
    fallback() external payable {
        callDepth++;
        if (callDepth < 4)
            attackInternal();
    }
}

// ----
// setProtected(bool): true -> 
// setProtected(bool):"1" -> ""
// attack() -> 
// attack():"" -> ""
// setProtected(bool): false -> 
// setProtected(bool):"0" -> ""
// attack() -> 460
// attack():"" -> "460"
