var assert = require('assert');
var formatters = require('../lib/web3/formatters.js');
var BigNumber = require('bignumber.js');

describe('formatters', function () {
    describe('outputBlockFormatter', function () {
        it('should return the correct value', function () {
            
            assert.deepEqual(formatters.outputBlockFormatter({
                hash: '0x34234kjh23kj4234',
                parentHash: '0x34234kjh23kj4234',
                miner: '0x34234kjh23kj4234',
                stateRoot: '0x34234kjh23kj4234',
                sha3Uncles: '0x34234kjh23kj4234',
                bloom: '0x34234kjh23kj4234',
                difficulty: '0x3e8',
                totalDifficulty: '0x3e8',
                number: '0x3e8',
                minGasPrice: '0x3e8',
                gasLimit: '0x3e8',
                gasUsed: '0x3e8',
                timestamp: '0x3e8',
                extraData: '0x34234kjh23kj4234',
                nonce: '0x34234kjh23kj4234',
                children: ['0x34234kjh23kj4234'],
                size: '0x3e8'
            }), {
                hash: '0x34234kjh23kj4234',
                parentHash: '0x34234kjh23kj4234',
                miner: '0x34234kjh23kj4234',
                stateRoot: '0x34234kjh23kj4234',
                sha3Uncles: '0x34234kjh23kj4234',
                bloom: '0x34234kjh23kj4234',
                difficulty: new BigNumber(1000),
                totalDifficulty: new BigNumber(1000),
                number: 1000,
                minGasPrice: new BigNumber(1000),
                gasLimit: 1000,
                gasUsed: 1000,
                timestamp: 1000,
                extraData: '0x34234kjh23kj4234',
                nonce: '0x34234kjh23kj4234',
                children: ['0x34234kjh23kj4234'],
                size: 1000
            });
        });
    });
});
