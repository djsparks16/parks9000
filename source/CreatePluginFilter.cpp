#include "PluginProcessor.h"

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new FiveParksVST3AudioProcessor();
}
