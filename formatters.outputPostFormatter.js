var assert = require('assert');
var formatters = require('../lib/web3/formatters.js');

describe('formatters', function () {
    describe('outputPostFormatter', function () {
        it('should return the correct value', function () {
            
            assert.deepEqual(formatters.outputPostFormatter({
                expiry: '0x3e8',
                sent: '0x3e8',
                ttl: '0x3e8',
                workProved: '0x3e8',
                payload: '0x7b2274657374223a2274657374227d',
                topics: ['0x68656c6c6f','0x6d79746f70696373']                
            }), {
                expiry: 1000,
                sent: 1000,
                ttl: 1000,
                workProved: 1000,
                payload: {test: 'test'},
                payloadRaw: '0x7b2274657374223a2274657374227d',
                topics: ['hello','mytopics']
            });
        });
    });
});
