pragma solidity>=0.0;

import "./Deed.sol";

/**
 * @title Deed to hold ether in exchange for ownership of a node
 * @dev The deed can be controlled only by the registrar and can only send ether back to the owner.
 */
contract DeedImplementation is Deed {

    address payable constant burn = address(0xdead);

    address payable private _owner;
    address private _previousOwner;
    address private _registrar;

    uint private _creationDate;
    uint private _value;

    bool active;

    event OwnerChanged(address newOwner);
    event DeedClosed();

    modifier onlyRegistrar {
        require(msg.sender == _registrar);
        _;
    }

    modifier onlyActive {
        require(active);
        _;
    }

    /// Solidity upgrade: Warning on ignored constructor
    /// visibility.
    constructor(address payable initialOwner) public payable {
        _owner = initialOwner;
        _registrar = msg.sender;
        /// Solidity upgrade: `now` changed to `block.timestamp`
        _creationDate = block.timestamp;
        active = true;
        _value = msg.value;
    }

    /// Solidity upgrade: Function missing override specifier
    function setOwner(address payable newOwner) override external onlyRegistrar {
        require(newOwner != address(0x0));
        _previousOwner = _owner;  // This allows contracts to check who sent them the ownership
        _owner = newOwner;
        emit OwnerChanged(newOwner);
    }

    /// Solidity upgrade: Function missing override specifier
    function setRegistrar(address newRegistrar) override external onlyRegistrar {
        _registrar = newRegistrar;
    }

    /// Solidity upgrade: Function missing override specifier
    function setBalance(uint newValue, bool throwOnFailure) override external onlyRegistrar onlyActive {
        // Check if it has enough balance to set the value
        require(_value >= newValue);
        _value = newValue;
        // Send the difference to the owner
        require(_owner.send(address(this).balance - newValue) || !throwOnFailure);
    }

    /**
     * @dev Close a deed and refund a specified fraction of the bid value
     *
     * @param refundRatio The amount*1/1000 to refund
     */
    /// Solidity upgrade: Function missing override specifier
    function closeDeed(uint refundRatio) override external onlyRegistrar onlyActive {
        active = false;
        require(burn.send(((1000 - refundRatio) * address(this).balance)/1000));
        emit DeedClosed();
        _destroyDeed();
    }

    /**
     * @dev Close a deed and refund a specified fraction of the bid value
     */
    /// Solidity upgrade: Function missing override specifier
    function destroyDeed() override external {
        _destroyDeed();
    }

    /// Solidity upgrade: Function missing override identifier
    function owner() override external view returns (address) {
        return _owner;
    }

    /// Solidity upgrade: Function missing override identifier
    function previousOwner() override external view returns (address) {
        return _previousOwner;
    }

    /// Solidity upgrade: Function missing override specifier
    function value() override external view returns (uint) {
        return _value;
    }

    /// Solidity upgrade: Function missing override specifier
    function creationDate() override external view returns (uint) {
        /// Solidity upgrade: Missing return statement
        return _creationDate;
    }

    function _destroyDeed() internal {
        require(!active);

        // Instead of selfdestruct(owner), invoke owner fallback function to allow
        // owner to log an event if desired; but owner should also be aware that
        // its fallback function can also be invoked by setBalance
        if (_owner.send(address(this).balance)) {
            selfdestruct(burn);
        }
    }
}
