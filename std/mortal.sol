import "./owned.sol";

contract mortal is owned {
    function kill() {
        if (msg.sender == owner)
            selfdestruct(owner);
    }
}
