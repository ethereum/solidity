pragma solidity ^0.4.11;


/// @title Abstract oracle contract - Functions to be implemented by oracles
contract Oracle {

    function isOutcomeSet() public constant returns (bool);
    function getOutcome() public constant returns (int);
}
