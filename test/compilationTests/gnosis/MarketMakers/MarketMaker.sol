pragma solidity ^0.4.11;
import "../Markets/Market.sol";


/// @title Abstract market maker contract - Functions to be implemented by market maker contracts
contract MarketMaker {

    /*
     *  Public functions
     */
    function calcCost(Market market, uint8 outcomeTokenIndex, uint outcomeTokenCount) public constant returns (uint);
    function calcProfit(Market market, uint8 outcomeTokenIndex, uint outcomeTokenCount) public constant returns (uint);
    function calcMarginalPrice(Market market, uint8 outcomeTokenIndex) public constant returns (uint);
}
