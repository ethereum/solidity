## 0. Smart Contract
1. The Smart Contract:
    - A smart contract in the sense of Solidity is a collection of code (its functions) and data (its state) that resides at a specific           address on the Ethereum blockchain.These Contracts are responsible for storing all of the programing logic that executes on top of         the blockchain.
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
       [Remix documentation](https://remix-ide.readthedocs.io/en/latest/).
2. Remix Deploy Location:
   - Fist 
   
## 2. Step by Step using Remix how to deploy a Contract
0. Create the Contract that you would like to deploy. Documentation on how to create a contracts can be found here ([Contracts](https://solidity.readthedocs.io/en/latest/introduction-to-smart-contracts.html#))
1. Using the left hand navigation bar navigate to the Deploy & Run Transactions tab.

       
