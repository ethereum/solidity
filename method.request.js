var chai = require('chai');
var assert = chai.assert;
var web3 = require('../index');

describe('lib/web3/method', function () {
    describe('request', function () {
        it('should create proper request', function () {
            
            var callback = function (err, result) {};
            var expected = {
                method: 'eth_getBalance',
                callback: callback,
                params: ['0x0000000000000000000000000000000000000000', 'latest'],
            };

            var request = web3.eth.getBalance.request('0x0000000000000000000000000000000000000000', 'latest', callback);

            expected.format = request.format;
            assert.deepEqual(request, expected);
        });
    });
});

