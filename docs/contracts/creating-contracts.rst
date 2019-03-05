.. index:: ! contract;creation, constructor

******************
Creating Contracts
******************

Contracts can be created "from outside" via Ethereum transactions or from within Solidity contracts.

IDEs, such as `Remix <https://remix.ethereum.org/>`_, make the creation process seamless using UI elements.

Creating contracts programmatically on Ethereum is best done via using the JavaScript API `web3.js <https://github.com/ethereum/web3.js>`_.
It has a function called `web3.eth.Contract <https://web3js.readthedocs.io/en/1.0/web3-eth-contract.html#new-contract>`_
to facilitate contract creation.

When a contract is created, its :ref:`constructor <constructor>`  (a function declared with the ``constructor`` keyword) is executed once.

A constructor is optional. Only one constructor is allowed, which means
overloading is not supported.

After the constructor has executed, the final code of the contract is deployed to the
blockchain. This code includes all public and external functions and all functions
that are reachable from there through function calls. The deployed code does not
include the constructor code or internal functions only called from the constructor.

.. index:: constructor;arguments

Internally, constructor arguments are passed :ref:`ABI encoded <ABI>` after the code of
the contract itself, but you do not have to care about this if you use ``web3.js``.

If a contract wants to create another contract, the source code
(and the binary) of the created contract has to be known to the creator.
This means that cyclic creation dependencies are impossible.

::

    pragma solidity >=0.4.22 <0.7.0;

    contract OwnedToken {
        // `TokenCreator` is a contract type that is defined below.
        // It is fine to reference it as long as it is not used
        // to create a new contract.
        TokenCreator creator;
        address owner;
        bytes32 name;

        // This is the constructor which registers the
        // creator and the assigned name.
        constructor(bytes32 _name) public {
            // State variables are accessed via their name
            // and not via e.g. `this.owner`. Functions can
            // be accessed directly or through `this.f`,
            // but the latter provides an external view
            // to the function. Especially in the constructor,
            // you should not access functions externally,
            // because the function does not exist yet.
            // See the next section for details.
            owner = msg.sender;

            // We do an explicit type conversion from `address`
            // to `TokenCreator` and assume that the type of
            // the calling contract is `TokenCreator`, there is
            // no real way to check that.
            creator = TokenCreator(msg.sender);
            name = _name;
        }

        function changeName(bytes32 newName) public {
            // Only the creator can alter the name --
            // the comparison is possible since contracts
            // are explicitly convertible to addresses.
            if (msg.sender == address(creator))
                name = newName;
        }

        function transfer(address newOwner) public {
            // Only the current owner can transfer the token.
            if (msg.sender != owner) return;

            // We ask the creator contract if the transfer
            // should proceed by using a function of the
            // `TokenCreator` contract defined below. If
            // the call fails (e.g. due to out-of-gas),
            // the execution also fails here.
            if (creator.isTokenTransferOK(owner, newOwner))
                owner = newOwner;
        }
    }

    contract TokenCreator {
        function createToken(bytes32 name)
           public
           returns (OwnedToken tokenAddress)
        {
            // Create a new `Token` contract and return its address.
            // From the JavaScript side, the return type is
            // `address`, as this is the closest type available in
            // the ABI.
            return new OwnedToken(name);
        }

        function changeName(OwnedToken tokenAddress, bytes32 name) public {
            // Again, the external type of `tokenAddress` is
            // simply `address`.
            tokenAddress.changeName(name);
        }

        // Perform checks to determine if transferring a token to the
        // `OwnedToken` contract should proceed
        function isTokenTransferOK(address currentOwner, address newOwner)
            public
            pure
            returns (bool ok)
        {
            // Check an arbitrary condition to see if transfer should proceed
            return keccak256(abi.encodePacked(currentOwner, newOwner))[0] == 0x7f;
        }
    }
