/// Implements ERC 20 Token standard: https://github.com/ethereum/EIPs/issues/20
pragma solidity ^0.4.11;


/// @title Abstract token contract - Functions to be implemented by token contracts
contract Token {

    /*
     *  Events
     */
    event Transfer(address indexed from, address indexed to, uint value);
    event Approval(address indexed owner, address indexed spender, uint value);

    /*
     *  Public functions
     */
    function transfer(address to, uint value) public returns (bool);
    function transferFrom(address from, address to, uint value) public returns (bool);
    function approve(address spender, uint value) public returns (bool);
    function balanceOf(address owner) public constant returns (uint);
    function allowance(address owner, address spender) public constant returns (uint);
    function totalSupply() public constant returns (uint);
}
