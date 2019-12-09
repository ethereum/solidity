pragma solidity >0.4.23;

contract Storage {
    function Storage() public {}
    function start();
    function state() public view returns (bool);
    function stop() public;
}

contract Observable {
    function state() public view returns (bool);
}

contract VolatileStorage is Storage, Observable {
    function start() {}
    function state() public view returns (bool) {}
    function stop() {}
}