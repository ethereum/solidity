var assert = require('assert');
var event = require('../lib/web3/event.js');
var f = require('../lib/solidity/formatters.js');

describe('lib/web3/event', function () {
    describe('inputParser', function () {
        it('should create basic filter input object', function () {
            
            // given
            var address = '0x012345'; 
            var signature = '0x987654';
            var e = {
                name: 'Event',
                inputs: [{"name":"a","type":"uint256","indexed":true},{"name":"b","type":"hash256","indexed":false}]
            };

            // when
            var impl = event.inputParser(address, signature, e);
            var result = impl();

            // then
            assert.equal(result.address, address); 
            assert.equal(result.topics.length, 1);
            assert.equal(result.topics[0], signature);

        });

        it('should create filter input object with options', function () {
            
            // given
            var address = '0x012345';
            var signature = '0x987654';
            var options = {
                fromBlock: 1,
                toBlock: 2,
            };
            var e = {
                name: 'Event',
                inputs: [{"name":"a","type":"uint256","indexed":true},{"name":"b","type":"hash256","indexed":false}]
            };

            // when
            var impl = event.inputParser(address, signature, e); 
            var result = impl({}, options);

            // then
            assert.equal(result.address, address);
            assert.equal(result.topics.length, 1);
            assert.equal(result.topics[0], signature);
            assert.equal(result.fromBlock, options.fromBlock);
            assert.equal(result.toBlock, options.toBlock);
        
        });

        it('should create filter input object with indexed params', function () {
        
            // given
            var address = '0x012345';
            var signature = '0x987654';
            var options = {
                fromBlock: 1,
                toBlock: 2
            };
            var e = {
                name: 'Event',
                inputs: [{"name":"a","type":"uint256","indexed":true},{"name":"b","type":"hash256","indexed":false}]
            };

            // when
            var impl = event.inputParser(address, signature, e); 
            var result = impl({a: 4}, options);

            // then
            assert.equal(result.address, address);
            assert.equal(result.topics.length, 2);
            assert.equal(result.topics[0], signature);
            assert.equal(result.topics[1], '0x' + f.formatInputInt(4));
            assert.equal(result.fromBlock, options.fromBlock);
            assert.equal(result.toBlock, options.toBlock);

        });

        it('should create filter input object with an array of indexed params', function () {
        
            // given
            var address = '0x012345';
            var signature = '0x987654';
            var options = {
                fromBlock: 1,
                toBlock: 2,
            };
            var e = {
                name: 'Event',
                inputs: [{"name":"a","type":"uint256","indexed":true},{"name":"b","type":"hash256","indexed":false}]
            };

            // when
            var impl = event.inputParser(address, signature, e); 
            var result = impl({a: [4, 69]}, options);

            // then
            assert.equal(result.address, address);
            assert.equal(result.topics.length, 2);
            assert.equal(result.topics[0], signature);
            assert.equal(result.topics[1][0], f.formatInputInt(4));
            assert.equal(result.topics[1][1], f.formatInputInt(69));
            assert.equal(result.fromBlock, options.fromBlock);
            assert.equal(result.toBlock, options.toBlock);

        });
    });
});

