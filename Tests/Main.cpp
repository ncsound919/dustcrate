#include <JuceHeader.h>

// Pull in all test compilation units.
#include "PresetValidatorTest.cpp"
#include "SampleLibraryTest.cpp"

//==============================================================================
int main()
{
    juce::UnitTestRunner runner;
    runner.setAssertOnFailure(false);
    runner.runAllTests();

    int failures = 0;
    for (int i = 0; i < runner.getNumResults(); ++i)
        if (auto* r = runner.getResult(i))
            failures += r->failures;

    if (failures > 0)
        std::cerr << failures << " test(s) failed.\n";
    else
        std::cout << "All tests passed.\n";

    return failures > 0 ? 1 : 0;
}
