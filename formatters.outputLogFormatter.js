var assert = require('assert');
var formatters = require('../lib/web3/formatters.js');

describe('formatters', function () {
    describe('outputLogFormatter', function () {
        it('should return the correct value', function () {
            
            assert.deepEqual(formatters.outputLogFormatter({
                transactionIndex: '0x3e8',
                logIndex: '0x3e8',
                blockNumber: '0x3e8',
                transactionHash: '0x7b2274657374223a2274657374227d',
                blockHash: '0x7b2274657374223a2274657374227d',
                data: '0x7b2274657374223a2274657374227d',
                topics: ['0x68656c6c6f','0x6d79746f70696373']                
            }), {
                transactionIndex: 1000,
                logIndex: 1000,
                blockNumber: 1000,
                transactionHash: '0x7b2274657374223a2274657374227d',
                blockHash: '0x7b2274657374223a2274657374227d',
                data: '0x7b2274657374223a2274657374227d',
                topics: ['0x68656c6c6f','0x6d79746f70696373']
            });
        });
    });
});
