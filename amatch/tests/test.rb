require 'test/unit'
require 'amatch'

class TC_AmatchTest < Test::Unit::TestCase

    def setup
        @matcher = Amatch.new('test')
    end

    def test_match
        assert(@matcher.match('') == 4)
        assert(@matcher.match('test') == 0)
        assert(@matcher.match('test') == 0)
        assert(@matcher.match('testa') == 1)
        assert(@matcher.match('atest') == 1)
        assert(@matcher.match('teast') == 1)
        assert(@matcher.match('est') == 1)
        assert(@matcher.match('tes') == 1)
        assert(@matcher.match('tst') == 1)
        assert(@matcher.match('best') == 1)
        assert(@matcher.match('tost') == 1)
        assert(@matcher.match('tesa') == 1)
        assert(@matcher.match('taex') == 3)
        assert(@matcher.matchr('') == 1.0)
        assert(@matcher.matchr('tesa') == 0.25)
        assert(@matcher.match('aaatestbbb') == 6)
    end

    def test_search
        assert(@matcher.search('') == 4)
        assert(@matcher.searchr('') == 1.0)
        assert(@matcher.search('aaatestbbb') == 0)
        assert(@matcher.search('aaataexbbb') == 3)
        assert(@matcher.searchr('aaataexbbb') == 0.75)
        assert(@matcher.search('aaaaaaaaa') == 4)
        assert(@matcher.searchr('aaaaaaaaa') == 1.0)
    end

    def test_compare
        assert(@matcher.compare('') == -4)
        assert(@matcher.comparer('') == -1.0)
        assert(@matcher.compare('taex') == 3)
        assert(@matcher.comparer('tesa') == 0.25)
        assert(@matcher.compare('aaatestbbb') == 6)
        assert(@matcher.compare('test') == 0)
        assert(@matcher.compare('tex') == -2)
        assert(@matcher.comparer('tsa') == -0.5)
        assert(@matcher.compare('wxyz') == 4)
        assert(@matcher.comparer('wxyz') == 1.0)
        assert_raises(TypeError) { @matcher.match(:foo) }
    end

    def test_array_result
        assert(@matcher.match([]) == []);
        assert(@matcher.match(["tets", "test"]) == [2, 0]);
        assert(@matcher.matchr(["tets", "test"]) == [0.5, 0]);
        assert(@matcher.compare(["tets", "test"]) == [2, 0]);
        assert(@matcher.comparer(["tets", "test"]) == [0.5, 0]);
        assert(@matcher.search(["tetsaaa", "testaaa"]) == [1, 0]);
        assert(@matcher.searchr(["tetsaaa", "testaaa"]) == [0.25, 0]);
        assert_raises(TypeError) { @matcher.match([:foo, "bar"]) }
    end

    def test_weights
        assert(@matcher.subw == 1)
        assert(@matcher.insw == 1)
        assert(@matcher.delw == 1)
        @matcher.subw = 2
        assert(@matcher.subw == 2)
        assert(@matcher.match('tast') == 2)
        @matcher.subw = 1
        assert(@matcher.match('tast') == 1)
        @matcher.insw = 2
        assert(@matcher.insw == 2)
        assert(@matcher.match('teist') == 2)
        @matcher.delw = 2
        assert(@matcher.delw == 2)
        assert(@matcher.match('tst') == 2)
        @matcher.resetw    
        assert(@matcher.subw == 1)
        assert(@matcher.insw == 1)
        assert(@matcher.delw == 1)
        @matcher.subw = :something
        assert_raises(TypeError) { @matcher.match('anything') }
        @matcher.subw = 1
        @matcher.insw = :something
        assert_raises(TypeError) { @matcher.match('anything') }
        @matcher.insw = 1
        @matcher.delw = :something
        assert_raises(TypeError) { @matcher.match('anything') }
    end

end
    # vim: set noet sw=4 ts=4:
