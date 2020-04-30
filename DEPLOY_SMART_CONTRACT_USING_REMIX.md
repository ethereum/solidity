## 0. Smart Contract
1. The Smart Contract:
    - A smart contract in the sense of Solidity is a collection of code (its functions) and data (its state) that resides at a specific       address on the Ethereum blockchain.These Contracts are responsible for storing all of the programing logic that executes on top of       the blockchain.
2. Key Criteria of a Smart Contract:
    - It must have open-source code.
    - Code is stored in a blockchain.
    - Code cannot be changed once its deployed.
    - Eexecution of the code is automated.
    - Results are Visable in a decentralized blockchain.
    - Execution resulst are unalterable.
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
2. Choosing a Deploy Location:
   -  There are three options Remix Provides Users with:
        1. JavaScript VM - JavaScript Virtual Machine module that allows compiling and running with the V8 Virtual Machines contexts.
            This method is most useful when running an isolated Ethereum node and conducting tests on contracts.  
         
        2. Injected Web3 - This method will try to use the WEB3 provider embedded in the browser. This can be configured to connect to the          MainNet or TestNet. This method allows for interaction using a real network. ([TestNet](https://docs.ethhub.io/using-ethereum/test-networks/))
        
        3. Web3 Provider - This method provides the largest amount of control and allows the user to connect to their own node.([Web3 Provider](https://web3js.readthedocs.io/en/v1.2.0/web3-eth.html))
  
3. Account:This is the identification method remix uses to identify users. The address that is generated in the account field is most similar to a public key(More on the [Account](https://en.wikipedia.org/wiki/Public-key_cryptography)).
   
## 2. Deploying a Smart Contract With the Remix Tool
1. Create a Solidity Smart Contract that you would like to deploy (this should be in the form of a .sol file).(Extra documentation on building [Contracts](https://solidity.readthedocs.io/en/latest/introduction-to-smart-contracts.html#)).
2. Using the left hand navigation bar navigate to the Deploy & Run Transactions tab.
3. Define the location you would like to deploy your smart contract to using the environment field.
4. Select an Eth address using the account field.
5. Select the contract file you would like to deploy using the settings you have selected.
6. Hit the deploy button.
7. The Created smart contracts will then appear in generated gray boxes at the bottom of the remix tool.

## 3. Deploying a Smart Contract Using the Remix, MetaMask and Ropsten TestNet.
1. Install the MetaMask tool [MetaMask](https://metamask.io/) (Allows for communication with the blockchain through a smart contract).
2. Connect to the [Ropsten](https://ropsten.etherscan.io/) TestNet(MetaMask will connect you to the MainNet but for testing purposes we want use the TestNet).
3. Create A Smart Contract using the Remix IDE (this should be in the form of a .sol file).(Extra documentation on building [Contracts](https://solidity.readthedocs.io/en/latest/introduction-to-smart-contracts.html#)).
4. Request Ether from MetaMask [Request](https://faucet.metamask.io/) (You will need atleast 1 ether to run on the TestNet).
5. Using the Remix IDE hit the + sign at the top left of the browser (Create and Deploy). Click deploy to Ropsten and then Confirm the transaction using MetaMask.

## 4. Deploying a Smart Contract Using the Solidity Compiler.
1. Install the Solidity Compiler (Solc) Using the following Node Command ```npm install -g solc ```.
2. Intall Geth (A command line interface that is used to run a full Eth Node) Installation guide for Windows can be found [Here](https://geth.ethereum.org/downloads/).
3. Create a Solidity Smart conract in your local development tool.
4. Compile your ABI and bin and display the contents of the compiled files.
5. Create your Geth Node ([Online Instructions](https://medium.com/mercuryprotocol/dev-highlights-of-this-week-cb33e58c745f)).
6. Deploy The Smart Contract using Geth.


