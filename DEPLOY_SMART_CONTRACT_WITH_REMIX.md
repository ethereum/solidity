## 0. Smart Contract
1. The Smart Contract:
    - A smart contract in the sense of Solidity is a collection of code (its functions) and data (its state) that resides at a specific           address on the Ethereum blockchain.These Contracts are responsible for storing all of the programing logic that executes with the           blockchain.
### Example Contract
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
## 1. Remix Compiler
1. Introduction to Remix:
    - The Remix IDE is a powerful open source tool that helps you write Solidity contracts straight from a browser. Remix supports the            usage of JavaScript and both local and browser usage.The latest documentation about the remix tool can be found at:
       https://remix-ide.readthedocs.io/en/latest/
