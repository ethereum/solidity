pragma solidity >=0.0;


/// @title Abstract oracle contract - Functions to be implemented by oracles
abstract contract Oracle {

    function isOutcomeSet() virtual public view returns (bool);
    function getOutcome() virtual public view returns (int);
}
