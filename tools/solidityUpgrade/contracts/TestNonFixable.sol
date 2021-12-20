pragma solidity >0.4.23;

contract Storage {
    function Storage() {}
    function init() public;
    function idle();
    function destroy() public view;
}

contract VolatileStorage is Storage {
    uint[] array;
    function init() public { array.length = 3; }
    function idle() {}
    function destroy() public view {}
}