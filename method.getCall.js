var chai = require('chai');
var assert = chai.assert;
var Method = require('../lib/web3/method');

describe('lib/web3/method', function () {
    describe('getCall', function () {
        it('should return call name', function () {
            
            // given
            var call = 'hello_call_world';
            var method = new Method({
                call: call
            });

            // when
            var result = method.getCall();

            // then
            assert.equal(call, result);
        });

        it('should return call based on args', function () {
            
            // given
            var call = function (args) {
                return args ? args.length.toString() : '0';
            };
            
            var method = new Method({
                call: call
            });
            
            // when
            var r0 = method.getCall();
            var r1 = method.getCall([1]);
            var r2 = method.getCall([1, 2]);

            // then
            assert.equal(r0, '0');
            assert.equal(r1, '1');
            assert.equal(r2, '2');
            
        });
    });
});

