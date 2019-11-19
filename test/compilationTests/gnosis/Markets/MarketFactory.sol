pragma solidity >=0.0;
import "../Events/Event.sol";
import "../MarketMakers/MarketMaker.sol";
import "../Markets/Market.sol";


/// @title Abstract market factory contract - Functions to be implemented by market factories
abstract contract MarketFactory {

    /*
     *  Events
     */
    event MarketCreation(address indexed creator, Market market, Event eventContract, MarketMaker marketMaker, uint24 fee);

    /*
     *  Public functions
     */
    function createMarket(Event eventContract, MarketMaker marketMaker, uint24
fee) virtual public returns (Market);
}
