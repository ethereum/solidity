pragma solidity ^0.4.11;
import "../Oracles/Oracle.sol";
import "../Events/EventFactory.sol";
import "../Markets/MarketFactory.sol";


/// @title Futarchy oracle contract - Allows to create an oracle based on market behaviour
/// @author Stefan George - <stefan@gnosis.pm>
contract FutarchyOracle is Oracle {
    using Math for *;

    /*
     *  Events
     */
    event FutarchyFunding(uint funding);
    event FutarchyClosing();
    event OutcomeAssignment(uint winningMarketIndex);

    /*
     *  Constants
     */
    uint8 public constant LONG = 1;

    /*
     *  Storage
     */
    address creator;
    Market[] public markets;
    CategoricalEvent public categoricalEvent;
    uint public deadline;
    uint public winningMarketIndex;
    bool public isSet;

    /*
     *  Modifiers
     */
    modifier isCreator () {
        // Only creator is allowed to proceed
        require(msg.sender == creator);
        _;
    }

    /*
     *  Public functions
     */
    /// @dev Constructor creates events and markets for futarchy oracle
    /// @param _creator Oracle creator
    /// @param eventFactory Event factory contract
    /// @param collateralToken Tokens used as collateral in exchange for outcome tokens
    /// @param oracle Oracle contract used to resolve the event
    /// @param outcomeCount Number of event outcomes
    /// @param lowerBound Lower bound for event outcome
    /// @param upperBound Lower bound for event outcome
    /// @param marketFactory Market factory contract
    /// @param marketMaker Market maker contract
    /// @param fee Market fee
    /// @param _deadline Decision deadline
    function FutarchyOracle(
        address _creator,
        EventFactory eventFactory,
        Token collateralToken,
        Oracle oracle,
        uint8 outcomeCount,
        int lowerBound,
        int upperBound,
        MarketFactory marketFactory,
        MarketMaker marketMaker,
        uint24 fee,
        uint _deadline
    )
        public
    {
        // Deadline is in the future
        require(_deadline > now);
        // Create decision event
        categoricalEvent = eventFactory.createCategoricalEvent(collateralToken, this, outcomeCount);
        // Create outcome events
        for (uint8 i = 0; i < categoricalEvent.getOutcomeCount(); i++) {
            ScalarEvent scalarEvent = eventFactory.createScalarEvent(
                categoricalEvent.outcomeTokens(i),
                oracle,
                lowerBound,
                upperBound
            );
            markets.push(marketFactory.createMarket(scalarEvent, marketMaker, fee));
        }
        creator = _creator;
        deadline = _deadline;
    }

    /// @dev Funds all markets with equal amount of funding
    /// @param funding Amount of funding
    function fund(uint funding)
        public
        isCreator
    {
        // Buy all outcomes
        require(   categoricalEvent.collateralToken().transferFrom(creator, this, funding)
                && categoricalEvent.collateralToken().approve(categoricalEvent, funding));
        categoricalEvent.buyAllOutcomes(funding);
        // Fund each market with outcome tokens from categorical event
        for (uint8 i = 0; i < markets.length; i++) {
            Market market = markets[i];
            // Approve funding for market
            require(market.eventContract().collateralToken().approve(market, funding));
            market.fund(funding);
        }
        FutarchyFunding(funding);
    }

    /// @dev Closes market for winning outcome and redeems winnings and sends all collateral tokens to creator
    function close()
        public
        isCreator
    {
        // Winning outcome has to be set
        Market market = markets[uint(getOutcome())];
        require(categoricalEvent.isOutcomeSet() && market.eventContract().isOutcomeSet());
        // Close market and transfer all outcome tokens from winning outcome to this contract
        market.close();
        market.eventContract().redeemWinnings();
        market.withdrawFees();
        // Redeem collateral token for winning outcome tokens and transfer collateral tokens to creator
        categoricalEvent.redeemWinnings();
        require(categoricalEvent.collateralToken().transfer(creator, categoricalEvent.collateralToken().balanceOf(this)));
        FutarchyClosing();
    }

    /// @dev Allows to set the oracle outcome based on the market with largest long position
    function setOutcome()
        public
    {
        // Outcome is not set yet and deadline has passed
        require(!isSet && deadline <= now);
        // Find market with highest marginal price for long outcome tokens
        uint highestMarginalPrice = markets[0].marketMaker().calcMarginalPrice(markets[0], LONG);
        uint highestIndex = 0;
        for (uint8 i = 1; i < markets.length; i++) {
            uint marginalPrice = markets[i].marketMaker().calcMarginalPrice(markets[i], LONG);
            if (marginalPrice > highestMarginalPrice) {
                highestMarginalPrice = marginalPrice;
                highestIndex = i;
            }
        }
        winningMarketIndex = highestIndex;
        isSet = true;
        OutcomeAssignment(winningMarketIndex);
    }

    /// @dev Returns if winning outcome is set
    /// @return Is outcome set?
    function isOutcomeSet()
        public
        constant
        returns (bool)
    {
        return isSet;
    }

    /// @dev Returns winning outcome
    /// @return Outcome
    function getOutcome()
        public
        constant
        returns (int)
    {
        return int(winningMarketIndex);
    }
}
