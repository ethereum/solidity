pragma solidity >=0.0;
import "../Oracles/FutarchyOracle.sol";


/// @title Futarchy oracle factory contract - Allows to create Futarchy oracle contracts
/// @author Stefan George - <stefan@gnosis.pm>
contract FutarchyOracleFactory {

    /*
     *  Events
     */
    event FutarchyOracleCreation(
        address indexed creator,
        FutarchyOracle futarchyOracle,
        Token collateralToken,
        Oracle oracle,
        uint8 outcomeCount,
        int lowerBound,
        int upperBound,
        MarketFactory marketFactory,
        MarketMaker marketMaker,
        uint24 fee,
        uint deadline
    );

    /*
     *  Storage
     */
    EventFactory eventFactory;

    /*
     *  Public functions
     */
    /// @dev Constructor sets event factory contract
    /// @param _eventFactory Event factory contract
    constructor(EventFactory _eventFactory)
        public
    {
        require(address(_eventFactory) != address(0));
        eventFactory = _eventFactory;
    }

    /// @dev Creates a new Futarchy oracle contract
    /// @param collateralToken Tokens used as collateral in exchange for outcome tokens
    /// @param oracle Oracle contract used to resolve the event
    /// @param outcomeCount Number of event outcomes
    /// @param lowerBound Lower bound for event outcome
    /// @param upperBound Lower bound for event outcome
    /// @param marketFactory Market factory contract
    /// @param marketMaker Market maker contract
    /// @param fee Market fee
    /// @param deadline Decision deadline
    /// @return Oracle contract
    function createFutarchyOracle(
        Token collateralToken,
        Oracle oracle,
        uint8 outcomeCount,
        int lowerBound,
        int upperBound,
        MarketFactory marketFactory,
        MarketMaker marketMaker,
        uint24 fee,
        uint deadline
    )
        public
        returns (FutarchyOracle futarchyOracle)
    {
        futarchyOracle = new FutarchyOracle(
            msg.sender,
            eventFactory,
            collateralToken,
            oracle,
            outcomeCount,
            lowerBound,
            upperBound,
            marketFactory,
            marketMaker,
            fee,
            deadline
        );
        emit FutarchyOracleCreation(
            msg.sender,
            futarchyOracle,
            collateralToken,
            oracle,
            outcomeCount,
            lowerBound,
            upperBound,
            marketFactory,
            marketMaker,
            fee,
            deadline
        );
    }
}
