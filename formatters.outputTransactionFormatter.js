var assert = require('assert');
var formatters = require('../lib/web3/formatters.js');
var BigNumber = require('bignumber.js');

describe('formatters', function () {
    describe('outputTransactionFormatter', function () {
        it('should return the correct value', function () {
            
            assert.deepEqual(formatters.outputTransactionFormatter({
                input: '0x34234kjh23kj4234',
                from: '0x00000',
                to: '0x00000',
                value: '0x3e8',
                gas: '0x3e8',
                gasPrice: '0x3e8',
                nonce: '0xb',
                transactionIndex: '0x1',
                blockNumber: '0x3e8',
                blockHash: '0x34234bf23bf4234'
            }), {
                input: '0x34234kjh23kj4234',
                from: '0x00000',
                to: '0x00000',
                value: new BigNumber(1000),
                gas: 1000,
                gasPrice: new BigNumber(1000),
                nonce: 11,
                blockNumber: 1000,
                blockHash: '0x34234bf23bf4234',
                transactionIndex: 1
            });
        });
    });
});
