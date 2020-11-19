pragma solidity >=0.0;

interface Deed {

    function setOwner(address payable newOwner) external;
    function setRegistrar(address newRegistrar) external;
    function setBalance(uint newValue, bool throwOnFailure) external;
    function closeDeed(uint refundRatio) external;
    function destroyDeed() external;

    function owner() external view returns (address);
    function previousOwner() external view returns (address);
    function value() external view returns (uint);
    function creationDate() external view returns (uint);

}
