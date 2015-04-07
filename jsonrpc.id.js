var chai = require('chai');
var assert = chai.assert;
var Jsonrpc = require('../lib/web3/jsonrpc');

describe('lib/web3/jsonrpc', function () {
    describe('id', function () {
        it('should increment the id', function () {
            
            // given
            var a = Jsonrpc.getInstance();
            var b = Jsonrpc.getInstance();
            var method = 'm';

            // when
            var p1 = a.toPayload(method);
            var p2 = b.toPayload(method);

            // then
            assert.equal(p2.id, p1.id + 1); 
        });
    });
});

