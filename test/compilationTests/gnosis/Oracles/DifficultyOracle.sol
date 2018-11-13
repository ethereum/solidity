pragma solidity >=0.0;
import "../Oracles/Oracle.sol";


/// @title Difficulty oracle contract - Oracle to resolve difficulty events at given block
/// @author Stefan George - <stefan@gnosis.pm>
contract DifficultyOracle is Oracle {

    /*
     *  Events
     */
    event OutcomeAssignment(uint difficulty);

    /*
     *  Storage
     */
    uint public blockNumber;
    uint public difficulty;

    /*
     *  Public functions
     */
    /// @dev Contract constructor validates and sets target block number
    /// @param _blockNumber Target block number
    constructor(uint _blockNumber)
        public
    {
        // Block has to be in the future
        require(_blockNumber > block.number);
        blockNumber = _blockNumber;
    }

    /// @dev Sets difficulty as winning outcome for specified block
    function setOutcome()
        public
    {
        // Block number was reached and outcome was not set yet
        require(block.number >= blockNumber && difficulty == 0);
        difficulty = block.difficulty;
        emit OutcomeAssignment(difficulty);
    }

    /// @dev Returns if difficulty is set
    /// @return Is outcome set?
    function isOutcomeSet()
        public
        view
        returns (bool)
    {
        // Difficulty is always bigger than 0
        return difficulty > 0;
    }

    /// @dev Returns difficulty
    /// @return Outcome
    function getOutcome()
        public
        view
        returns (int)
    {
        return int(difficulty);
    }
}
