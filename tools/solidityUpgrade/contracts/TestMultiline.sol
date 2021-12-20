pragma solidity >0.4.23;

contract Storage {
    function Storage() {}
    function init() public;
    function idle();
    function destroy() public view;
}

contract VolatileStorage is Storage {
    function init()
        public
    {}

    function idle() {}

    function destroy()
        public
        view
    {}
}