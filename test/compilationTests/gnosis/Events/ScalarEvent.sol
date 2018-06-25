pragma solidity ^0.4.11;
import "../Events/Event.sol";


/// @title Scalar event contract - Scalar events resolve to a number within a range
/// @author Stefan George - <stefan@gnosis.pm>
contract ScalarEvent is Event {
    using Math for *;

    /*
     *  Constants
     */
    uint8 public constant SHORT = 0;
    uint8 public constant LONG = 1;
    uint24 public constant OUTCOME_RANGE = 1000000;

    /*
     *  Storage
     */
    int public lowerBound;
    int public upperBound;

    /*
     *  Public functions
     */
    /// @dev Contract constructor validates and sets basic event properties
    /// @param _collateralToken Tokens used as collateral in exchange for outcome tokens
    /// @param _oracle Oracle contract used to resolve the event
    /// @param _lowerBound Lower bound for event outcome
    /// @param _upperBound Lower bound for event outcome
    constructor(
        Token _collateralToken,
        Oracle _oracle,
        int _lowerBound,
        int _upperBound
    )
        public
        Event(_collateralToken, _oracle, 2)
    {
        // Validate bounds
        require(_upperBound > _lowerBound);
        lowerBound = _lowerBound;
        upperBound = _upperBound;
    }

    /// @dev Exchanges sender's winning outcome tokens for collateral tokens
    /// @return Sender's winnings
    function redeemWinnings()
        public
        returns (uint winnings)
    {
        // Winning outcome has to be set
        require(isOutcomeSet);
        // Calculate winnings
        uint24 convertedWinningOutcome;
        // Outcome is lower than defined lower bound
        if (outcome < lowerBound)
            convertedWinningOutcome = 0;
        // Outcome is higher than defined upper bound
        else if (outcome > upperBound)
            convertedWinningOutcome = OUTCOME_RANGE;
        // Map outcome to outcome range
        else
            convertedWinningOutcome = uint24(OUTCOME_RANGE * (outcome - lowerBound) / (upperBound - lowerBound));
        uint factorShort = OUTCOME_RANGE - convertedWinningOutcome;
        uint factorLong = OUTCOME_RANGE - factorShort;
        uint shortOutcomeTokenCount = outcomeTokens[SHORT].balanceOf(msg.sender);
        uint longOutcomeTokenCount = outcomeTokens[LONG].balanceOf(msg.sender);
        winnings = shortOutcomeTokenCount.mul(factorShort).add(longOutcomeTokenCount.mul(factorLong)) / OUTCOME_RANGE;
        // Revoke all outcome tokens
        outcomeTokens[SHORT].revoke(msg.sender, shortOutcomeTokenCount);
        outcomeTokens[LONG].revoke(msg.sender, longOutcomeTokenCount);
        // Payout winnings to sender
        require(collateralToken.transfer(msg.sender, winnings));
        WinningsRedemption(msg.sender, winnings);
    }

    /// @dev Calculates and returns event hash
    /// @return Event hash
    function getEventHash()
        public
        constant
        returns (bytes32)
    {
        return keccak256(collateralToken, oracle, lowerBound, upperBound);
    }
}
