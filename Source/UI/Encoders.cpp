/*
  ==============================================================================

    Encoders.cpp
    Created: 19 Feb 2022 12:19:42am
    Author:  No

  ==============================================================================
*/

#include <JuceHeader.h>
#include "Encoders.h"
#include "UserInputManager.h"
#include "Definitions/ChannelFamily/ChannelType/ChannelType.h"
#include "Definitions/Programmer/Programmer.h"
#include "Definitions/Command/Command.h"
#include "Definitions/ChannelFamily/ChannelFamilyManager.h"
#include "UI/BKColorPicker.h"

//==============================================================================
EncodersUI::EncodersUI(const String& contentName):
    ShapeShifterContent(Encoders::getInstance(), contentName)
{
}

EncodersUI::~EncodersUI()
{
}

juce_ImplementSingleton(Encoders);

Encoders::Encoders():
    channels(),
    ControllableContainer("Encoders")
{
    addAndMakeVisible(&commandLine);

    paramNumbersOrNames = addTrigger("123", ""); paramNumbersOrNames->setCustomShortName("verbose"); btnNumbersOrNames = paramNumbersOrNames->createButtonUI(); addAndMakeVisible(btnNumbersOrNames);
    btnNumbersOrNames->customBGColor = Colour(59, 59, 59); btnNumbersOrNames->useCustomBGColor = true;
    paramHighLight = addTrigger("HL", ""); paramHighLight->setCustomShortName("hl"); btnHighLight = paramHighLight->createButtonUI(); addAndMakeVisible(btnHighLight);
    btnHighLight->customBGColor = Colour(59, 59, 59); btnHighLight->useCustomBGColor = true;
    paramBlind = addTrigger("Blind", ""); paramBlind->setCustomShortName("blind"); btnBlind = paramBlind->createButtonUI(); addAndMakeVisible(btnBlind);
    btnBlind->customBGColor = Colour(59, 59, 59); btnBlind->useCustomBGColor = true;
    paramMode = addTrigger("Val", ""); paramMode->setCustomShortName("valThru"); btnMode = paramMode->createButtonUI(); addAndMakeVisible(btnMode);
    btnMode->customBGColor = Colour(59, 59, 59); btnMode->useCustomBGColor = true;
    paramBigMoveLeft = addTrigger("<<", ""); paramBigMoveLeft->setCustomShortName("moveLeftBig"); btnBigMoveLeft = paramBigMoveLeft->createButtonUI(); addAndMakeVisible(btnBigMoveLeft);
    btnBigMoveLeft->customBGColor = Colour(59, 59, 59); btnBigMoveLeft->useCustomBGColor = true;
    paramLittleMoveLeft = addTrigger("<", ""); paramLittleMoveLeft->setCustomShortName("moveLeft"); btnLittleMoveLeft = paramLittleMoveLeft->createButtonUI(); addAndMakeVisible(btnLittleMoveLeft);
    btnLittleMoveLeft->customBGColor = Colour(59, 59, 59); btnLittleMoveLeft->useCustomBGColor = true;
    paramBigMoveRight = addTrigger(">>", ""); paramBigMoveRight->setCustomShortName("moveRightBig"); btnBigMoveRight = paramBigMoveRight->createButtonUI(); addAndMakeVisible(btnBigMoveRight);
    btnBigMoveRight->customBGColor = Colour(59, 59, 59); btnBigMoveRight->useCustomBGColor = true;
    paramLittleMoveRight = addTrigger(">", ""); paramLittleMoveRight->setCustomShortName("moveRight"); btnLittleMoveRight = paramLittleMoveRight->createButtonUI(); addAndMakeVisible(btnLittleMoveRight);
    btnLittleMoveRight->customBGColor = Colour(59, 59, 59); btnLittleMoveRight->useCustomBGColor = true;
    paramEncoderRange = addTrigger("0-1", ""); paramEncoderRange->setCustomShortName("range"); btnEncoderRange = paramEncoderRange->createButtonUI(); addAndMakeVisible(btnEncoderRange);
    btnEncoderRange->customBGColor = Colour(59, 59, 59); btnEncoderRange->useCustomBGColor = true;
    paramCommandDown = addTrigger("Cmd -", ""); paramCommandDown->setCustomShortName("cmdDown"); btnCommandDown = paramCommandDown->createButtonUI(); addAndMakeVisible(btnCommandDown);
    btnCommandDown->customBGColor = Colour(59, 59, 59); btnCommandDown->useCustomBGColor = true;
    paramCommandUp = addTrigger("Cmd +", ""); paramCommandUp->setCustomShortName("cmdUp"); btnCommandUp = paramCommandUp->createButtonUI(); addAndMakeVisible(btnCommandUp);
    btnCommandUp->customBGColor = Colour(59, 59, 59); btnCommandUp->useCustomBGColor = true;
    paramExplodeCommand = addTrigger("<>", ""); paramExplodeCommand->setCustomShortName("explode"); btnExplodeCommand = paramExplodeCommand->createButtonUI(); addAndMakeVisible(btnExplodeCommand);
    btnExplodeCommand->customBGColor = Colour(59, 59, 59); btnExplodeCommand->useCustomBGColor = true;
    addAndMakeVisible(&commandNumber);
    commandNumber.setJustificationType(juce::Justification::centred);
    initEncoders();

}

Encoders::~Encoders()
{
    delete btnNumbersOrNames;
    delete btnHighLight;
    delete btnBlind;
    delete btnMode;
    delete btnBigMoveLeft;
    delete btnLittleMoveLeft;
    delete btnBigMoveRight;
    delete btnLittleMoveRight;
    delete btnEncoderRange;
    delete btnCommandDown;
    delete btnCommandUp;
    delete btnExplodeCommand;
}

void Encoders::initEncoders()
{
    nEncoders = dynamic_cast<BKEngine*>(Engine::mainEngine)->encodersNumber->intValue();
    encoders.clear();
    labels.clear();
    for (int i = 0; i < nEncoders; i++) {
        Slider* s = new Slider();
        addAndMakeVisible(s);
        s->setSliderStyle(Slider::RotaryHorizontalVerticalDrag);
        s->setRange(0, 1);
        s->setValue(0, juce::dontSendNotification);
        s->setColour(Slider::rotarySliderFillColourId, Colour(63, 63, 63));
        s->setNumDecimalPlacesToDisplay(5);
        s->addListener(this);
        s->setWantsKeyboardFocus(false);
        s->addMouseListener(this, false);
        encoders.add(s);

        Label* l = new Label();
        addAndMakeVisible(l);
        l->setText("", juce::dontSendNotification);
        l->attachToComponent(s, false);
        labels.add(l);
        l->setWantsKeyboardFocus(false);
        l->addMouseListener(this,true);
    }
    resized();
    updateEncoders();
}

void Encoders::paint (juce::Graphics& g)
{
    
}

void Encoders::resized()
{
    int windowH = getHeight();
    int windowW = getWidth();
    int btnWidth = 40;
    int margin = 5;

    int total = (13*btnWidth) + (3*margin);
    float ratio = windowW / float(total);

    float btnHeight = windowH / 7.0;
    btnHeight = jmin(20.0f, btnHeight);

    btnWidth *= ratio;
    margin *= ratio;

    commandNumber.setBounds(0,0, btnWidth, btnHeight);
    btnMode->setBounds(windowW - (0 * margin) - (1 * btnWidth), 0, btnWidth, btnHeight);
    btnNumbersOrNames->setBounds(windowW - (0 * margin) - (2 * btnWidth), 0, btnWidth, btnHeight);
    btnEncoderRange->setBounds(windowW - (0 * margin) - (3 * btnWidth), 0, btnWidth, btnHeight);
    btnHighLight->setBounds(windowW - (0 * margin) - (4 * btnWidth), 0, btnWidth, btnHeight);
    btnBlind->setBounds(windowW - (0 * margin) - (5 * btnWidth), 0, btnWidth, btnHeight);
    btnBigMoveRight->setBounds(windowW - (1 * margin) - (5 * btnWidth) - (1 * btnWidth), 0, btnWidth, btnHeight);
    btnLittleMoveRight->setBounds(windowW - (1 * margin) - (5 * btnWidth) - (2 * btnWidth), 0, btnWidth, btnHeight);
    btnLittleMoveLeft->setBounds(windowW - (1 * margin) - (5 * btnWidth) - (3 * btnWidth), 0, btnWidth, btnHeight);
    btnBigMoveLeft->setBounds(windowW - (1 * margin) - (5 * btnWidth) - (4 * btnWidth), 0, btnWidth, btnHeight);
    btnCommandUp->setBounds(windowW - (2 * margin) - (5 * btnWidth) - (5 * btnWidth), 0, btnWidth, btnHeight);
    btnCommandDown->setBounds(windowW - (2 * margin) - (5 * btnWidth) - (6 * btnWidth), 0, btnWidth, btnHeight);
    btnExplodeCommand->setBounds(windowW - (3 * margin) - (5 * btnWidth) - (7 * btnWidth), 0, btnWidth, btnHeight);

    commandLine.setBounds(0, 2 * btnHeight, windowW, btnHeight);

    if (filterBtns.size() > 0) {
        float w = getWidth() / filterBtns.size();
        for (int i = 0; i < filterBtns.size(); i++) {
            TextButton* t = filterBtns[i];
            t->setBounds(i * w, btnHeight, w, btnHeight);
        }
    }

    float w = windowW / nEncoders; 
    float h = windowH - (4*btnHeight);
    for (int i = 0; i < nEncoders; i++) {
        encoders[i]->setBounds(i*w, 4*btnHeight, w, h);
        encoders[i]->setTextBoxStyle(Slider::TextBoxBelow, false, w-4, btnHeight);
        labels[i]->setBounds(i*w, 3*btnHeight, w, btnHeight);
        labels[i]->setJustificationType(Justification::centred);
    }
}

void Encoders::sliderValueChanged(Slider* slider)
{   
    int index = encoders.indexOf(slider);
    double v = slider->getValue();
    if (encoderRange == 1) { v /= 100.; }
    else if (encoderRange == 2) { v /= 255.; }

    UserInputManager::getInstance()->encoderValueChanged(index, v, "");
}



void Encoders::buttonClicked(Button* b) {
        // filters
    int i = filterBtns.indexOf(dynamic_cast<TextButton*>(b));
    if (i >= 0) {
        ChannelFamily* target = availableFilters[i];
        if (selectedFilters.indexOf(target) >= 0) {
            selectedFilters.removeAllInstancesOf(target);
        }
        else {
            selectedFilters.add(target);
        }
        updateChannels();
    }
}

void Encoders::triggerTriggered(Trigger* t)
{
    if (t == paramEncoderRange) {
        encoderRange = (encoderRange + 1) % 3;
        updateRangeButton();
    }
    else if (t == paramMode) {
        mode = (mode + 1) % 2;
        updateModeButton();
    }
    else if (t == paramNumbersOrNames) {
        numberOrNames = (numberOrNames + 1) % 2;
        updateNumbersOrNamesButton();
        updateCommandLine();
    }
    else if (t == paramBigMoveLeft) {
        int bigOffset = engine->encoderBigNumber->getValue();
        offsetEncoders(-bigOffset);
    }
    else if (t == paramBigMoveRight) {
        int bigOffset = engine->encoderBigNumber->getValue();
        offsetEncoders(bigOffset);
    }
    else if (t == paramLittleMoveLeft) {
        offsetEncoders(-1);
    }
    else if (t == paramLittleMoveRight) {
        offsetEncoders(1);
    }
    else if (t == paramCommandUp) {
        if (UserInputManager::getInstance()->currentProgrammer != nullptr) {
            UserInputManager::getInstance()->currentProgrammer->selectNextCommand();
        }
        updateChannels();
    }
    else if (t == paramCommandDown) {
        if (UserInputManager::getInstance()->currentProgrammer != nullptr) {
            UserInputManager::getInstance()->currentProgrammer->selectPrevCommand();
        }
        updateChannels();
    }
    else if (t == paramExplodeCommand) {
        if (disableNextExplode) {
            disableNextExplode = false;
        }
        else if (UserInputManager::getInstance()->currentProgrammer != nullptr && UserInputManager::getInstance()->currentProgrammer->currentUserCommand != nullptr) {
            UserInputManager::getInstance()->currentProgrammer->currentUserCommand->explodeSelection();
            UserInputManager::getInstance()->currentProgrammer->selectNextCommand();
        }
        updateChannels();
    }
    else if (t == paramHighLight) {
        UserInputManager::getInstance()->toggleHightlight();
        updateChannels();
    }
    else if (t == paramBlind) {
        UserInputManager::getInstance()->toggleBlind();
        updateChannels();
    }

}

void Encoders::updateModeButton() {
    if (mode == 0) {
        paramMode->setNiceName("Val");
    }
    else if (mode == 1) {
        paramMode->setNiceName("Thru");
    }
    else if (mode == 2) {
        paramMode->setNiceName("Time");
    }
    updateEncoders();
}

void Encoders::updateNumbersOrNamesButton()
{
    if (numberOrNames == 0) {
        paramNumbersOrNames->setNiceName("123");
    }
    else if (numberOrNames == 1) {
        paramNumbersOrNames->setNiceName("Abc");
    }
    updateCommandLine();
}

void Encoders::updateHLButton()
{
    if (UserInputManager::getInstance()->currentProgrammer != nullptr) {
        bool hl = UserInputManager::getInstance()->currentProgrammer->highlightCurrentCommand->getValue();
        if (hl) {
            btnHighLight->customBGColor = Colour(127,0,0);
        }
        else {
            btnHighLight->customBGColor = Colour(59, 59, 59);
        }
    }

}

void Encoders::updateBlindButton()
{
    if (UserInputManager::getInstance()->currentProgrammer != nullptr) {
        String editMode = UserInputManager::getInstance()->currentProgrammer->editionMode->getValueData();
        if (editMode == "blind") {
            btnBlind->customBGColor = Colour(127, 0, 0);
        }
        else {
            btnBlind->customBGColor = Colour(59, 59, 59);
        }
    }
}

void Encoders::updateRangeButton() {
    double min = 0;
    double max = 1;
    if (encoderRange == 0) {
        paramEncoderRange->setNiceName("0-1");
        max = 1;
    }
    else if (encoderRange == 1) {
        paramEncoderRange->setNiceName("%");
        max = 100;
    }
    else if (encoderRange == 2) {
        paramEncoderRange->setNiceName("255");
        max = 255;
    }
    for (int i = 0; i < nEncoders; i++) {
        encoders[i]->setRange(min, max);
    }
    updateEncoders();
}

void Encoders::updateFilterBtns()
{
    float scale = 1;
    if (engine != nullptr && engine->encodersScale != nullptr) {
        scale = engine->encodersScale->getValue();
    }

    for (int i = 0; i < filterBtns.size(); i++) {
        removeChildComponent(filterBtns[i]);
    }
    filterBtns.clear();
    if (availableFilters.size() > 0) {
        float w = getWidth()/availableFilters.size();
        for (int i = 0; i < availableFilters.size(); i++) {
            TextButton* t = filterBtns.add(std::make_unique<TextButton>());
            t->setTransform(AffineTransform::scale(scale));
            t->setButtonText(availableFilters[i]->niceName);
            t->setBounds(i * w, 20, w, 20);
            t->addListener(this);
            if (selectedFilters.size() > 0) {
                if (selectedFilters.indexOf(availableFilters[i]) >= 0) {
                    t->setColour(TextButton::ColourIds::buttonColourId, Colour(196, 196, 196));
                    t->setColour(TextButton::ColourIds::textColourOffId, Colour(64, 64, 64));
                }
                else {
                    t->setColour(TextButton::ColourIds::buttonColourId, Colour(64, 64, 64));
                }
            }
            addAndMakeVisible(t);
        }
    }
    resized();
}

void Encoders::updateEncoders() {
    Command * currentCommand = nullptr; 
    if (UserInputManager::getInstance()->currentProgrammer != nullptr) {
        currentCommand = UserInputManager::getInstance()->currentProgrammer->currentUserCommand;
    }
    encodersOffset = jmin(encodersOffset, channels.size()-1);
    encodersOffset = jmax(encodersOffset, 0);

    lastOrigin.clear();

    for (int i = 0; i < nEncoders; i++) {
        int channelId = i+encodersOffset;
        if (mode == 2) {
            String l = "";
            if (i == 0) { l = "Delay From"; }
            if (i == 1) { l = "Delay To"; }
            if (i == 2) { l = "Fade From"; }
            if (i == 3) { l = "Fade to "; }
            labels[i]->setText(l, juce::sendNotification);
        }
        else if (channels.size() > channelId) {
            labels[i]->setText(String(channels[channelId]->niceName), juce::sendNotification);
            UserInputManager::getInstance()->feedback("/encoder/" + String(i + 1), String(channels[channelId]->niceName), "");
            encoders[i]->setEnabled(true);
            encoders[i]->setColour(Slider::rotarySliderFillColourId, Colour(192, 192, 192));
            encoders[i]->setValue(0, juce::dontSendNotification);
            if (currentCommand != nullptr) {
                float v = currentCommand->getChannelValue(channels[channelId], mode == 1);
                if (v >= 0) {
                    UserInputManager::getInstance()->feedback("/encoder/" + String(i + 1), v, "");
                    encoders[i]->setColour(Slider::rotarySliderFillColourId, Colour(255, 0, 0));
                    if (encoderRange == 1) { v *= 100; }
                    else if (encoderRange == 2) { v *= 255; }
                    encoders[i]->setValue(v, juce::dontSendNotification);
                }
                else {
                    UserInputManager::getInstance()->feedback("/encoder/" + String(i + 1), 0, "");
                }
            }
        }
        else {
            labels[i]->setText("", juce::dontSendNotification);
            UserInputManager::getInstance()->feedback("/encoder/" + String(i + 1), "", "");
            encoders[i]->setEnabled(false);
            encoders[i]->setColour(Slider::rotarySliderFillColourId, Colour(63, 63, 63));
            encoders[i]->setValue(0, juce::dontSendNotification);
            UserInputManager::getInstance()->feedback("/encoder/" + String(i + 1), 0, "");
        }
        //labels[i]->repaint();
    }
    //repaint();
    String cmdNumText = "";
    if (UserInputManager::getInstance()->currentProgrammer != nullptr && currentCommand != nullptr) {
        int index = UserInputManager::getInstance()->currentProgrammer->commands.items.indexOf(currentCommand);
        index = index +1;
        int tot = UserInputManager::getInstance()->currentProgrammer->commands.items.size();
        cmdNumText = String(index)+"/"+String(tot);
    }
    commandNumber.setText(cmdNumText, juce::NotificationType::dontSendNotification);
    updateEncodersValues();


}

void Encoders::updateChannels()
{
    channels.clear();
    availableFilters.clear();
    Command* currentCommand = nullptr;
    Array<ChannelType* >chans;
    if (UserInputManager::getInstance()->currentProgrammer != nullptr && UserInputManager::getInstance()->currentProgrammer->currentUserCommand != nullptr) {
        currentCommand = UserInputManager::getInstance()->currentProgrammer->currentUserCommand;
        currentCommand->selection.computeSelection();
        chans = currentCommand->selection.getControllableChannelsTypes();
    }
    

    int currentIndex = 0;

    for (int i = 0; i < ChannelFamilyManager::getInstance()->items.size(); i++) {
        for (int j = 0; j < ChannelFamilyManager::getInstance()->items[i]->definitions.items.size(); j++) {
            if (chans.contains(ChannelFamilyManager::getInstance()->items[i]->definitions.items[j])) {
                if (availableFilters.indexOf(ChannelFamilyManager::getInstance()->items[i]) == -1) {
                    availableFilters.add(ChannelFamilyManager::getInstance()->items[i]);
                }
                if (selectedFilters.size() == 0 || selectedFilters.indexOf(ChannelFamilyManager::getInstance()->items[i]) >= 0) {
                    channels.set(currentIndex, ChannelFamilyManager::getInstance()->items[i]->definitions.items[j]);
                    currentIndex++;
                }
            }
        }
    }
    for (int i = selectedFilters.size()-1; i >= 0; i--) {
        ChannelFamily* f = selectedFilters[i];
        if (availableFilters.indexOf(f) == -1) {
            selectedFilters.remove(i);
        }
    }
    updateFilterBtns();
    updateEncoders();
    updateCommandLine();
    updateHLButton();
    Brain::getInstance()->virtualFadersNeedUpdate = true;
    MessageManager::callAsync([this]() {
        BKColorPicker::getInstance()->repaint();
    });
}

void Encoders::updateEncodersValues() {
    const MessageManagerLock mmLock;
    Command* currentCommand = nullptr;
    Array<ChannelType* >chans;
    if (UserInputManager::getInstance()->currentProgrammer != nullptr) {
        currentCommand = UserInputManager::getInstance()->currentProgrammer->currentUserCommand;
    }
    Array<int> filledEncoders;
    if (currentCommand != nullptr) {
        for (int i = 0; i < currentCommand->values.items.size(); i++) {
            CommandValue* cv = currentCommand->values.items[i];
            if (cv->presetOrValue->getValue() == "value") {
                ChannelType* ct = dynamic_cast<ChannelType*>(cv->channelType->targetContainer.get());
                if (ct != nullptr) {
                    for (int ci = 0; ci < nEncoders; ci++) {
                        int channelId = ci + encodersOffset;
                        if (channels[channelId] == ct) {
                            float v = currentCommand->getChannelValue(channels[channelId], mode == 1);
                            if (encoderRange == 1) { v *= 100; }
                            else if (encoderRange == 2) { v *= 255; }
                            encoders[ci]->setValue(v, juce::dontSendNotification);
                            encoders[ci]->setColour(Slider::rotarySliderFillColourId, Colour(255, 0, 0));
                            filledEncoders.add(ci);
                        }
                    }
                }
            }
        }

        
        for (int ci = 0; ci < nEncoders; ci++) {

            if (!filledEncoders.contains(ci)) {

                int channelId = ci + encodersOffset;
                if (channelId < channels.size()) {

                    ChannelType* channel = channels[channelId];
                    double value = 0;
                    int n = 0;
                    currentCommand->isComputing.enter();
                    for (int iFixt = 0; iFixt < currentCommand->selection.computedSelectedSubFixtures.size(); iFixt++) {
                        SubFixture* sf = currentCommand->selection.computedSelectedSubFixtures[iFixt];
                        if (sf != nullptr && sf->channelsMap.contains(channel)) {
                            SubFixtureChannel* sfc = sf->channelsMap.getReference(channel);
                            n++;
                            value += sfc->postCuelistValue;
                        }
                    }
                    currentCommand->isComputing.exit();

                    if (n > 0) {
                        value = value / (float)n;
                        if (encoderRange == 1) { value *= 100; }
                        else if (encoderRange == 2) { value *= 255; }
                        encoders[ci]->setValue(value, juce::dontSendNotification);
                    }
                }
                //encoders[ci]->setColour(Slider::rotarySliderFillColourId, Colour(255, 0, 0));
            }
        }

    }



}

void Encoders::updateCommandLine()
{
    if (UserInputManager::getInstance()->currentProgrammer != nullptr) {
        bool useNames = numberOrNames == 1;
        String txt = UserInputManager::getInstance()->getProgrammer(true)->getTextCommand(useNames);
        commandLine.setText(txt, juce::dontSendNotification);
        UserInputManager::getInstance()->feedback("/encoders/cmdline", txt, "");
    }
}

void Encoders::mouseDoubleClick(const MouseEvent& e)
{
    Slider* s = dynamic_cast<Slider* >(e.eventComponent);

    int index = encoders.indexOf(s);
    UserInputManager::getInstance()->encoderValueChanged(index, -2, "");
    updateChannels();
    updateEncoders();
    updateEncodersValues();

}

void Encoders::mouseUp(const MouseEvent& e)
{  
    Component* c = e.eventComponent;
    Label* l = dynamic_cast<Label*>(c);
    if (l != nullptr && labels.indexOf(l) >= 0) {
        Slider* s = encoders[labels.indexOf(l)];
        if (s->isEnabled()) {
            float val = s->getValue();
            s->setValue(0, juce::dontSendNotification);
            s->setValue(1, juce::dontSendNotification);
            s->setValue(val, juce::sendNotification);
        }
    }
}

void Encoders::clearFilters()
{
    selectedFilters.clear();
    updateChannels();
}

void Encoders::toggleFilter(ChannelFamily* cf, bool forceSolo)
{
    if (availableFilters.indexOf(cf) == -1) return;
    bool newState = selectedFilters.indexOf(cf) == -1;
    if (forceSolo) {
        selectedFilters.clear();
    }
    if (newState) {
        selectedFilters.add(cf);
    }
    else {
        selectedFilters.removeAllInstancesOf(cf);
    }
    MessageManager::callAsync([this](){ updateChannels();});
}

void Encoders::offsetEncoders(int n)
{
    encodersOffset = jmax(0, encodersOffset + n);
    updateEncoders();
}

void Encoders::clear()
{
    channels.clear();
    clearFilters();
}

void Encoders::mouseDown(const MouseEvent& e)
{
    if (e.eventComponent != btnExplodeCommand) return;
    if (e.mods.isRightButtonDown()) {
        PopupMenu p;
        p.addItem("Explode command", [this]() {
            if (UserInputManager::getInstance()->currentProgrammer != nullptr && UserInputManager::getInstance()->currentProgrammer->currentUserCommand != nullptr) {
                UserInputManager::getInstance()->currentProgrammer->currentUserCommand->explodeSelection();
                UserInputManager::getInstance()->currentProgrammer->selectNextCommand();
            }
            });
        p.addItem("Explode command with output", [this]() {
            if (UserInputManager::getInstance()->currentProgrammer != nullptr && UserInputManager::getInstance()->currentProgrammer->currentUserCommand != nullptr) {
                UserInputManager::getInstance()->currentProgrammer->currentUserCommand->explodeSelection(true);
                UserInputManager::getInstance()->currentProgrammer->selectNextCommand();
            }
            });
        p.showMenuAsync(PopupMenu::Options(), [this](int result) {});

    }
    else {
        if (UserInputManager::getInstance()->currentProgrammer != nullptr && UserInputManager::getInstance()->currentProgrammer->currentUserCommand != nullptr) {
            UserInputManager::getInstance()->currentProgrammer->currentUserCommand->explodeSelection();
            UserInputManager::getInstance()->currentProgrammer->selectNextCommand();
        }
    }
    // do something else
    updateChannels();
}
