#include "controllers/inputreplacement/InputReplacementController.hpp"

#include "Application.hpp"
#include "controllers/inputreplacement/InputReplacement.hpp"
#include "mocks/BaseApplication.hpp"
#include "Test.hpp"

using namespace chatterino;

namespace {

class MockApplication : public mock::BaseApplication
{
public:
    MockApplication()
        : inputReplacements(this->paths_)
    {
    }

    InputReplacementController *getInputReplacements() override
    {
        return &this->inputReplacements;
    }

    InputReplacementController inputReplacements;
};

}  // namespace

class TestInputReplacementController : public ::testing::Test
{
protected:
    void SetUp() override
    {
        this->mockApplication = std::make_unique<MockApplication>();
    }

    void TearDown() override
    {
        this->mockApplication.reset();
    }

    std::unique_ptr<MockApplication> mockApplication;
};

TEST_F(TestInputReplacementController, replaceCharactersInMesssage)
{
    struct TestCase {
        std::vector<InputReplacement> replacements;
        QString input;
        QString expectedMessage;
    };

    auto regularReplace = [](auto pattern, auto replace,
                             bool caseSensitive = true) {
        return InputReplacement(pattern, false, true, replace, caseSensitive);
    };
    auto regexReplace = [](auto pattern, auto regex,
                           bool caseSensitive = true) {
        return InputReplacement(pattern, true, true, regex, caseSensitive);
    };

    std::vector<TestCase> testCases{
        {
            .replacements = {regularReplace("foo1", "baz1")},
            .input = "foo1 Kappa",
            .expectedMessage = "baz1 Kappa",
        },
        {
            .replacements = {regularReplace("foo1", "baz1", false)},
            .input = "FoO1 Kappa",
            .expectedMessage = "baz1 Kappa",
        },
        {
            .replacements = {regexReplace("f(o+)1", "baz1[\\1]")},
            .input = "foo1 Kappa",
            .expectedMessage = "baz1[oo] Kappa",
        },

        {
            .replacements = {regexReplace("f(o+)1", R"(baz1[\0][\1][\2])")},
            .input = "foo1 Kappa",
            .expectedMessage = "baz1[\\0][oo][\\2] Kappa",
        },
        {
            .replacements = {regexReplace("f(o+)(\\d+)", "baz1[\\1+\\2]")},
            .input = "foo123 Kappa",
            .expectedMessage = "baz1[oo+123] Kappa",
        },
        {
            .replacements = {regexReplace("(?<=foo)(\\d+)", "[\\1]")},
            .input = "foo123 Kappa",
            .expectedMessage = "foo[123] Kappa",
        },
        {
            .replacements = {regexReplace("a(?=a| )", "b")},
            .input =
                "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                "aaaa"
                "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa "
                "Kappa",
            .expectedMessage =
                "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb"
                "bbbb"
                "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb "
                "Kappa",
        },
        {
            .replacements = {regexReplace("abc", "def", false)},
            .input = "AbC Kappa",
            .expectedMessage = "def Kappa",
        },
        {
            .replacements =
                {
                    regexReplace("abc", "def", false),
                    regularReplace("def", "ghi"),
                },
            .input = "AbC Kappa",
            .expectedMessage = "ghi Kappa",
        },
        {
            .replacements =
                {
                    regexReplace("a(?=a| )", "b"),
                    regexReplace("b(?=b| )", "c"),
                },
            .input =
                "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                "aaaa"
                "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa "
                "Kappa",
            .expectedMessage =
                "cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc"
                "cccc"
                "cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc "
                "Kappa",
        },
        {
            .replacements = {regexReplace("(f)o(o)", "\\1\\2")},
            .input = "foo",
            .expectedMessage = "fo",
        },
        {
            .replacements = {regexReplace("(f)o(o) (b)a(z)", R"(\1\2 \3\4)",
                                          false)},
            .input = "Foo Baz",
            .expectedMessage = "Fo Bz",
        },
        {.replacements = {regexReplace("\\b([a-z]+)(?:($)|( +))", "\\1_\\3"),
                          regexReplace("\\b([a-z]+)(<)", "\\1")},
         .input = "abc def gHi jkl< mno p",
         .expectedMessage = "abc_ def_ gHi jkl mno_ p_"},
    };

    for (const auto &test : testCases)
    {
        for (const auto &replacement : test.replacements)
        {
            getApp()->getInputReplacements()->items.append(replacement);
        }

        auto message = test.input;

        getApp()->getInputReplacements()->replaceCharactersInMessage(message);

        for (int i = static_cast<int>(test.replacements.size() - 1); i >= 0;
             --i)
        {
            getApp()->getInputReplacements()->items.removeAt(i);
        }

        EXPECT_EQ(message, test.expectedMessage)
            << "Message not equal for input '" << test.input
            << "' - expected: '" << test.expectedMessage << "' got: '"
            << message << "'";
    }
}
