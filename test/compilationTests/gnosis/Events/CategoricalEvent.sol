pragma solidity >=0.0;
import "../Events/Event.sol";


/// @title Categorical event contract - Categorical events resolve to an outcome from a set of outcomes
/// @author Stefan George - <stefan@gnosis.pm>
contract CategoricalEvent is Event {

    /*
     *  Public functions
     */
    /// @dev Contract constructor validates and sets basic event properties
    /// @param _collateralToken Tokens used as collateral in exchange for outcome tokens
    /// @param _oracle Oracle contract used to resolve the event
    /// @param outcomeCount Number of event outcomes
    constructor(
        Token _collateralToken,
        Oracle _oracle,
        uint8 outcomeCount
    )
        public
        Event(_collateralToken, _oracle, outcomeCount)
    {

    }

    /// @dev Exchanges sender's winning outcome tokens for collateral tokens
    /// @return Sender's winnings
    function redeemWinnings()
        public
        override
        returns (uint winnings)
    {
        // Winning outcome has to be set
        require(isOutcomeSet);
        // Calculate winnings
        winnings = outcomeTokens[uint(outcome)].balanceOf(msg.sender);
        // Revoke tokens from winning outcome
        outcomeTokens[uint(outcome)].revoke(msg.sender, winnings);
        // Payout winnings
        require(collateralToken.transfer(msg.sender, winnings));
        emit WinningsRedemption(msg.sender, winnings);
    }

    /// @dev Calculates and returns event hash
    /// @return Event hash
    function getEventHash()
        public
        override
        view
        returns (bytes32)
    {
        return keccak256(abi.encodePacked(collateralToken, oracle, outcomeTokens.length));
    }
}
