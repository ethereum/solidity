pragma solidity >0.4.23;

contract Updateable {
    function run() public view returns (bool);
    function update() public;
}

contract Upgradable {
    function run() public view returns (bool);
    function upgrade();
}

contract Source is Updateable, Upgradable {
    function Source() public {}

    function run()
        public
        view
        returns (bool) {}

    function update() {}
    function upgrade() {}
}