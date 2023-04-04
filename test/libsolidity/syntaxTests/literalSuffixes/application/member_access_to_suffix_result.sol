function functionSuffix(uint) pure suffix returns (function () external) {}
function addressSuffix(uint) pure suffix returns (address payable) {}
function arraySuffix(uint) pure suffix returns (uint[] memory) {}

contract C {
    bytes4 a = (1000 functionSuffix).selector;
    address b = (1000 functionSuffix).address;
    uint c = (1000 arraySuffix).length;
    bool d = (1000 addressSuffix).send(1);
}
