## 0. Smart Contract
1. The Smart Contract:
    - A smart contract in the sense of Solidity is a collection of code (its functions) and data (its state) that resides at a specific           address on the Ethereum blockchain. These Contracts are responsible for storing all of the programing logic that execute with the           blockchain.
```cpp
pragma solidity >=0.4.22 <0.7.0;

contract SimpleWallet {
    uint storedWallet;
    
    function getWalletValue() external view returns(uint){
        return storedWallet;
    }
    
    function setWalletValue()(unit _value) external {
        storedWallet = _value;
    }
    
    
{




```
