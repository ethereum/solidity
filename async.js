var chai = require('chai');
var assert = chai.assert;
var web3 = require('../index');
var FakeHttpProvider = require('./helpers/FakeHttpProvider');

// use sendTransaction as dummy
var method = 'sendTransaction';

var tests = [{
    result: '0xb',
    formattedResult: '0xb',
    call: 'eth_'+ method
}];

describe('async', function () {
    tests.forEach(function (test, index) {
        it('test: ' + index, function (done) {
            
            // given
            var provider = new FakeHttpProvider();
            web3.setProvider(provider);
            provider.injectResult(test.result);
            provider.injectValidation(function (payload) {
                assert.equal(payload.jsonrpc, '2.0');
                assert.equal(payload.method, test.call);
                assert.deepEqual(payload.params, [{}]);
            });

            // when 
            web3.eth[method]({}, function(error, result){

                // then
                assert.isNull(error);
                assert.strictEqual(test.formattedResult, result);
                
                done();
            });
            
        });

        it('error test: ' + index, function (done) {
            
            // given
            var provider = new FakeHttpProvider();
            web3.setProvider(provider);
            provider.injectError({
                    message: test.result,
                    code: -32603
            });
            provider.injectValidation(function (payload) {
                assert.equal(payload.jsonrpc, '2.0');
                assert.equal(payload.method, test.call);
                assert.deepEqual(payload.params, [{}]);
            });

            // when 
            web3.eth[method]({}, function(error, result){

                // then
                assert.isUndefined(result);
                assert.strictEqual(test.formattedResult, error.message);

                done();
            });
            
        });
    });
});

