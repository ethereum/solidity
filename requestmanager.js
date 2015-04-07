var chai = require('chai');
var assert = chai.assert;
var RequestManager = require('../lib/web3/requestmanager');
var FakeHttpProvider = require('./helpers/FakeHttpProvider');

// TODO: handling errors!
// TODO: validation of params!

describe('lib/web3/requestmanager', function () {
    describe('send', function () {
        it('should return expected result synchronously', function () {
            var provider = new FakeHttpProvider();
            var manager = RequestManager.getInstance();
            manager.setProvider(provider);
            var expected = 'hello_world';
            provider.injectResult(expected);
            
            var result = manager.send({
                method: 'test',
                params: [1,2,3]
            });

            assert.equal(expected, result);
        });

        it('should return expected result asynchronously', function (done) {
            var provider = new FakeHttpProvider();
            var manager = RequestManager.getInstance();
            manager.setProvider(provider);
            var expected = 'hello_world';
            provider.injectResult(expected);
            
            manager.sendAsync({
                method: 'test',
                params: [1,2,3]
            }, function (error, result) {
                assert.equal(error, null);
                assert.equal(expected, result);
                done();
            });
        });
    });
});

