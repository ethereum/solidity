var assert = require('assert');
var formatters = require('../lib/web3/formatters.js');

describe('formatters', function () {
    describe('outputLogFormatter', function () {
        it('should return the correct value', function () {
            
            assert.deepEqual(formatters.outputLogFormatter({
                transactionIndex: '0x3e8',
                logIndex: '0x3e8',
                blockNumber: '0x3e8',
                transactionHash: '0xd6960376d6c6dea93647383ffb245cfced97ccc5c7525397a543a72fdaea5265',
                blockHash: '0xd6960376d6c6dea93647383ffb245cfced97ccc5c7525397a543a72fdaea5265',
                data: '0x7b2274657374223a2274657374227',
                topics: ['0x68656c6c6f','0x6d79746f70696373']                
            }), {
                transactionIndex: 1000,
                logIndex: 1000,
                blockNumber: 1000,
                transactionHash: '0xd6960376d6c6dea93647383ffb245cfced97ccc5c7525397a543a72fdaea5265',
                blockHash: '0xd6960376d6c6dea93647383ffb245cfced97ccc5c7525397a543a72fdaea5265',
                data: '0x7b2274657374223a2274657374227',
                topics: ['0x68656c6c6f','0x6d79746f70696373']
            });
        });
        it('should return the correct value, when null values are present', function () {
            
            assert.deepEqual(formatters.outputLogFormatter({
                transactionIndex: null,
                logIndex: null,
                blockNumber: null,
                transactionHash: null,
                blockHash: null,
                data: '0x7b2274657374223a2274657374227',
                topics: ['0x68656c6c6f','0x6d79746f70696373']                
            }), {
                transactionIndex: null,
                logIndex: null,
                blockNumber: null,
                transactionHash: null,
                blockHash: null,
                data: '0x7b2274657374223a2274657374227',
                topics: ['0x68656c6c6f','0x6d79746f70696373']
            });
        });
    });
});
