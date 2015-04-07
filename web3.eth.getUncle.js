var chai = require('chai');
var web3 = require('../index');
var BigNumber = require('bignumber.js');
var testMethod = require('./helpers/test.method.js');

var method = 'getUncle';

var blockResult = {
    "number": "0x1b4",
    "hash": "0xe670ec64341771606e55d6b4ca35a1a6b75ee3d5145a99d05921026d1527331",
    "parentHash": "0x9646252be9520f6e71339a8df9c55e4d7619deeb018d2a3f2d21fc165dde5eb5",
    "nonce": "0xe04d296d2460cfb8472af2c5fd05b5a214109c25688d3704aed5484f9a7792f2",
    "sha3Uncles": "0x1dcc4de8dec75d7aab85b567b6ccd41ad312451b948a7413f0a142fd40d49347",
    "logsBloom": "0xe670ec64341771606e55d6b4ca35a1a6b75ee3d5145a99d05921026d1527331",
    "transactionsRoot": "0x56e81f171bcc55a6ff8345e692c0f86e5b48e01b996cadc001622fb5e363b421",
    "stateRoot": "0xd5855eb08b3387c0af375e9cdb6acfc05eb8f519e419b874b6ff2ffda7ed1dff",
    "miner": "0x4e65fda2159562a496f9f3522f89122a3088497a",
    "difficulty": "0x027f07",
    "totalDifficulty":  "0x027f07",
    "size":  "0x027f07",
    "extraData": "0x0000000000000000000000000000000000000000000000000000000000000000",
    "gasLimit": "0x9f759",
    "minGasPrice": "0x9f759",
    "gasUsed": "0x9f759",
    "timestamp": "0x54e34e8e",
    "transactions": ['0x460cfb8472af2c5fd05b5a2','0x460cfb8472af2c5fd05b5a2'],
    "uncles": ["0x460cfb8472af2c5fd05b5a2", "0xd5460cfb8472af2c5fd05b5a2"]
};
var formattedBlockResult = {
    "number": 436,
    "hash": "0xe670ec64341771606e55d6b4ca35a1a6b75ee3d5145a99d05921026d1527331",
    "parentHash": "0x9646252be9520f6e71339a8df9c55e4d7619deeb018d2a3f2d21fc165dde5eb5",
    "nonce": "0xe04d296d2460cfb8472af2c5fd05b5a214109c25688d3704aed5484f9a7792f2",
    "sha3Uncles": "0x1dcc4de8dec75d7aab85b567b6ccd41ad312451b948a7413f0a142fd40d49347",
    "logsBloom": "0xe670ec64341771606e55d6b4ca35a1a6b75ee3d5145a99d05921026d1527331",
    "transactionsRoot": "0x56e81f171bcc55a6ff8345e692c0f86e5b48e01b996cadc001622fb5e363b421",
    "stateRoot": "0xd5855eb08b3387c0af375e9cdb6acfc05eb8f519e419b874b6ff2ffda7ed1dff",
    "miner": "0x4e65fda2159562a496f9f3522f89122a3088497a",
    "difficulty": new BigNumber(163591),
    "totalDifficulty":  new BigNumber(163591),
    "size":  163591,
    "extraData": "0x0000000000000000000000000000000000000000000000000000000000000000",
    "gasLimit": 653145,
    "minGasPrice": new BigNumber(653145),
    "gasUsed": 653145,
    "timestamp": 1424182926,
    "transactions": ['0x460cfb8472af2c5fd05b5a2','0x460cfb8472af2c5fd05b5a2'],
    "uncles": ["0x460cfb8472af2c5fd05b5a2", "0xd5460cfb8472af2c5fd05b5a2"]
};
var blockResultWithTx = {
    "number": "0x1b4",
    "hash": "0xe670ec64341771606e55d6b4ca35a1a6b75ee3d5145a99d05921026d1527331",
    "parentHash": "0x9646252be9520f6e71339a8df9c55e4d7619deeb018d2a3f2d21fc165dde5eb5",
    "nonce": "0xe04d296d2460cfb8472af2c5fd05b5a214109c25688d3704aed5484f9a7792f2",
    "sha3Uncles": "0x1dcc4de8dec75d7aab85b567b6ccd41ad312451b948a7413f0a142fd40d49347",
    "logsBloom": "0xe670ec64341771606e55d6b4ca35a1a6b75ee3d5145a99d05921026d1527331",
    "transactionsRoot": "0x56e81f171bcc55a6ff8345e692c0f86e5b48e01b996cadc001622fb5e363b421",
    "stateRoot": "0xd5855eb08b3387c0af375e9cdb6acfc05eb8f519e419b874b6ff2ffda7ed1dff",
    "miner": "0x4e65fda2159562a496f9f3522f89122a3088497a",
    "difficulty": "0x027f07",
    "totalDifficulty":  "0x027f07",
    "size":  "0x027f07",
    "extraData": "0x0000000000000000000000000000000000000000000000000000000000000000",
    "gasLimit": "0x9f759",
    "minGasPrice": "0x9f759",
    "gasUsed": "0x9f759",
    "timestamp": "0x54e34e8e",
    "transactions": [{
        "status": "mined",
        "hash":"0xc6ef2fc5426d6ad6fd9e2a26abeab0aa2411b7ab17f30a99d3cb96aed1d1055b",
        "nonce":"0x",
        "blockHash": "0x6fd9e2a26ab",
        "blockNumber": "0x15df",
        "transactionIndex":  "0x1",
        "from":"0x407d73d8a49eeb85d32cf465507dd71d507100c1",
        "to":"0x85h43d8a49eeb85d32cf465507dd71d507100c1",
        "value":"0x7f110",
        "gas": "0x7f110",
        "gasPrice":"0x09184e72a000",
        "input":"0x603880600c6000396000f30060",
    }],
    "uncles": ["0x460cfb8472af2c5fd05b5a2", "0xd5460cfb8472af2c5fd05b5a2"]
};
var formattedBlockResultWithTx = {
    "number": 436,
    "hash": "0xe670ec64341771606e55d6b4ca35a1a6b75ee3d5145a99d05921026d1527331",
    "parentHash": "0x9646252be9520f6e71339a8df9c55e4d7619deeb018d2a3f2d21fc165dde5eb5",
    "nonce": "0xe04d296d2460cfb8472af2c5fd05b5a214109c25688d3704aed5484f9a7792f2",
    "sha3Uncles": "0x1dcc4de8dec75d7aab85b567b6ccd41ad312451b948a7413f0a142fd40d49347",
    "logsBloom": "0xe670ec64341771606e55d6b4ca35a1a6b75ee3d5145a99d05921026d1527331",
    "transactionsRoot": "0x56e81f171bcc55a6ff8345e692c0f86e5b48e01b996cadc001622fb5e363b421",
    "stateRoot": "0xd5855eb08b3387c0af375e9cdb6acfc05eb8f519e419b874b6ff2ffda7ed1dff",
    "miner": "0x4e65fda2159562a496f9f3522f89122a3088497a",
    "difficulty": new BigNumber(163591),
    "totalDifficulty":  new BigNumber(163591),
    "size":  163591,
    "extraData": "0x0000000000000000000000000000000000000000000000000000000000000000",
    "gasLimit": 653145,
    "minGasPrice": new BigNumber(653145),
    "gasUsed": 653145,
    "timestamp": 1424182926,
    "transactions": [{
        "status": "mined",
        "hash":"0xc6ef2fc5426d6ad6fd9e2a26abeab0aa2411b7ab17f30a99d3cb96aed1d1055b",
        "nonce":"0x",
        "blockHash": "0x6fd9e2a26ab",
        "blockNumber": 5599,
        "transactionIndex":  1,
        "from":"0x407d73d8a49eeb85d32cf465507dd71d507100c1",
        "to":"0x85h43d8a49eeb85d32cf465507dd71d507100c1",
        "value": new BigNumber(520464),
        "gas": 520464,
        "gasPrice": new BigNumber(10000000000000),
        "input":"0x603880600c6000396000f30060",
    }],
    "uncles": ["0x460cfb8472af2c5fd05b5a2", "0xd5460cfb8472af2c5fd05b5a2"]
};

var tests = [{
    args: ['0x47d33b27bb249a2dbab4c0612bf9caf4c1950855', 2],
    formattedArgs: ['0x47d33b27bb249a2dbab4c0612bf9caf4c1950855', '0x2', false],
    result: blockResult,
    formattedResult: formattedBlockResult,
    call: 'eth_getUncleByBlockHashAndIndex'
},{
    args: [436, 1],
    formattedArgs: ['0x1b4', '0x1', false],
    result: blockResult,
    formattedResult: formattedBlockResult,
    call: 'eth_getUncleByBlockNumberAndIndex'
},{
    args: [436, 1, true],
    formattedArgs: ['0x1b4', '0x1', true],
    result: blockResultWithTx,
    formattedResult: formattedBlockResultWithTx,
    call: 'eth_getUncleByBlockNumberAndIndex'
}];

testMethod.runTests('eth', method, tests);
