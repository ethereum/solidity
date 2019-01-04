pragma solidity ^0.4.24;

contract AssetMgr {


    function MyAssetBalance(address asset) public returns(uint256){
        return msg.sender.balanceOf(asset);
    }

    function TransferAsset(address receiver, address asset, uint256 value) public payable returns(bool) {
        address theContract = this;
        if (theContract.balanceOf(asset) >= value && value > 0) {
            receiver.transferAsset(asset,value);
            return true;
        } else {
            return false;
        }
    }

    function SendAsset(address receiver, address asset, uint256 value) public payable returns(bool) {
        address theContract = this;
        if (theContract.balanceOf(asset) >= value && value > 0) {
            return receiver.sendAsset(asset,value);
        } else {
            return false;
        }
    }

}