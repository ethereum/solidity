pragma solidity ^0.4.11;
import "../Oracles/UltimateOracle.sol";


/// @title Ultimate oracle factory contract - Allows to create ultimate oracle contracts
/// @author Stefan George - <stefan@gnosis.pm>
contract UltimateOracleFactory {

    /*
     *  Events
     */
    event UltimateOracleCreation(
        address indexed creator,
        UltimateOracle ultimateOracle,
        Oracle oracle,
        Token collateralToken,
        uint8 spreadMultiplier,
        uint challengePeriod,
        uint challengeAmount,
        uint frontRunnerPeriod
    );

    /*
     *  Public functions
     */
    /// @dev Creates a new ultimate Oracle contract
    /// @param oracle Oracle address
    /// @param collateralToken Collateral token address
    /// @param spreadMultiplier Defines the spread as a multiple of the money bet on other outcomes
    /// @param challengePeriod Time to challenge oracle outcome
    /// @param challengeAmount Amount to challenge the outcome
    /// @param frontRunnerPeriod Time to overbid the front-runner
    /// @return Oracle contract
    function createUltimateOracle(
        Oracle oracle,
        Token collateralToken,
        uint8 spreadMultiplier,
        uint challengePeriod,
        uint challengeAmount,
        uint frontRunnerPeriod
    )
        public
        returns (UltimateOracle ultimateOracle)
    {
        ultimateOracle = new UltimateOracle(
            oracle,
            collateralToken,
            spreadMultiplier,
            challengePeriod,
            challengeAmount,
            frontRunnerPeriod
        );
        emit UltimateOracleCreation(
            msg.sender,
            ultimateOracle,
            oracle,
            collateralToken,
            spreadMultiplier,
            challengePeriod,
            challengeAmount,
            frontRunnerPeriod
        );
    }
}
