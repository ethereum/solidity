var assert = require('assert');
var formatters = require('../lib/web3/formatters.js');
var BigNumber = require('bignumber.js');

describe('formatters', function () {
    describe('outputTransactionFormatter', function () {
        it('should return the correct value', function () {
            
            assert.deepEqual(formatters.outputTransactionFormatter({
                input: '0x3454645634534',
                from: '0x00000',
                to: '0x00000',
                value: '0x3e8',
                gas: '0x3e8',
                gasPrice: '0x3e8',
                nonce: '0xb',
                transactionIndex: '0x1',
                blockNumber: '0x3e8',
                blockHash: '0xc9b9cdc2092a9d6589d96662b1fd6949611163fb3910cf8a173cd060f17702f9'
            }), {
                input: '0x3454645634534',
                from: '0x00000',
                to: '0x00000',
                value: new BigNumber(1000),
                gas: 1000,
                gasPrice: new BigNumber(1000),
                nonce: 11,
                blockNumber: 1000,
                blockHash: '0xc9b9cdc2092a9d6589d96662b1fd6949611163fb3910cf8a173cd060f17702f9',
                transactionIndex: 1
            });
        });

        it('should return the correct value, when null values are present', function () {
            
            assert.deepEqual(formatters.outputTransactionFormatter({
                input: '0x3454645634534',
                from: '0x00000',
                to: null,
                value: '0x3e8',
                gas: '0x3e8',
                gasPrice: '0x3e8',
                nonce: '0xb',
                transactionIndex: null,
                blockNumber: null,
                blockHash: null
            }), {
                input: '0x3454645634534',
                from: '0x00000',
                to: null,
                value: new BigNumber(1000),
                gas: 1000,
                gasPrice: new BigNumber(1000),
                nonce: 11,
                blockNumber: null,
                blockHash: null,
                transactionIndex: null
            });
        });
    });
});
