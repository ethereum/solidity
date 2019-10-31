/*
This file is part of the DAO.

The DAO is free software: you can redistribute it and/or modify
it under the terms of the GNU lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

The DAO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU lesser General Public License for more details.

You should have received a copy of the GNU lesser General Public License
along with the DAO.  If not, see <http://www.gnu.org/licenses/>.
*/


/*
Basic account, used by the DAO contract to separately manage both the rewards
and the extraBalance accounts.
*/

contract ManagedAccountInterface {
    // The only address with permission to withdraw from this account
    address public owner;
    // If true, only the owner of the account can receive ether from it
    bool public payOwnerOnly;
    // The sum of ether (in wei) which has been sent to this contract
    uint public accumulatedInput;

    /// @notice Sends `_amount` of wei to _recipient
    /// @param _amount The amount of wei to send to `_recipient`
    /// @param _recipient The address to receive `_amount` of wei
    /// @return True if the send completed
    function payOut(address payable _recipient, uint _amount) public returns (bool);

    event PayOut(address indexed _recipient, uint _amount);
}


contract ManagedAccount is ManagedAccountInterface{

    // The constructor sets the owner of the account
    constructor(address _owner, bool _payOwnerOnly) public {
        owner = _owner;
        payOwnerOnly = _payOwnerOnly;
    }

    // When the contract receives a transaction without data this is called.
    // It counts the amount of ether it receives and stores it in
    // accumulatedInput.
    function() external payable {
        accumulatedInput += msg.value;
    }

    function payOut(address payable _recipient, uint _amount) public override returns (bool) {
        if (msg.sender != owner || (payOwnerOnly && _recipient != owner))
            revert();
        (bool success,) = _recipient.call.value(_amount)("");
        if (success) {
            emit PayOut(_recipient, _amount);
            return true;
        } else {
            return false;
        }
    }
}
