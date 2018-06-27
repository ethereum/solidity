pragma solidity ^0.4.11;
import "../Oracles/DifficultyOracle.sol";


/// @title Difficulty oracle factory contract - Allows to create difficulty oracle contracts
/// @author Stefan George - <stefan@gnosis.pm>
contract DifficultyOracleFactory {

    /*
     *  Events
     */
    event DifficultyOracleCreation(address indexed creator, DifficultyOracle difficultyOracle, uint blockNumber);

    /*
     *  Public functions
     */
    /// @dev Creates a new difficulty oracle contract
    /// @param blockNumber Target block number
    /// @return Oracle contract
    function createDifficultyOracle(uint blockNumber)
        public
        returns (DifficultyOracle difficultyOracle)
    {
        difficultyOracle = new DifficultyOracle(blockNumber);
        emit DifficultyOracleCreation(msg.sender, difficultyOracle, blockNumber);
    }
}
