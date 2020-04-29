## 0. Smart Contract
1. The Smart Contract:
    - A smart contract in the sense of Solidity is a collection of code (its functions) and data (its state) that resides at a specific       address on the Ethereum blockchain.These Contracts are responsible for storing all of the programing logic that executes on top of       the blockchain.
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
    - The Remix IDE is a powerful open source tool that helps you write Solidity contracts straight from a browser. Remix supports the          usage of JavaScript and both local and browser usage.The latest documentation about the remix tool can be found at:
       [Remix documentation](https://remix-ide.readthedocs.io/en/latest/).
2. Remix Deploy Location:
   - JavaScript VM 
   - Injected Web3
   - Web3 Provider
3. Account: (More on the [Account](https://en.wikipedia.org/wiki/Public-key_cryptography)).
   
## 2. Deploying Our Contract With the Remix Tool
1. Create the Contract that you would like to deploy.(Extra documentation on building [Contracts](https://solidity.readthedocs.io/en/latest/introduction-to-smart-contracts.html#)).
2. Using the left hand navigation bar navigate to the Deploy & Run Transactions tab.
3. Define the location you would like to deploy your smart contract to using the environment field.
4. Select an Eth address using the account field.
5. Select the contract file you would like to depoly using the settings you have selected.
6. Hit the deploy button.
7. The Created smart contracts will then appear in gray boxs at the bottom of the remix tool.


