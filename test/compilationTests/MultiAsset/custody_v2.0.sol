pragma solidity ^0.4.24;


contract SimpleCustody {
    address public owner;
    mapping (address => mapping (address => uint256)) bank;

    event Save(address indexed _saver, address indexed _asset, uint256 _value);
    event Allocate(address indexed _receiver, address indexed _asset, uint256 _value);

    constructor () public payable {
        owner = msg.sender;
        if (msg.value > 0 || msg.assetvalue > 0) {
            intersave();
        }
    }

    function save() public payable{
        if (msg.value > 0 || msg.assetvalue > 0) {
            intersave();
        }
    }

    function intersave() internal {
        address thisC = address(this);
        uint256 totalBalance;
        uint256 value;
        if (msg.value > 0) {
            totalBalance = thisC.balance;
            value = msg.value;
        } else {
            totalBalance = thisC.balanceOf(msg.asset);
            value = msg.assetvalue;
        }
        require(totalBalance + value > totalBalance);
        uint256 old = bank[msg.sender][msg.asset];
        bank[msg.sender][msg.asset] = old + value;
        emit Save(msg.sender,msg.asset,value);
    }

    function mySaving(address asset) public view returns (uint256) {
        return bank[msg.sender][asset];
    }


    function allocate(address receiver,address asset, uint256 amount) public {
        require(msg.sender == owner);
        uint256 balance;
        address thisC = address(this);
        address aoa;
        if (asset == aoa) {
            //aoa
            balance = thisC.balance;
        } else {
            balance = thisC.balanceOf(asset);
        }
        require(balance >= amount);
        uint256 old = bank[msg.sender][asset];
        if (asset == aoa) {
            receiver.transfer(amount);
        } else {
            receiver.transferAsset(asset,amount);
        }
        bank[msg.sender][asset] = old - amount;
        emit Allocate(receiver,asset,amount);
    }

    function multialloc(address receiver,address asset, uint256 amount) public {
        allocate(receiver,address(0),amount);
        allocate(receiver,asset,amount);
    }
}