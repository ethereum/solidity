pragma solidity >=0.0;
import "../Tokens/StandardToken.sol";


/// @title Token contract - Token exchanging Trx 1:1
/// @author Stefan George - <stefan@gnosis.pm>
contract EtherToken is StandardToken {
    using Math for *;

    /*
     *  Events
     */
    event Deposit(address indexed sender, uint value);
    event Withdrawal(address indexed receiver, uint value);

    /*
     *  Constants
     */
    string public constant name = "Trx Token";
    string public constant symbol = "Trx";
    uint8 public constant decimals = 6;

    /*
     *  Public functions
     */
    /// @dev Buys tokens with Trx, exchanging them 1:1
    function deposit()
        public
        payable
    {
        balances[msg.sender] = balances[msg.sender].add(msg.value);
        totalTokens = totalTokens.add(msg.value);
        emit Deposit(msg.sender, msg.value);
    }

    /// @dev Sells tokens in exchange for Trx, exchanging them 1:1
    /// @param value Number of tokens to sell
    function withdraw(uint value)
        public
    {
        // Balance covers value
        balances[msg.sender] = balances[msg.sender].sub(value);
        totalTokens = totalTokens.sub(value);
        msg.sender.transfer(value);
        emit Withdrawal(msg.sender, value);
    }
}
