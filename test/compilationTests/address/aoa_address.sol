pragma solidity ^0.4.23;

contract aoaAddr {

    address public owner;
    //address literal is special
    address assetId = AoA34F6feAA439EA2e92438365933067acaFf5e3b7C;
    //string should not be affected
    string public str = "AoA34F6feAA439EA2e92438365933067acaFf5e3b7C";

    mapping(address => uint256) public bs;

    constructor () public {
        owner = msg.sender;
    }
    function getAssetId() public view returns(address) {
        return assetId;
    }

    function deposit() public payable {
        //在代码内部调用上面的getAssetId()函数，应该都是内部的地址表示形式
        if (msg.asset == getAssetId() && msg.value > 0) {
            uint256 old = bs[msg.sender];
            require(old + msg.value > old);
            bs[msg.sender] = old + msg.value;
        } else {
            revert();
        }
    }
}