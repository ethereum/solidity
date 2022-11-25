// SPDX-License-Identifier: MIT
pragma solidity ^0.8.0;

// This is an adapted version of StdCheats by foundry:
// https://github.com/foundry-rs/forge-std/blob/master/src/StdCheats.sol
contract StdCheatsSafe {
    struct Chain {
        string name;
        uint256 chainId;
        string rpcUrl;
    }

    struct Chains {
        Chain Anvil;
        Chain Hardhat;
        Chain Mainnet;
        Chain Goerli;
        Chain Sepolia;
        Chain Optimism;
        Chain OptimismGoerli;
        Chain ArbitrumOne;
        Chain ArbitrumOneGoerli;
        Chain ArbitrumNova;
        Chain Polygon;
        Chain PolygonMumbai;
        Chain Avalanche;
        Chain AvalancheFuji;
        Chain BnbSmartChain;
        Chain BnbSmartChainTestnet;
        Chain GnosisChain;
    }

    Chains stdChains;

    function f(string[2][] memory rpcs) public returns (uint256) {
        stdChains = Chains({
            Anvil: Chain("Anvil", 31337, "http://127.0.0.1:8545"),
            Hardhat: Chain("Hardhat", 31337, "http://127.0.0.1:8545"),
            Mainnet: Chain("Mainnet", 1, "https://api.mycryptoapi.com/eth"),
            Goerli: Chain("Goerli", 5, "https://goerli.infura.io/v3/84842078b09946638c03157f83405213"),
            Sepolia: Chain("Sepolia", 11155111, "https://rpc.sepolia.dev"),
            Optimism: Chain("Optimism", 10, "https://mainnet.optimism.io"),
            OptimismGoerli: Chain("OptimismGoerli", 420, "https://goerli.optimism.io"),
            ArbitrumOne: Chain("ArbitrumOne", 42161, "https://arb1.arbitrum.io/rpc"),
            ArbitrumOneGoerli: Chain("ArbitrumOneGoerli", 421613, "https://goerli-rollup.arbitrum.io/rpc"),
            ArbitrumNova: Chain("ArbitrumNova", 42170, "https://nova.arbitrum.io/rpc"),
            Polygon: Chain("Polygon", 137, "https://polygon-rpc.com"),
            PolygonMumbai: Chain("PolygonMumbai", 80001, "https://rpc-mumbai.matic.today"),
            Avalanche: Chain("Avalanche", 43114, "https://api.avax.network/ext/bc/C/rpc"),
            AvalancheFuji: Chain("AvalancheFuji", 43113, "https://api.avax-test.network/ext/bc/C/rpc"),
            BnbSmartChain: Chain("BnbSmartChain", 56, "https://bsc-dataseed1.binance.org"),
            BnbSmartChainTestnet: Chain("BnbSmartChainTestnet", 97, "https://data-seed-prebsc-1-s1.binance.org:8545"),
            GnosisChain: Chain("GnosisChain", 100, "https://rpc.gnosischain.com")
        });

        for (uint256 i = 0; i < rpcs.length; i++) {
            (string memory name, string memory rpcUrl) = (rpcs[i][0], rpcs[i][1]);
            // forgefmt: disable-start
            if (isEqual(name, "anvil")) stdChains.Anvil.rpcUrl = rpcUrl;
            else if (isEqual(name, "hardhat")) stdChains.Hardhat.rpcUrl = rpcUrl;
            else if (isEqual(name, "mainnet")) stdChains.Mainnet.rpcUrl = rpcUrl;
            else if (isEqual(name, "goerli")) stdChains.Goerli.rpcUrl = rpcUrl;
            else if (isEqual(name, "sepolia")) stdChains.Sepolia.rpcUrl = rpcUrl;
            else if (isEqual(name, "optimism")) stdChains.Optimism.rpcUrl = rpcUrl;
            else if (isEqual(name, "optimism_goerli", "optimism-goerli")) stdChains.OptimismGoerli.rpcUrl = rpcUrl;
            else if (isEqual(name, "arbitrum_one", "arbitrum-one")) stdChains.ArbitrumOne.rpcUrl = rpcUrl;
            else if (isEqual(name, "arbitrum_one_goerli", "arbitrum-one-goerli")) stdChains.ArbitrumOneGoerli.rpcUrl = rpcUrl;
            else if (isEqual(name, "arbitrum_nova", "arbitrum-nova")) stdChains.ArbitrumNova.rpcUrl = rpcUrl;
            else if (isEqual(name, "polygon")) stdChains.Polygon.rpcUrl = rpcUrl;
            else if (isEqual(name, "polygon_mumbai", "polygon-mumbai")) stdChains.PolygonMumbai.rpcUrl = rpcUrl;
            else if (isEqual(name, "avalanche")) stdChains.Avalanche.rpcUrl = rpcUrl;
            else if (isEqual(name, "avalanche_fuji", "avalanche-fuji")) stdChains.AvalancheFuji.rpcUrl = rpcUrl;
            else if (isEqual(name, "bnb_smart_chain", "bnb-smart-chain")) stdChains.BnbSmartChain.rpcUrl = rpcUrl;
            else if (isEqual(name, "bnb_smart_chain_testnet", "bnb-smart-chain-testnet")) stdChains.BnbSmartChainTestnet.rpcUrl = rpcUrl;
            else if (isEqual(name, "gnosis_chain", "gnosis-chain")) stdChains.GnosisChain.rpcUrl = rpcUrl;
            // forgefmt: disable-end
        }
        return 0;
    }

    function isEqual(string memory a, string memory b) private pure returns (bool) {
        return keccak256(abi.encode(a)) == keccak256(abi.encode(b));
    }

    function isEqual(string memory a, string memory b, string memory c) private pure returns (bool) {
        return keccak256(abi.encode(a)) == keccak256(abi.encode(b))
            || keccak256(abi.encode(a)) == keccak256(abi.encode(c));
    }
}


