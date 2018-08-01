pragma solidity ^0.4.11;

import "./safeMath.sol";
import "./owned.sol";

contract tokenDB is safeMath, ownedDB {

    struct allowance_s {
        uint256 amount;
        uint256 nonce;
    }

    mapping(address => mapping(address => allowance_s)) public allowance;
    mapping (address => uint256) public balanceOf;
    uint256 public totalSupply;

    function increase(address owner, uint256 value) external returns(bool success) {
        /*
            Increase of balance of the address in database. Only owner can call it.

            @owner          Address
            @value          Quantity

            @success        Was the Function successful?
        */
        require( isOwner() );
        balanceOf[owner] = safeAdd(balanceOf[owner], value);
        totalSupply = safeAdd(totalSupply, value);
        return true;
    }

    function decrease(address owner, uint256 value) external returns(bool success) {
        /*
            Decrease of balance of the address in database. Only owner can call it.

            @owner          Address
            @value          Quantity

            @success        Was the Function successful?
        */
        require( isOwner() );
        balanceOf[owner] = safeSub(balanceOf[owner], value);
        totalSupply = safeSub(totalSupply, value);
        return true;
    }

    function setAllowance(address owner, address spender, uint256 amount, uint256 nonce) external returns(bool success) {
        /*
            Set allowance in the database. Only owner can call it.

            @owner          Owner address
            @spender        Spender address
            @amount         Amount to set
            @nonce          Transaction count

            @success        Was the Function successful?
        */
        require( isOwner() );
        allowance[owner][spender].amount = amount;
        allowance[owner][spender].nonce = nonce;
        return true;
    }

    function getAllowance(address owner, address spender) public view returns(bool success, uint256 remaining, uint256 nonce) {
        /*
            Get allowance from the database.

            @owner          Owner address
            @spender        Spender address

            @success        Was the Function successful?
            @remaining      Remaining amount of the allowance
            @nonce          Transaction count
        */
        return ( true, allowance[owner][spender].amount, allowance[owner][spender].nonce );
    }
}
